#include "Game.h"
#include <iostream>
#include "HUD.h"
#include "Enemies/Orc.h"
#include "Enemies/Vampire.h"
#include "Enemies/Boss.h"
#include <SDL_ttf.h>
#include "Support/AudioManager.h"
#include "algorithm"



Game::Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight)
    : level(renderer, 30, 20), spawnTimer(0.25f), roundTimer(5.0f), renderer_(renderer),
      windowWidth(windowWidth), windowHeight(windowHeight) {
    AudioManager::init();
    player = new Player(this, renderer, Vector2D(15, 10));
    hud = new HUD(renderer, player);

    SDL_Texture* fire = TextureLoader::loadTexture(renderer, "Fire.jpg");
    SDL_Texture* ice = TextureLoader::loadTexture(renderer, "Ice.jpg");
    hud->addSkill(fire, 5.0f);
    hud->addSkill(ice, 10.0f);
}

void Game::run() {
    auto time1 = std::chrono::system_clock::now();
    auto time2 = std::chrono::system_clock::now();
    const float dT = 1.0f / 60.0f;
    bool running = true;

    while (running) {
        time2 = std::chrono::system_clock::now();
        std::chrono::duration<float> timeDelta = time2 - time1;
        float timeDeltaFloat = timeDelta.count();

        if (timeDeltaFloat >= dT) {
            time1 = time2;
            std::cout << "Game::run - dT: " << dT << "\n";
            processEvents(renderer_, running);
            update(renderer_, dT, level);
            draw(renderer_);
            if (gameState == GameState::Quit) {
                running = false;
            }
        }
    }
}
Game::~Game() {
    delete player;
    delete hud;
    TextureLoader::deallocateTextures();
}

void Game::processEvents(SDL_Renderer* renderer, bool& running) {
    bool mouseDownThisFrame = false;
    AudioManager::init();
    //Process events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

            }

    // Gọi handleInput của HUD để xử lý thanh trượt âm lượng
        if (hud->handleInput(event, this)) {
            continue; // Nếu đang kéo slider, không xử lý input khác
        }

    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_1]) {
        if (hud->skills[0].ready()) { // Kiểm tra kỹ năng lửa sẵn sàng
        AudioManager::playSound("Data/Sound/fire.wav");
        Mix_VolumeChunk(AudioManager::getSound("Data/Sound/fire.wav"), 50); // 32 là âm lượng nhỏ

            hud->useSkill(0); // Kích hoạt kỹ năng
            // Offset dựa trên hướng cuối cùng của nhân vật
            Vector2D offset = (player->getLastDirection() == 1) ? Vector2D(2.0f, -0.5f) : Vector2D(-3.0f, -0.5f);
            Vector2D firePos = player->getPos() + offset; // Cách nhân vật 1 đơn vị theo hướng
            FireEffect fireEffect(renderer, firePos);
            addFireEffect(fireEffect); // Thêm hiệu ứng lửa
            player->currentMP -= 10;
        }

    }
    if (keyState[SDL_SCANCODE_2]) {
        if (hud->skills[1].ready()) { // Kỹ năng băng là skill thứ 2
            AudioManager::playSound("Data/Sound/ice.wav");
            Mix_VolumeChunk(AudioManager::getSound("Data/Sound/ice.wav"), 50); // 32 là âm lượng nhỏ

            hud->useSkill(1);
            Vector2D icePos = player->getPos() + Vector2D(0.0f, 1.0f);
            IceEffect iceEffect(renderer, icePos);
            addIceEffect(iceEffect);
            player->currentMP -= 15;
        }
    }
    player->handleInput(keyState, renderer);
}
}






void Game::update(SDL_Renderer* renderer, float dT, Level& level) {
    switch (gameState) {
        case GameState::Gameplay:
            if (player->isDead) {
                player->deathTimer.countDown(dT);
                player->frameTimer += dT;
                if (player->frameTimer >= player->frameTime) {
                    player->frameTimer = 0.0f;
                    if (player->frame < 6) {
                        player->frame++;
                    } else {
                        gameState = GameState::GameOver;
                    }
                }
                return;
            }

            updateUnits(dT);
            for (auto& coin : coins) coin->update(dT);
            updateFireEffects(dT);
            updateIceEffects(dT);
            updateSpawnUnitsIfRequired(renderer, dT);
            player->update(dT, listUnits, renderer, level);
            hud->update(dT, listUnits);
            level.checkPotionPickup(player->getPos(), player);
            updateCamera();

            if (!bossSpawned && spawnUnitCount == 0 && allEnemiesDead()) {
                std::cout << "Tất cả quái đã chết! Spawn boss...\n";
                AudioManager::init();
                AudioManager::playSound("Data/Sound/warning.mp3");

                spawnBoss();
            }

            // Kiểm tra Boss chết
            if (bossSpawned && allEnemiesDead()) {
                std::cout << "Boss đã chết! Phá đảo Map 1.\n";
                gameState = GameState::Victory; // Chuyển sang trạng thái Victory
            }
            break;

        case GameState::GameOver:
            showGameOverMenu(renderer);
            break;

        case GameState::Menu:
            if (showMenu(renderer)) {
                gameState = GameState::Gameplay;
            } else {
                gameState = GameState::Quit;
            }
            break;

        case GameState::Victory:
            showVictoryMenu(renderer); // Hiển thị khung Victory
            break;

        case GameState::Paused:
            showPauseMenu(renderer); // Hiển thị Pause Menu
            break;

        case GameState::Quit:
            break;
    }
}





void Game::updateUnits(float dT) {
    // Cập nhật trước
    for (auto& unit : listUnits) {
        if (unit) {
            unit->update(dT, level, listUnits, *player);
        }
    }
    // Xóa sau
    listUnits.erase(
        std::remove_if(listUnits.begin(), listUnits.end(),
            [](const std::shared_ptr<Unit>& unit) { return !unit || unit->isDead(); }),
        listUnits.end()
    );
}







void Game::updateSpawnUnitsIfRequired(SDL_Renderer* renderer, float dT) {
    static bool roundStarted = false; // Biến để theo dõi trạng thái vòng đấu

    // Nếu đây là vòng đầu tiên, bắt đầu ngay lập tức
    if (!roundStarted) {
        spawnUnitCount = 15;  // Chỉ spawn 15 quái duy nhất
        roundStarted = true;
    }

    // Chỉ spawn quái nếu còn số lượng cần spawn và đủ thời gian chờ
    spawnTimer.countDown(dT);
    if (spawnUnitCount > 0 && spawnTimer.timeSIsZero()) {
        spawnTimer.resetToMax(); // Đặt lại thời gian spawn quái tiếp theo

        // Chọn 1 vị trí random từ 4 phía màn hình
        std::vector<Vector2D> spawnLocations = {
            {rand() % level.GetX(), 0},                         // Trên
            {rand() % level.GetX(), level.GetY() - 1},         // Dưới
            {0, rand() % level.GetY()},                         // Trái
            {level.GetX() - 1, rand() % level.GetY()}         // Phải
        };

        Vector2D spawnPos = spawnLocations[rand() % spawnLocations.size()];
        addUnit(renderer, spawnPos);
        spawnUnitCount--; // Giảm số lượng quái cần spawn

        }
    }







void Game::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Màu xanh lá cây giống cỏ
    SDL_RenderClear(renderer); // Xóa sạch renderer

    level.draw(renderer, tileSize, cameraPos.x, cameraPos.y);
    level.drawDecor(renderer, cameraPos.x, cameraPos.y);

    for (auto& unitSelected : listUnits)
        if (unitSelected) unitSelected->draw(renderer, tileSize, cameraPos);
    for (auto& fireEffect : listFireEffects)
        fireEffect.draw(renderer, tileSize, cameraPos);
    for (auto& iceEffect : listIceEffects)
        iceEffect.draw(renderer, tileSize, cameraPos);
    player->draw(renderer, tileSize, cameraPos);
    hud->draw(renderer);
    for (auto& coin : coins)
        coin->draw(renderer, tileSize, cameraPos);

    SDL_RenderPresent(renderer);
}


void Game::addUnit(SDL_Renderer* renderer, Vector2D posMouse) {
    int enemyType = rand() % 3;  // 0: Slime, 1: Orc

    if (enemyType == 0)
        listUnits.push_back(std::make_shared<Unit>(renderer, posMouse));  // Slime
    else if (enemyType == 1)
        listUnits.push_back(std::make_shared<Orc>(renderer, posMouse));   // Orc
    else
        listUnits.push_back(std::make_shared<Vampire>(renderer, posMouse));   // Orc

}



void Game::updateCamera() {
    Vector2D targetPos = player->getPos();
    std::cout<< player->getPos().x << " " << player->getPos().y;
    // Tính vị trí mục tiêu của camera
    Vector2D targetCameraPos;
    targetCameraPos.x = targetPos.x - (windowWidth / (2 * tileSize));
    targetCameraPos.y = targetPos.y - (windowHeight / (2 * tileSize));

    // Nội suy mượt mà
    const float lerpSpeed = 0.1f;
    cameraPos.x = cameraPos.x + (targetCameraPos.x - cameraPos.x) * lerpSpeed;
    cameraPos.y = cameraPos.y + (targetCameraPos.y - cameraPos.y) * lerpSpeed;

    // Giới hạn camera
    float maxCameraX = level.GetX() - (windowWidth / tileSize);
    float maxCameraY = level.GetY() - (windowHeight / tileSize);

    if (cameraPos.x < 0) cameraPos.x = 0;
    if (cameraPos.y < 0) cameraPos.y = 0;
    if (cameraPos.x > maxCameraX) cameraPos.x = maxCameraX;
    if (cameraPos.y > maxCameraY) cameraPos.y = maxCameraY;

}


bool Game::allEnemiesDead() {
    for (auto& unit : listUnits) {
        if (unit->isAlive()) return false;
    }
    return true;
}




void Game::spawnBoss() {
    if (!renderer_) {
        std::cout << "Error: Renderer is null in Game::spawnBoss!" << std::endl;
        return;
    }

    std::cout << "Boss đã xuất hiện tại vị trí cố định!\n";

    // Lấy vị trí player
    Vector2D playerPos = player->getPos();
    std::cout << "Player Pos: (" << playerPos.x << ", " << playerPos.y << ")\n";

    // Chọn hướng ngẫu nhiên hoặc cố định (ví dụ: bên phải player)
    Vector2D bossOffset(3.0f, 0.0f); // Cách player 3 đơn vị bên phải, bạn có thể thay đổi
    // Một số tùy chọn khác:
    // Vector2D bossOffset(-3.0f, 0.0f); // Bên trái
    // Vector2D bossOffset(0.0f, -3.0f); // Phía trên
    // Vector2D bossOffset(0.0f, 3.0f);  // Phía dưới

    Vector2D bossPosition = playerPos + bossOffset;

    // Giới hạn vị trí trong bản đồ
    float maxX = static_cast<float>(level.GetX() - 1); // tileCountX - 1
    float maxY = static_cast<float>(level.GetY() - 1); // tileCountY - 1
    bossPosition.x = std::max(0.5f, std::min(bossPosition.x, maxX - 0.5f));
    bossPosition.y = std::max(0.5f, std::min(bossPosition.y, maxY - 0.5f));

    // Tạo boss
    auto boss = std::make_shared<Boss>(renderer_, bossPosition);
    if (!boss) {
        std::cout << "Error: Failed to create Boss!" << std::endl;
        return;
    }

    // Thêm vào danh sách đơn vị
    listUnits.push_back(boss);
    bossSpawned = true;

    // Debug thông tin
    std::cout << "Boss HP: " << boss->getHealth()
              << ", Pos: (" << bossPosition.x << ", " << bossPosition.y
              << "), List size: " << listUnits.size() << "\n";
}
void Game::updateFireEffects(float dT) {
    auto it = listFireEffects.begin();
    while (it != listFireEffects.end()) {
        it->update(dT, listUnits);
        if (it->isFinished()) {
            it = listFireEffects.erase(it);
        } else {
            ++it;
        }
    }
}


void Game::updateIceEffects(float dT) {
    auto it = listIceEffects.begin();
    while (it != listIceEffects.end()) {
        it->update(dT, listUnits);
        if (it->isFinished()) {
            it = listIceEffects.erase(it);
        } else {
            ++it;
        }
    }
}


bool Game::showMenu(SDL_Renderer* renderer) {
    bool inMenu = true;
    AudioManager::init();
    AudioManager::playMusic("Data/Sound/menu_background_music.ogg", -1);
    Mix_VolumeMusic(30);

    bool playSelected = false;

    SDL_Texture* background = TextureLoader::loadTexture(renderer, "menu_background.png");
    SDL_Texture* playButton = TextureLoader::loadTexture(renderer, "play_button.png");
    SDL_Texture* playButtonHover = TextureLoader::loadTexture(renderer, "play03.png");
    SDL_Texture* aboutButton = TextureLoader::loadTexture(renderer, "about_button.png");
    SDL_Texture* aboutButtonHover = TextureLoader::loadTexture(renderer, "about03.png");
    SDL_Texture* storeButton = TextureLoader::loadTexture(renderer, "back_button.png");
    SDL_Texture* storeButtonHover = TextureLoader::loadTexture(renderer, "back03.png");

    SDL_Rect playRect = { 100, 240, 150, 60 };
    SDL_Rect aboutRect = { 350, 240, 150, 60 };
    SDL_Rect storeRect = { 600, 240, 150, 60 };

    while (inMenu) {
        SDL_Event event;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        bool isHoverPlay = (mouseX >= playRect.x && mouseX <= playRect.x + playRect.w &&
                            mouseY >= playRect.y && mouseY <= playRect.y + playRect.h);
        bool isHoverAbout = (mouseX >= aboutRect.x && mouseX <= aboutRect.x + aboutRect.w &&
                             mouseY >= aboutRect.y && mouseY <= aboutRect.y + aboutRect.h);
        bool isHoverStore = (mouseX >= storeRect.x && mouseX <= storeRect.x + storeRect.w &&
                             mouseY >= storeRect.y && mouseY <= storeRect.y + storeRect.h);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState = GameState::Quit;
                inMenu = false;
                playSelected = false;
                break;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (isHoverPlay) {
                    AudioManager::playSound("Data/Sound/press_button.mp3");
                    playSelected = true;
                    gameState = GameState::Gameplay;
                    inMenu = false;
                }
                if (isHoverAbout) {
                    AudioManager::playSound("Data/Sound/press_button.mp3");
                    if (showAboutScreen(renderer)) {
                        inMenu = false;
                    }
                }
                if (isHoverStore) {
                    AudioManager::playSound("Data/Sound/press_button.mp3");
                    gameState = GameState::Quit;
                    inMenu = false;              // Thoát vòng lặp menu
                    playSelected = false;        // Đảm bảo không vào Gameplay
                    break;
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, nullptr, nullptr);
        SDL_RenderCopy(renderer, isHoverPlay ? playButtonHover : playButton, nullptr, &playRect);
        SDL_RenderCopy(renderer, isHoverAbout ? aboutButtonHover : aboutButton, nullptr, &aboutRect);
        SDL_RenderCopy(renderer, isHoverStore ? storeButtonHover : storeButton, nullptr, &storeRect);
        SDL_RenderPresent(renderer);
    }

    if (playSelected) {
        Mix_HaltMusic();
        AudioManager::playMusic("Data/Sound/playing_background_music.mp3", -1);
        Mix_VolumeMusic(30);
    }

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(playButton);
    SDL_DestroyTexture(playButtonHover);
    SDL_DestroyTexture(aboutButton);
    SDL_DestroyTexture(aboutButtonHover);
    SDL_DestroyTexture(storeButton);
    SDL_DestroyTexture(storeButtonHover);

    return playSelected;
}

bool Game::showAboutScreen(SDL_Renderer* renderer) {
    bool inAbout = true;

    // Load texture với kiểm tra NULL
    SDL_Texture* background = TextureLoader::loadTexture(renderer, "menu_background.png");
    SDL_Texture* backButton = TextureLoader::loadTexture(renderer, "back_button.png");
    SDL_Texture* backButtonHover = TextureLoader::loadTexture(renderer, "back03.png");

    // Kiểm tra load texture
    if (!background || !backButton) {
        std::cout << "Lỗi load texture: menu_background.png hoặc back_button.png\n";
        return false;
    }

    // Tạo overlay
    SDL_Texture* overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 800, 600);
    if (!overlay) {
        std::cout << "Lỗi tạo overlay texture\n";
        SDL_DestroyTexture(background);
        SDL_DestroyTexture(backButton);
        return false;
    }

    SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, overlay);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);

    SDL_Rect backRect = { 20, 20, 100, 50 };

    while (inAbout) {
        SDL_Event event;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool isButtonHover = (mouseX >= backRect.x && mouseX <= backRect.x + backRect.w &&
                              mouseY >= backRect.y && mouseY <= backRect.y + backRect.h);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                inAbout = false;
                break;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                if (isButtonHover) {
                    AudioManager::playSound("Data/Sound/press_button.mp3");
                    inAbout = false;
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, nullptr, nullptr);
        SDL_RenderCopy(renderer, overlay, nullptr, nullptr);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect infoBox = { 100, 100, 600, 400 };
        SDL_RenderFillRect(renderer, &infoBox);

        renderText(renderer, "Not Your Own Adventure !", 140, 120, 32);
        renderText(renderer, "Developer: Capybara ", 140, 170, 24);
        renderText(renderer, "Fight enemies and enjoy your adventure !", 140, 220, 24);
        renderText(renderer, "Press back to return to menu.", 140, 270, 24);

        SDL_RenderCopy(renderer, isButtonHover ? backButtonHover : backButton, nullptr, &backRect);
        SDL_RenderPresent(renderer);
    }

    // Dọn dẹp
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(overlay);
    SDL_DestroyTexture(backButton);
    return false;
}

void Game::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize) {
    // Khởi tạo SDL_ttf
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            std::cout << "SDL_ttf Init Error: " << TTF_GetError() << "\n";
            return;
        }
    }

    // Mở font
    TTF_Font* font = TTF_OpenFont("Data/Font/ThaleahFat.ttf", fontSize);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return;
    }

    // Tạo texture từ text
    SDL_Color color = { 0, 0, 0 }; // Màu đen
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) {
        std::cout << "Text Surface Error: " << TTF_GetError() << "\n";
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cout << "Text Texture Error: " << SDL_GetError() << "\n";
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font);
        return;
    }

    // Xác định vị trí hiển thị
    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };

    // Vẽ lên renderer
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    // Giải phóng bộ nhớ
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
}

void Game::showGameOverMenu(SDL_Renderer* renderer) {
    bool selecting = true;
    AudioManager::init();
    SDL_Event event;

    SDL_Texture* menuBackground = TextureLoader::loadTexture(renderer, "cut_frame_fixed.png");
    SDL_Texture* restartTexture = TextureLoader::loadTexture(renderer, "restart01.png");
    SDL_Texture* quitTexture = TextureLoader::loadTexture(renderer, "back_button.png");
    SDL_Texture* restartHoverTexture = TextureLoader::loadTexture(renderer, "restart03.png");
    SDL_Texture* quitHoverTexture = TextureLoader::loadTexture(renderer, "back03.png");

    SDL_Rect menuRect = { windowWidth / 2 - 200, windowHeight / 2 - 120, 400, 120 };
    SDL_Rect retryButton = { windowWidth / 2 - 150, windowHeight / 2 - 50, 100, 40 };
    SDL_Rect quitButton = { windowWidth / 2 + 55, windowHeight / 2 - 50, 100, 40 };

    while (selecting) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool isRestart = (mouseX >= retryButton.x && mouseX <= retryButton.x + retryButton.w &&
                          mouseY >= retryButton.y && mouseY <= retryButton.y + retryButton.h);
        bool isQuit = (mouseX >= quitButton.x && mouseX <= quitButton.x + quitButton.w &&
                       mouseY >= quitButton.y && mouseY <= quitButton.y + quitButton.h);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gameState = GameState::Quit;
                    selecting = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (isRestart) {
                        AudioManager::playSound("Data/Sound/press_button.mp3");
                        restartGame();
                        gameState = GameState::Gameplay;
                        selecting = false;
                    }
                    if (isQuit) {
                        AudioManager::playSound("Data/Sound/press_button.mp3");
                        restartGame();
                        if (showMenu(renderer)) {
                            gameState = GameState::Gameplay;
                        } else {
                            gameState = GameState::Quit;
                        }
                        selecting = false;
                    }
                    break;
            }
        }

        // Vẽ lại khung cảnh gameplay thay vì xóa màn hình
        level.draw(renderer, tileSize, cameraPos.x, cameraPos.y);
        for (auto& unitSelected : listUnits)
            if (unitSelected != nullptr)
                unitSelected->draw(renderer, tileSize, cameraPos);
        for (auto& fireEffect : listFireEffects)
            fireEffect.draw(renderer, tileSize, cameraPos);
        for (auto& iceEffect : listIceEffects)
            iceEffect.draw(renderer, tileSize, cameraPos);
        player->draw(renderer, tileSize, cameraPos);
        level.drawDecor(renderer, cameraPos.x, cameraPos.y);
        hud->draw(renderer);
        for (auto& coin : coins)
            coin->draw(renderer, tileSize, cameraPos);

        // Vẽ menu Game Over lên trên
        SDL_RenderCopy(renderer, menuBackground, nullptr, &menuRect);
        renderText(renderer, "YOU LOST!", windowWidth / 2 - 50, windowHeight / 2 - 80, 30);
        SDL_RenderCopy(renderer, isRestart ? restartHoverTexture : restartTexture, nullptr, &retryButton);
        SDL_RenderCopy(renderer, isQuit ? quitHoverTexture : quitTexture, nullptr, &quitButton);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(menuBackground);
    SDL_DestroyTexture(restartTexture);
    SDL_DestroyTexture(quitTexture);
    SDL_DestroyTexture(restartHoverTexture);
    SDL_DestroyTexture(quitHoverTexture);
}

void Game::restartGame() {
    player->reset();
    listUnits.clear();
    listFireEffects.clear();
    listIceEffects.clear();
    coins.clear();
    bossSpawned = false;
    spawnUnitCount = 15;
    gameOver = false;
    gameState = GameState::Gameplay;
    hud->survivalTime = 0.0f;
}

void Game::showVictoryMenu(SDL_Renderer* renderer) {
    bool selecting = true;
    SDL_Event event;

    SDL_Texture* victoryBackground = TextureLoader::loadTexture(renderer, "cut_frame_fixed.png"); // Giả định có file ảnh
    SDL_Texture* menuButton = TextureLoader::loadTexture(renderer, "back_button.png");
    SDL_Texture* menuButtonHover = TextureLoader::loadTexture(renderer, "back03.png");

    SDL_Rect victoryRect = { windowWidth / 2 - 200, windowHeight / 2 - 120, 400, 200 };
    SDL_Rect menuRect = { windowWidth / 2 - 50, windowHeight / 2 - 10, 100, 40 };

    while (selecting) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool isMenuHover = (mouseX >= menuRect.x && mouseX <= menuRect.x + menuRect.w &&
                            mouseY >= menuRect.y && mouseY <= menuRect.y + menuRect.h);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gameState = GameState::Quit;
                    selecting = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (isMenuHover) {
                        AudioManager::playSound("Data/Sound/press_button.mp3");
                        restartGame(); // Reset game trước khi về menu
                        if (showMenu(renderer)) {
                            gameState = GameState::Gameplay;
                        } else {
                            gameState = GameState::Quit;
                        }
                        selecting = false;
                    }
                    break;
            }
        }

        // Vẽ lại khung cảnh gameplay làm nền
        level.draw(renderer, tileSize, cameraPos.x, cameraPos.y);
        for (auto& unitSelected : listUnits)
            if (unitSelected != nullptr)
                unitSelected->draw(renderer, tileSize, cameraPos);
        for (auto& fireEffect : listFireEffects)
            fireEffect.draw(renderer, tileSize, cameraPos);
        for (auto& iceEffect : listIceEffects)
            iceEffect.draw(renderer, tileSize, cameraPos);
        player->draw(renderer, tileSize, cameraPos);
        level.drawDecor(renderer, cameraPos.x, cameraPos.y);
        hud->draw(renderer);
        for (auto& coin : coins)
            coin->draw(renderer, tileSize, cameraPos);

        // Vẽ khung Victory
        SDL_RenderCopy(renderer, victoryBackground, nullptr, &victoryRect);
        renderText(renderer, "Map 1 cleared!", windowWidth / 2 - 80, windowHeight / 2 - 80, 30);
        renderText(renderer, "New map is updating...", windowWidth / 2 - 80, windowHeight / 2 - 50, 20);
        SDL_RenderCopy(renderer, isMenuHover ? menuButtonHover : menuButton, nullptr, &menuRect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(victoryBackground);
    SDL_DestroyTexture(menuButton);
    SDL_DestroyTexture(menuButtonHover);
}

void Game::showPauseMenu(SDL_Renderer* renderer) {
    bool paused = true;
    SDL_Event event;

    SDL_Texture* pauseBackground = TextureLoader::loadTexture(renderer, "cut_frame_fixed.png"); // Giả định file ảnh
    SDL_Texture* resumeButton = TextureLoader::loadTexture(renderer, "back_button.png");
    SDL_Texture* resumeButtonHover = TextureLoader::loadTexture(renderer, "back03.png");

    SDL_Rect pauseRect = { windowWidth / 2 - 200, windowHeight / 2 - 120, 400, 150 };
    SDL_Rect resumeRect = { windowWidth / 2 - 50, windowHeight / 2 - 30, 100, 40 };

    while (paused) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool isResumeHover = (mouseX >= resumeRect.x && mouseX <= resumeRect.x + resumeRect.w &&
                              mouseY >= resumeRect.y && mouseY <= resumeRect.y + resumeRect.h);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gameState = GameState::Quit;
                    paused = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (isResumeHover) {
                        AudioManager::playSound("Data/Sound/press_button.mp3");
                        gameState = GameState::Gameplay; // Tiếp tục game
                        paused = false;
                    }
                    break;
            }
        }

        // Vẽ lại khung cảnh gameplay làm nền
        level.draw(renderer, tileSize, cameraPos.x, cameraPos.y);
        for (auto& unitSelected : listUnits)
            if (unitSelected != nullptr)
                unitSelected->draw(renderer, tileSize, cameraPos);
        for (auto& fireEffect : listFireEffects)
            fireEffect.draw(renderer, tileSize, cameraPos);
        for (auto& iceEffect : listIceEffects)
            iceEffect.draw(renderer, tileSize, cameraPos);
        player->draw(renderer, tileSize, cameraPos);
        level.drawDecor(renderer, cameraPos.x, cameraPos.y);
        hud->draw(renderer);
        for (auto& coin : coins)
            coin->draw(renderer, tileSize, cameraPos);

        // Vẽ khung Pause
        SDL_RenderCopy(renderer, pauseBackground, nullptr, &pauseRect);
        renderText(renderer, "Game Paused", windowWidth / 2 - 75, windowHeight / 2 - 70, 30);
        SDL_RenderCopy(renderer, isResumeHover ? resumeButtonHover : resumeButton, nullptr, &resumeRect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(pauseBackground);
    SDL_DestroyTexture(resumeButton);
    SDL_DestroyTexture(resumeButtonHover);
}

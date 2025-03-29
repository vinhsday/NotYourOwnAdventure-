#include "Game.h"
#include <iostream>
#include "HUD.h"
#include "Orc.h"
#include "Vampire.h"
#include "Boss.h"
#include <SDL_ttf.h>



Game::Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight)
    : level(renderer, 30, 20), // Tăng kích thước bản đồ: 30x20 tile
      spawnTimer(0.25f), roundTimer(5.0f), renderer_(renderer) {
    player = new Player(this, renderer, Vector2D(15, 10)); // Đặt nhân vật ở giữa bản đồ

    hud = new HUD(renderer, player);


        // Hiển thị menu trước khi vào game
    if (!showMenu(renderer)) {
        return; // Nếu người chơi không chọn Play, thoát game
    }


// Load icon kỹ năng
SDL_Texture* fireballIcon = TextureLoader::loadTexture(renderer, "Icon9.png");
SDL_Texture* shieldIcon = TextureLoader::loadTexture(renderer, "Icon10.png");

// Thêm kỹ năng vào HUD
hud->addSkill(fireballIcon, 5.0f);  // Kỹ năng Fireball có cooldown 5s
hud->addSkill(shieldIcon, 10.0f);   // Kỹ năng Shield có cooldown 10s

    //Run the game.
    if (window != nullptr && renderer != nullptr) {
        //Load the overlay texture.

        //Store the current times for the clock.
        auto time1 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::now();

        //The amount of time for each frame (60 fps).
        const float dT = 1.0f / 60.0f;


        //Start the game loop and run until it's time to stop.
        bool running = true;
        while (running) {
            //Determine how much time has elapsed since the last frame.
            time2 = std::chrono::system_clock::now();
            std::chrono::duration<float> timeDelta = time2 - time1;
            float timeDeltaFloat = timeDelta.count();

            //If enough time has passed then do everything required to generate the next frame.
            if (timeDeltaFloat >= dT) {
                //Store the new time for the next frame.
                time1 = time2;

                processEvents(renderer, running);
                update(renderer, dT, level);
                draw(renderer);
            }
        }
    }
}


Game::~Game() {
    //Clean up.
    TextureLoader::deallocateTextures();
}



void Game::processEvents(SDL_Renderer* renderer, bool& running) {
    bool mouseDownThisFrame = false;

    //Process events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

            }
        }

    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_1]) {
        if (hud->skills[0].ready()) { // Kiểm tra kỹ năng lửa sẵn sàng
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
            hud->useSkill(1);
            Vector2D icePos = player->getPos() + Vector2D(0.0f, 1.0f);
            IceEffect iceEffect(renderer, icePos);
            addIceEffect(iceEffect);
        }
    }
    player->handleInput(keyState, renderer);
}






void Game::update(SDL_Renderer* renderer, float dT, Level& level) {
     if (player->isDead) {
        player->deathTimer.countDown(dT);
        player->frameTimer += dT;

        // Chạy animation chết
        if (player->frameTimer >= player->frameTime) {
            player->frameTimer = 0.0f;
            if (player->frame < 6) {
                player->frame++;
            } else {
                gameOver = true; // Khi chạy hết animation chết
            }
        }

        if (gameOver) {
            showGameOverMenu(renderer);
        }

        return; // Ngưng update nếu nhân vật chết
    }

    //Update the units.
    updateUnits(dT);


    // Cập nhật coin
    for (auto& coin : coins) {
        coin->update(dT);
    }
    //Update the projectiles.
    updateProjectiles(dT);

    updateFireEffects(dT);

    updateIceEffects(dT);


    updateSpawnUnitsIfRequired(renderer, dT);

    player->update(dT, listUnits, renderer, level);

    hud->update(dT);

    level.checkPotionPickup(player->getPos(), player);




    updateCamera();

    if (!bossSpawned && spawnUnitCount == 0 && allEnemiesDead()) {
    std::cout << "Tất cả quái đã chết! Spawn boss...\n";
    triggerBossSpawn();
}


}


void Game::updateUnits(float dT) {
    //Loop through the list of units and update all of them.
    auto it = listUnits.begin();
    while (it != listUnits.end()) {
        bool increment = true;

        if ((*it) != nullptr) {
            (*it)->update(dT, level, listUnits, *player);

            //Check if the unit is still alive.  If not then erase it and don't increment the iterator.
            if ((*it)->isAlive() == false) {
                it = listUnits.erase(it);
                increment = false;
            }
        }

        if (increment)
            it++;
    }
}


void Game::updateProjectiles(float dT) {
    auto it = listProjectiles.begin();
    while (it != listProjectiles.end()) {
        it->update(dT, listUnits);
        if (it->getCollisionOccurred()) {
            it = listProjectiles.erase(it);  // ✅ Xóa đạn khi va chạm
        } else {
            ++it;
        }
    }
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
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    level.draw(renderer, tileSize, cameraPos.x, cameraPos.y);


    for (auto& unitSelected : listUnits)
        if (unitSelected != nullptr)
            unitSelected->draw(renderer, tileSize, cameraPos);

    for (auto& projectileSelected : listProjectiles)
        projectileSelected.draw(renderer, tileSize, cameraPos);

    for (auto& fireEffect : listFireEffects) // Vẽ hiệu ứng lửa
        fireEffect.draw(renderer, tileSize, cameraPos);

    for (auto& iceEffect : listIceEffects) // Vẽ hiệu ứng băng
        iceEffect.draw(renderer, tileSize, cameraPos);

    player->draw(renderer, tileSize, cameraPos);

    level.drawDecor(renderer, cameraPos.x, cameraPos.y);

     // ⭐ Thêm thanh máu và mana của người chơi ⭐
    hud->draw(renderer);

    for (auto& coin : coins) {
    coin->draw(renderer, tileSize, cameraPos);
}


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


void Game::triggerBossSpawn() {
    std::cout << "Boss xuất hiện! Hiệu ứng rung đất!\n";
    for (int i = 0; i < 10; i++) {
        cameraPos.x += (i % 2 == 0) ? 0.5f : -0.5f;
        SDL_Delay(30);
    }
    spawnBoss();
}

void Game::spawnBoss() {
    std::cout << "Boss đã xuất hiện tại vị trí cố định!\n";
    Vector2D bossPosition = player->getPos() - Vector2D(5,5); // Đặt vị trí boss cố định
    auto boss = std::make_shared<Boss>(renderer_, bossPosition);
    listUnits.push_back(boss);
    bossSpawned = true;

    std::cout << "Boss HP: " << boss->getHealth() << "\n"; // Kiểm tra HP
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
    bool playSelected = false;

    SDL_Texture* background = TextureLoader::loadTexture(renderer, "menu_background.png");
    SDL_Texture* playButton = TextureLoader::loadTexture(renderer, "play_button.png");
    SDL_Texture* playButtonHover = TextureLoader::loadTexture(renderer, "play03.png");
    SDL_Texture* aboutButton = TextureLoader::loadTexture(renderer, "about_button.png");
    SDL_Texture* aboutButtonHover = TextureLoader::loadTexture(renderer, "about03.png");
    SDL_Texture* storeButton = TextureLoader::loadTexture(renderer, "store_button.png");
    SDL_Texture* storeButtonHover = TextureLoader::loadTexture(renderer, "store03.png");

    SDL_Rect playRect = { 100, 240, 200, 80 };
    SDL_Rect aboutRect = { 350, 240, 200, 80 };
    SDL_Rect storeRect = { 600, 240, 200, 80 };


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
                inMenu = false;
                break;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (isHoverPlay) {
                    playSelected = true;
                    inMenu = false;
                }

                 if (isHoverAbout) {
                    if (showAboutScreen(renderer)) { // Gọi hàm About
                        inMenu = false; // Thoát hoàn toàn
                    }
                }

                if (isHoverStore) {
                    std::cout << "Store: Mở cửa hàng (chưa làm chức năng)\n";
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, nullptr, nullptr);

        // Hiệu ứng hover
        SDL_RenderCopy(renderer, isHoverPlay ? playButtonHover : playButton, nullptr, &playRect);
        SDL_RenderCopy(renderer, isHoverAbout ? aboutButtonHover : aboutButton, nullptr, &aboutRect);
        SDL_RenderCopy(renderer, isHoverStore ? storeButtonHover : storeButton, nullptr, &storeRect);

        SDL_RenderPresent(renderer);
    }
     // Hiệu ứng fade-out
    int alpha = 255;
    while (alpha > 0) {
        SDL_SetTextureAlphaMod(background, alpha);
        SDL_SetTextureAlphaMod(playButton, alpha);
        SDL_SetTextureAlphaMod(aboutButton, alpha);
        SDL_SetTextureAlphaMod(storeButton, alpha);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, nullptr, nullptr);
        SDL_RenderCopy(renderer, playButton, nullptr, &playRect);
        SDL_RenderCopy(renderer, aboutButton, nullptr, &aboutRect);
        SDL_RenderCopy(renderer, storeButton, nullptr, &storeRect);
        SDL_RenderPresent(renderer);

        alpha -= 5;
        SDL_Delay(10);
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

        renderText(renderer, "Tower Base Defense", 140, 120, 32);
        renderText(renderer, "Developer: Capybara Dev", 140, 170, 24);
        renderText(renderer, "Fight enemies and defend your base!", 140, 220, 24);
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
    SDL_Event event;

    SDL_Rect menuRect = { windowWidth / 2 - 100, windowHeight / 2 - 100, 200, 200 };
    SDL_Texture* restartTexture = TextureLoader::loadTexture(renderer,"restart01.png" );
    SDL_Texture* quitTexture = TextureLoader::loadTexture(renderer, "back_button.png");
    SDL_Texture* restartHoverTexture = TextureLoader::loadTexture(renderer,"restart03.png");
    SDL_Texture* quitHoverTexture = TextureLoader::loadTexture(renderer,"back03.png");

      // Vị trí các nút
        SDL_Rect retryButton = { windowWidth / 2 - 120, windowHeight / 2 - 20, 100, 40 };
        SDL_Rect quitButton = { windowWidth / 2 + 20, windowHeight / 2 - 20, 100, 40 };


    while (selecting) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    bool isRestart =(    mouseX >= retryButton.x && mouseX <= retryButton.x + retryButton.w &&
                        mouseY >= retryButton.y && mouseY <= retryButton.y + retryButton.h);

                    bool isQuit = (mouseX >= quitButton.x && mouseX <= quitButton.x + quitButton.w &&
                        mouseY >= quitButton.y && mouseY <= quitButton.y + quitButton.h);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    selecting = false;
                    exit(0);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    // Restart
                    if (isRestart) {
                        restartGame();
                        selecting = false;
                    }

                    // Quit
                    if (isQuit) {
                        showMenu(renderer);
                        selecting = false;
                        restartGame();

                    }
                    break;
            }
        }

        // Vẽ menu thua
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &menuRect);

        // Vẽ chữ
        renderText(renderer, "YOU LOST!", 140, 220, 24);


        SDL_RenderCopy(renderer, isRestart ? restartTexture : restartHoverTexture, nullptr, &retryButton);
        SDL_RenderCopy(renderer, isQuit ? quitTexture : quitHoverTexture, nullptr, &quitButton);


        SDL_RenderPresent(renderer);
    }
        // Giải phóng bộ nhớ
        SDL_DestroyTexture(restartTexture);
        SDL_DestroyTexture(quitTexture);
}


void Game::restartGame() {
    // Reset trạng thái game
    player->reset();
    listUnits.clear();
    listProjectiles.clear();
    listFireEffects.clear();
    listIceEffects.clear();
    coins.clear();

    bossSpawned = false;
    spawnUnitCount = 15;
    gameOver = false;
}


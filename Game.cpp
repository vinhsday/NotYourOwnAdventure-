#include "Game.h"
#include <iostream>
#include "HUD.h"
#include "Orc.h"
#include "Vampire.h"


Game::Game(SDL_Window* window, SDL_Renderer* renderer, int windowWidth, int windowHeight)
    : level(renderer, 30, 20), // Tăng kích thước bản đồ: 30x20 tile
      spawnTimer(0.25f), roundTimer(5.0f) {
    player = new Player(this, renderer, Vector2D(15, 10)); // Đặt nhân vật ở giữa bản đồ

    hud = new HUD(renderer, player);


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
    if (keyState[SDL_SCANCODE_1]) hud->useSkill(0);
    if (keyState[SDL_SCANCODE_2]) hud->useSkill(1);
    player->handleInput(keyState, renderer);
}






void Game::update(SDL_Renderer* renderer, float dT, Level& level) {
    //Update the units.
    updateUnits(dT);

    //Update the projectiles.
    updateProjectiles(dT);



    updateSpawnUnitsIfRequired(renderer, dT);

    player->update(dT, listUnits, renderer, level);

    hud->update(dT);


    updateCamera();

    std::cout << "📸 updateCamera() CALLED" << std::endl;

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
    // Nếu không còn quái trên màn hình và đã hết số lượng spawn -> Bắt đầu vòng mới
    if (listUnits.empty() && spawnUnitCount == 0) {
        roundTimer.countDown(dT);
        if (roundTimer.timeSIsZero()) {
            spawnUnitCount = 15;  // Mỗi vòng spawn 15 quái
            roundTimer.resetToMax();
        }
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

        std::cout << "Spawn enemy at: " << spawnPos.x << ", " << spawnPos.y
                  << " | Remaining: " << spawnUnitCount << std::endl;
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

    player->draw(renderer, tileSize, cameraPos);

    level.drawDecor(renderer, cameraPos.x, cameraPos.y);

     // ⭐ Thêm thanh máu và mana của người chơi ⭐
    hud->draw(renderer);

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
    std::cout << "🧑 Player Pos: " << targetPos.x << ", " << targetPos.y << std::endl;

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

    std::cout << "🎥 Camera Pos: " << cameraPos.x << ", " << cameraPos.y << std::endl;
}


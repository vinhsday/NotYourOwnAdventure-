#include "Level.h"
#include <iostream>
#include "Player.h"
#include "Support/AudioManager.h"


Level::Level(SDL_Renderer* renderer, int setTileCountX, int setTileCountY)
    : tileCountX(setTileCountX), tileCountY(setTileCountY),
      targetX(setTileCountX / 2), targetY(setTileCountY / 2) {
    lastPotionSpawnTime = std::chrono::steady_clock::now() - std::chrono::seconds(15);
    textureGrass = TextureLoader::loadTexture(renderer, "grass.jpg");
    potionHealthTexture = TextureLoader::loadTexture(renderer, "potion.png");
    potionManaTexture = TextureLoader::loadTexture(renderer, "Health_potion.png");
    // Load đồ vật trang trí
    decorTextures.push_back(TextureLoader::loadTexture(renderer, "Autumn_tree1.png"));
    decorTextures.push_back(TextureLoader::loadTexture(renderer, "Autumn_tree1.png"));
    decorTextures.push_back(TextureLoader::loadTexture(renderer, "Fruit_tree1.png"));

    // Thêm dòng này để tạo vị trí cho vật trang trí
    generateDecorPositions();

    // Add enemy spawners at corners
    int xMax = tileCountX - 1;
    int yMax = tileCountY - 1;
    setTileType(0, 0, TileType::enemySpawner);
    setTileType(xMax, 0, TileType::enemySpawner);
    setTileType(0, yMax, TileType::enemySpawner);
    setTileType(xMax, yMax, TileType::enemySpawner);

    calculateFlowField();

    spawnPotions();
}


void Level::generateDecorPositions() {
    decorPositions.clear();
    int numDecor = 7;  // Số lượng đồ trang trí
    float minSpacing = 3.0f; // 📏 Khoảng cách tối thiểu giữa các đồ trang trí (đơn vị tile)

    for (int i = 0; i < numDecor; i++) {
        bool validPosition = false;
        Vector2D newDecor;

        // 🔄 Lặp lại cho đến khi tìm được vị trí hợp lệ
        while (!validPosition) {
            int randX = rand() % tileCountX;
            int randY = rand() % tileCountY;
            newDecor = Vector2D(randX, randY);
            validPosition = true;

            // 🔍 Kiểm tra khoảng cách với các vật trang trí đã đặt
            for ( auto& decor : decorPositions) {
                if ((decor - newDecor).magnitude() < minSpacing) {
                    validPosition = false;
                    break;
                }
            }
        }

        decorPositions.push_back(newDecor);
    }
}



void Level::draw(SDL_Renderer* renderer, int tileSize, float camX, float camY) {
    if (!renderer) {
        std::cout << "Error: Renderer is null in Level::draw" << std::endl;
        return;
    }

    if (std::chrono::steady_clock::now() - lastPotionSpawnTime >= std::chrono::seconds(15)) {
        spawnPotions();
        lastPotionSpawnTime = std::chrono::steady_clock::now();
    }

    if (!textureGrass) {
        std::cout << "Error: textureGrass is null!" << std::endl;
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Màu xanh lá cây làm nền tạm thời
        SDL_RenderFillRect(renderer, nullptr);
    } else {
        for (int y = 0; y < tileCountY; y++) {
            for (int x = 0; x < tileCountX; x++) {
                // Sử dụng std::round để căn chỉnh chính xác
                int xPos = std::round(x * tileSize - camX);
                int yPos = std::round(y * tileSize - camY);
                // Tăng kích thước tile thêm 1 pixel để che khe hở
                SDL_Rect rect = {xPos, yPos, tileSize + 1, tileSize + 1};
                SDL_RenderCopy(renderer, textureGrass, NULL, &rect);
            }
        }
    }

    // Vẽ potion
    drawPotions(renderer, camX, camY);
}


void Level::spawnPotions() {
    int numPotions = 2;
    float minSpacing = 3.0f;
    int maxAttempts = 100;

    for (int i = 0; i < numPotions; i++) {
        bool validPosition = false;
        Vector2D newPotion;

        int attempts = 0;
        while (!validPosition && attempts < maxAttempts) {
            int randX = rand() % tileCountX;
            int randY = rand() % tileCountY;
            newPotion = Vector2D(randX, randY);
            validPosition = true;
            attempts++;

            // Kiểm tra vị trí không phải enemySpawner
            if (getTileType(randX, randY) == TileType::enemySpawner) {
                validPosition = false;
                continue;
            }

            // Kiểm tra khoảng cách với các potion khác
            for (auto& potion : potionPositions) {
                if ((potion - newPotion).magnitude() < minSpacing) {
                    validPosition = false;
                    break;
                }
            }

            // Nếu tìm thấy vị trí hợp lệ
            if (validPosition) {
                potionPositions.push_back(newPotion);

                // Random loại potion
                SDL_Texture* potionTexture = (rand() % 2 == 0) ? potionHealthTexture : potionManaTexture;
                potionTextures.push_back(potionTexture);

                std::cout << "Potion spawned at: (" << newPotion.x << ", " << newPotion.y << ")\n";
            }
        }
    }
}


Vector2D Level::getRandomEnemySpawnerLocation() {
    //Create a list of all tiles that are enemy spawners.
    std::vector<int> listSpawnerIndices;
    for (int count = 0; count < listTiles.size(); count++) {
        auto& tileSelected = listTiles[count];
        if (tileSelected.type == TileType::enemySpawner)
            listSpawnerIndices.push_back(count);
    }

    //If one or more spawners are found, pick one at random and output it's center position.
    if (listSpawnerIndices.empty() == false) {
        int index = listSpawnerIndices[rand() % listSpawnerIndices.size()];
        return Vector2D((float)(index % tileCountX) + 0.5f, (float)(index / tileCountX) + 0.5f);
    }

    return Vector2D(0.5f, 0.5f);
}



Level::TileType Level::getTileType(int x, int y) {
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY)
        return listTiles[index].type;

    return TileType::empty;
}


void Level::setTileType(int x, int y, TileType tileType) {
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY) {
        listTiles[index].type = tileType;
        calculateFlowField();
    }
}




void Level::calculateFlowField() {
    //Ensure the target is in bounds.
    int indexTarget = targetX + targetY * tileCountX;
    if (indexTarget > -1 && indexTarget < listTiles.size() &&
        targetX > -1 && targetX < tileCountX &&
        targetY > -1 && targetY < tileCountY) {

        //Reset the tile flow data.
        for (auto& tileSelected : listTiles) {
            tileSelected.flowDirectionX = 0;
            tileSelected.flowDirectionY = 0;
            tileSelected.flowDistance = flowDistanceMax;
        }

        //Calculate the flow field.
        calculateDistances();
        calculateFlowDirections();
    }
}


void Level::calculateDistances() {
    int indexTarget = targetX + targetY * tileCountX;

    //Create a queue that will contain the indices to be checked.
    std::queue<int> listIndicesToCheck;
    //Set the target tile's flow value to 0 and add it to the queue.
    listTiles[indexTarget].flowDistance = 0;
    listIndicesToCheck.push(indexTarget);

    //The offset of the neighboring tiles to be checked.
    const int listNeighbors[][2] = { { -1, 0}, {1, 0}, {0, -1}, {0, 1} };

    //Loop through the queue and assign distance to each tile.
    while (listIndicesToCheck.empty() == false) {
        int indexCurrent = listIndicesToCheck.front();
        listIndicesToCheck.pop();

        //Check each of the neighbors;
        for (int count = 0; count < 4; count++) {
            int neighborX = listNeighbors[count][0] + indexCurrent % tileCountX;
            int neighborY = listNeighbors[count][1] + indexCurrent / tileCountX;
            int indexNeighbor = neighborX + neighborY * tileCountX;

            //Ensure that the neighbor exists and isn't a wall.
            if (indexNeighbor > -1 && indexNeighbor < listTiles.size() &&
                neighborX > -1 && neighborX < tileCountX &&
                neighborY > -1 && neighborY < tileCountY &&
                listTiles[indexNeighbor].type != TileType::wall) {

                //Check if the tile has been assigned a distance yet or not.
                if (listTiles[indexNeighbor].flowDistance == flowDistanceMax) {
                    //If not the set it's distance and add it to the queue.
                    listTiles[indexNeighbor].flowDistance = listTiles[indexCurrent].flowDistance + 1;
                    listIndicesToCheck.push(indexNeighbor);
                }
            }
        }
    }
}


void Level::calculateFlowDirections() {
    //The offset of the neighboring tiles to be checked.
    const int listNeighbors[][2] = {
        {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
        {1, 0}, {1, -1}, {0, -1}, {-1, -1} };

    for (int indexCurrent = 0; indexCurrent < listTiles.size(); indexCurrent++) {
        //Ensure that the tile has been assigned a distance value.
        if (listTiles[indexCurrent].flowDistance != flowDistanceMax) {
            //Set the best distance to the current tile's distance.
            unsigned char flowFieldBest = listTiles[indexCurrent].flowDistance;

            //Check each of the neighbors;
            for (int count = 0; count < 8; count++) {
                int offsetX = listNeighbors[count][0];
                int offsetY = listNeighbors[count][1];

                int neighborX = offsetX + indexCurrent % tileCountX;
                int neighborY = offsetY + indexCurrent / tileCountX;
                int indexNeighbor = neighborX + neighborY * tileCountX;

                //Ensure that the neighbor exists.
                if (indexNeighbor > -1 && indexNeighbor < listTiles.size() &&
                    neighborX > -1 && neighborX < tileCountX &&
                    neighborY > -1 && neighborY < tileCountY) {
                    //If the current neighbor's distance is lower than the best then use it.
                    if (listTiles[indexNeighbor].flowDistance < flowFieldBest) {
                        flowFieldBest = listTiles[indexNeighbor].flowDistance;
                        listTiles[indexCurrent].flowDirectionX = offsetX;
                        listTiles[indexCurrent].flowDirectionY = offsetY;
                    }
                }
            }
        }
    }
}



Vector2D Level::getFlowNormal(int x, int y) {
    int index = x + y * tileCountX;
    if (index > -1 && index < listTiles.size() &&
        x > -1 && x < tileCountX &&
        y > -1 && y < tileCountY)
        return Vector2D((float)listTiles[index].flowDirectionX, (float)listTiles[index].flowDirectionY).normalize();

    return Vector2D();
}



void Level::drawDecor(SDL_Renderer* renderer, float camX, float camY) {
    if (decorTextures.empty()) {
        std::cout << "⚠️ No decor textures to draw!" << std::endl;
        return;
    }

    for (size_t i = 0; i < decorPositions.size(); ++i) {
        // Tính tọa độ pixel của đồ trang trí, điều chỉnh theo camera
        int xPos = (int)(decorPositions[i].x * 64) - (int)(camX * 64);
        int yPos = (int)(decorPositions[i].y * 64) - (int)(camY * 64);
        SDL_Rect decorRect = { xPos, yPos, 128, 128 };
        SDL_Texture* texture = decorTextures[i % decorTextures.size()];

        // Debug: In tọa độ để kiểm tra

        SDL_RenderCopy(renderer, texture, NULL, &decorRect);
    }
}


void Level::drawPotions(SDL_Renderer* renderer, float camX, float camY) {
    for (size_t i = 0; i < potionPositions.size(); i++) {
        int xPos = (int)(potionPositions[i].x * 64 - camX * 64);
        int yPos = (int)(potionPositions[i].y * 64 - camY * 64);
        SDL_Rect potionRect = { xPos, yPos, 32, 32 };

        // Hiển thị texture tùy loại potion
        if (i % 2 == 0)
            SDL_RenderCopy(renderer, potionHealthTexture, NULL, &potionRect);
        else
            SDL_RenderCopy(renderer, potionManaTexture, NULL, &potionRect);
    }
}


void Level::checkPotionPickup(Vector2D characterPosition, Player* player) {
    AudioManager::init();
    for (auto it = potionPositions.begin(); it != potionPositions.end();) {
        Vector2D potionPos = *it;

        // Kiểm tra khoảng cách giữa nhân vật và potion
        if ((potionPos - characterPosition).magnitude() < 1.0f) { // 1.0f là khoảng cách va chạm

            // Xác định loại potion
            size_t index = std::distance(potionPositions.begin(), it);
            if (potionTextures[index] == potionHealthTexture) {
                player->increaseHealth(); // Tăng máu
                AudioManager::playSound("Data/Sound/heal.wav");
                Mix_VolumeChunk(AudioManager::getSound("Data/Sound/heal.wav"), 50); // 32 là âm lượng nhỏ


            } else if (potionTextures[index] == potionManaTexture) {
                player->levelUp(); // Tăng cấp
                AudioManager::playSound("Data/Sound/levelup.wav");
                Mix_VolumeChunk(AudioManager::getSound("Data/Sound/levelup.wav"), 50); // 32 là âm lượng nhỏ
            }

            // Xóa potion khỏi danh sách
            it = potionPositions.erase(it);
            potionTextures.erase(potionTextures.begin() + index);
        } else {
            ++it;
        }
    }
}






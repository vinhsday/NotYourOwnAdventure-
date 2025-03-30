#include "TextureLoader.h"
#include <SDL_image.h>
#include <iostream>

std::unordered_map<std::string, SDL_Texture*> TextureLoader::umapTexturesLoaded;




SDL_Texture* TextureLoader::loadTexture(SDL_Renderer* renderer, std::string filename) {
    if (filename != "") {
        auto found = umapTexturesLoaded.find(filename);

        if (found != umapTexturesLoaded.end()) {
            //The texture was already loaded so return it.
            return found->second;
        }
        else {
            //Setup the relative filepath to the images folder using the input filename.
            std::string filepath = "Data/Images/" + filename;

            //Try to create a surface using the filepath.
            SDL_Surface* surfaceTemp = nullptr;
            if (filename.substr(filename.find_last_of(".") + 1) == "png" || filename.substr(filename.find_last_of(".") + 1) == "jpg") {
        surfaceTemp = IMG_Load(filepath.c_str());  // Load PNG
    } else {
        surfaceTemp = SDL_LoadBMP(filepath.c_str());  // Load BMP
    }

    if (!surfaceTemp) {
        std::cout << "Error: Failed to load image " << filename << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* textureOutput = SDL_CreateTextureFromSurface(renderer, surfaceTemp);
    SDL_FreeSurface(surfaceTemp);

    if (!textureOutput) {
        std::cout << "Error: SDL_CreateTextureFromSurface failed - " << SDL_GetError() << std::endl;
    }

                    return textureOutput;
                }
            }



    return nullptr;
}



void TextureLoader::deallocateTextures() {
    //Destroy all the textures
    while (umapTexturesLoaded.empty() == false) {
        auto it = umapTexturesLoaded.begin();
        if (it->second != nullptr)
            SDL_DestroyTexture(it->second);

        umapTexturesLoaded.erase(it);
    }
}

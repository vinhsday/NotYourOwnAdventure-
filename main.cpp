#include <iostream>
#include "SDL.h"
#include "Game.h"
#include <ctime>




int main(int argc, char* args[]) {
	//Seed the random number generator with the current time so that it will generate different
	//numbers every time the game is run.
	srand((unsigned)time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Error: Couldn't initialize SDL Video = " << SDL_GetError() << std::endl;
		return 1;
	}
	else {
		//Create the window.
		SDL_Window* window = SDL_CreateWindow("Tower Base Defense",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 576, 0);
		if (window == nullptr) {
			std::cout << "Error: Couldn't create window = " << SDL_GetError() << std::endl;
			return 1;
		}
		else {
			//Create a renderer for GPU accelerated drawing.
			SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
				SDL_RENDERER_PRESENTVSYNC);
			if (renderer == nullptr) {
				std::cout << "Error: Couldn't create renderer = " << SDL_GetError() << std::endl;
				return 1;
			}
			else {
				//Ensure transparent graphics are drawn correctly.
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

				//Output the name of the render driver.
				SDL_RendererInfo rendererInfo;
				SDL_GetRendererInfo(renderer, &rendererInfo);
				std::cout << "Renderer = " << rendererInfo.name << std::endl;

				//Get the dimensions of the window.
				int windowWidth = 0, windowHeight = 0;
				SDL_GetWindowSize(window, &windowWidth, &windowHeight);

				//Start the game.
				Game game(window, renderer, windowWidth, windowHeight);

				//Clean up.
				SDL_DestroyRenderer(renderer);
			}

			//Clean up.
			SDL_DestroyWindow(window);
		}

		//Clean up.
		SDL_Quit();
	}
	return 0;
}

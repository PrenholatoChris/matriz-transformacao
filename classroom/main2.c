#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 640
#define HEIGHT 480

int main(int arc, char *argv[]){
    SDL_Window *window;
    SDL_Event windowEvent;
    SDL_Renderer *renderer;
    int quit = 0;

    printf("Iniciando SDL...\n");
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Erro ao inicializar SDL! SDL Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    printf("Criando janela...\n");
    window = SDL_CreateWindow("Hello SDL World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL){
        printf("Erro ao criar janela! SDL Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    while(!quit){
        SDL_Delay(10);
        SDL_PollEvent(&windowEvent);

        switch (windowEvent.type){
            case SDL_QUIT:
                quit = 1;
                break;
            // TODO input handling code goes here
        }

        // clear window

        SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
        SDL_RenderClear(renderer);

        // TODO rendering code goes here
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, 10, 10,150, 150);

        // render window

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

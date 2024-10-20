#include "../common/game.h"

/* Entry point */
int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    /* Allocate and initialize game state and assets */
    game_state *game = new game_state();
    game_assets *assets = new game_assets();

    init_game(game);                       /* Initialize game state */
    init_sdl(&window, &renderer);          /* Initialize SDL */
    init_assets(assets, renderer);         /* Initialize assets */
    run_game_loop(game, renderer, assets); /* Game loop with fixed time step at 30 FPS*/

    /* Clean up and quit */
    SDL_Quit();
    delete game;
    delete assets;

    return 0;
}

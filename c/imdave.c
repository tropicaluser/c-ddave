#include "../common/game.h"

/* Entry point */
int main(int argc, char *argv[])
{
	SDL_Window *window;
	SDL_Renderer *renderer;

	struct game_state *game;
	struct game_assets *assets;

	/* Allocate and initialize game state and assets */
	game = malloc(sizeof(struct game_state));
	assets = malloc(sizeof(struct game_assets));

	init_game(game);
	init_sdl(&window, &renderer);					 /* Initialize SDL */
	init_assets(assets, renderer);				 /* Initialize assets */
	start_level(game);
	run_game_loop(game, renderer, assets); /* Game loop with fixed time step at 30 FPS*/

	/* Clean up and quit */
	SDL_Quit();
	free(game);
	free(assets);

	return 0;
};

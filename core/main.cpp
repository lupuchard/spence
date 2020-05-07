#include "map/Map.h"
#include "SFMLRenderer.h"
#include "SFMLEventManager.h"
#include "game/Game.h"

int main() {
	Map map;
	UI ui;
	Game game(map, ui);
	game.init();
	SFMLRenderer renderer(Pos2(1200, 800), 32, map, ui, game);
	SFMLEventManager events(renderer);

	map.set_renderer(renderer);

	while (renderer.get_window().isOpen()) {
		events.update();
		renderer.render();
	}
	return 0;
}

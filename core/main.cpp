#include "map/Map.h"
#include "SFMLRenderer.h"
#include "SFMLEventManager.h"

int main() {

	Map map;
	UI ui;
	Squirrel squirrel(map, ui);
	SFMLRenderer renderer(Pos2(1200, 800), 32, map, ui, squirrel);
	SFMLEventManager events(renderer);

	map.set_renderer(renderer);
	squirrel.init();

	while (renderer.get_window().isOpen()) {
		events.update();
		renderer.render();
	}
	return 0;
}

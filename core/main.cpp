#include "Map.h"
#include "SFMLRenderer.h"
#include "SFMLEventManager.h"

int main() {

	Map map = Map();
	Squirrel squirrel(map);
	SFMLRenderer renderer(Pos2(1000, 800), 32, map, squirrel);
	SFMLEventManager events(renderer);

	map.set_renderer(renderer);
	squirrel.init();

	while (renderer.get_window().isOpen()) {
		events.update();
		renderer.render();
	}
	return 0;
}

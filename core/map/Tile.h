#ifndef SPENCE_TILE_H
#define SPENCE_TILE_H

enum class Wall: uint8_t {
	None,
	Cover,
	Blocking,
};
struct Tile {
	Wall walls[4] = { Wall::None, Wall::None, Wall::None, Wall::None };
	int16_t type = -1;
};


#endif //SPENCE_TILE_H


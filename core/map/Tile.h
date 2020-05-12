#ifndef SPENCE_TILE_H
#define SPENCE_TILE_H

enum class Wall: uint8_t {
	None,
	Cover,
	Blocking,
};
struct Tile {
	Wall north_wall = Wall::None;
	Wall west_wall = Wall::None;
	int16_t type = -1;
};


#endif //SPENCE_TILE_H


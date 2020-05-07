#ifndef SPENCE_TILE_H
#define SPENCE_TILE_H

enum class Wall: uint8_t {
	None,
	Cover,
	Blocking,
};
struct Tile {
	enum Status: uint8_t {
		None,
		Floor,
		PassOnly,
		Impassable,
	};
	Tile(Status status = None): status(status) { }
	Wall walls[4] = { Wall::None, Wall::None, Wall::None, Wall::None };
	Status status = None;
	int16_t type = -1;
};


#endif //SPENCE_TILE_H


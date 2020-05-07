#ifndef SPENCE_FOV_H
#define SPENCE_FOV_H

#include "Map.h"

class Fov {
public:
	static Grid<char> calc(const Map& map, Pos2 pos, int radius);
};


#endif //SPENCE_FOV_H

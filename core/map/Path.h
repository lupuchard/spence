#ifndef SPENCE_PATH_H
#define SPENCE_PATH_H

#include "Map.h"

struct PathSettings {
	float ortho_cost = 1; // cost of normal orthogonal move
	float diag_cost  = 1; // cost of normal diagonal move
	float step_cost  = 0; // cost of climbing over low cover
	float climb_cost = 1; // cost of ascending/descending climbable wall
	float drop_cost  = 0; // cost of falling one story
};

struct PathNode {
	enum State { OPEN, CLOSED, ACCESSABLE, INACCESSABLE };
	Pos3 pos;
	State state = OPEN;
	Pos3 parent = Pos3(0, 0, -1);
	float dist = std::numeric_limits<float>::infinity();
};

class PathMap {
public:
	PathMap(): grid(Pos2()) { }
	PathMap(Pos3 source, Grid<PathNode> grid):
		source(source), grid(std::move(grid)) { }
	bool can_access(Pos3 pos);
	Pos3 source;

private:
	Grid<PathNode> grid;
	friend class Path;
};

class Path {
public:
	static PathMap calc(const Map& map, Pos3 pos, float radius, PathSettings& settings);
	static std::vector<Pos3> to(PathMap& path, Pos3 dest);
	static bool in_line(Pos3 a, Pos3 b, Pos3 c);
};


#endif //SPENCE_PATH_H

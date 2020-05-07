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
	Pos2 pos;
	State state = OPEN;
	Pos2 parent = Pos2(0, -1);
	float dist = std::numeric_limits<float>::infinity();
	int segment = 0;
};

class PathMap {
public:
	PathMap(): grid(Pos2()) { }
	PathMap(Pos2 source, Grid<PathNode> grid):
		source(source), grid(std::move(grid)) { }
	bool can_access(Pos2 pos);
	PathNode* get_node(Pos2 pos);
	Pos2 source;

private:
	Grid<PathNode> grid;
	friend class Path;
};

class Path {
public:
	static PathMap calc(const Map& map, Pos2 pos, float radius, PathSettings& settings, int num_segments = 1);
	static std::vector<Pos2> to(PathMap& path, Pos2 dest);
	static bool in_line(Pos2 a, Pos2 b, Pos2 c);
};


#endif //SPENCE_PATH_H

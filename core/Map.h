#ifndef SPENCE_MAP_H
#define SPENCE_MAP_H

#include <limits>
#include <unordered_map>
#include <memory>
#include "Grid.h"
#include "Unit.h"
#include "Tile.h"
#include "Renderer.h"

class Map {
public:
	Map();
	void set_renderer(Renderer& renderer);
	void reset(Pos2 size);
	//const Grid<Tile>& get_grid() const;
	Pos2 get_size() const;
	bool in_bounds(Pos2 pos) const;

	const std::vector<Tile>& get_tiles(Pos2 pos) const;
	bool has_tile(Pos3 pos) const;
	const Tile& get_tile(Pos3 pos) const;
	Wall get_wall(Pos3 pos, Dir dir) const;
	bool is_blocked(Pos3 pos, Dir dir) const;

	/*void set_tile(Pos3 pos, Tile tile);
	void set_tile_status(Pos3 pos, Tile::Status status);
	void set_tile_type(Pos3 pos, int16_t type);*/
	void set_wall(Pos3 pos, Dir dir, Wall wall);

	bool is_flat(Pos3 pos, Dir dir) const;
	bool has_cover(Pos3 pos, Dir dir) const;
	Grid<char> fov(Pos3 pos, int radius) const;

	struct PathSettings {
		const float ortho_cost = 1; // cost of normal orthogonal move
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
	struct PathMap {
		PathMap(): grid(Pos2()) { }
		PathMap(Pos3 source, Grid<PathNode> grid):
			source(source), grid(std::move(grid)) { }
		bool can_access(Pos3 pos);
		Pos3 source;
	private:
		Grid<PathNode> grid;
		friend class Map;
	};
	PathMap calc_pathmap(Pos3 pos, float radius, PathSettings& settings) const;
	std::vector<Pos3> to(PathMap& path, Pos3 dest) const;
	bool is_walkable(Pos3 start, Pos3 end) const;
	bool in_line(Pos3 a, Pos3 b, Pos3 c) const;

	std::string create_unit(Pos3 pos, const std::string& name);
	const std::vector<Unit>& get_units() const;
	Unit* get_unit(const std::string& name);

	void move(const Unit& unit, Pos3 pos);
	const Unit* get_unit(Pos3 pos) const;

private:
	bool test_walk(Pos2 a, Pos2 b, int z, std::vector<Dir> to_check) const;
	bool test_area(Pos2 corner1, Pos2 corner2, int height) const;

	Renderer* renderer = nullptr;
	Grid<Tile> grid;

	std::vector<Unit> units;
	std::unordered_map<std::string, Unit*> unit_map;
	Grid<Unit*> unit_grid;
};


#endif //SPENCE_MAP_H

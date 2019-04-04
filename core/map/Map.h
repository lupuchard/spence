#ifndef SPENCE_MAP_H
#define SPENCE_MAP_H

#include <limits>
#include <unordered_map>
#include <memory>
#include "Grid.h"
#include "../Unit.h"
#include "Tile.h"
#include "../Renderer.h"

class Map {
public:
	Map();
	void set_renderer(Renderer& renderer);
	void reset(Pos2 size);
	
	inline Pos2 get_size() const {
		return grid.get_size();
	}

	inline bool in_bounds(Pos2 pos) const {
		return grid.in_bounds(pos);
	}

	inline const std::vector<Tile>& get_tiles(Pos2 pos) const {
		return grid.get(pos);
	}

	inline bool has_tile(Pos3 pos) const {
		if (!in_bounds(pos.flat())) return false;
		const auto& tiles = get_tiles(pos.flat());
		return pos.z < tiles.size();
	}

	inline const Tile& get_tile(Pos3 pos) const {
		return grid.get(pos);
	}

	inline Wall get_wall(Pos3 pos, Dir dir) const {
		return get_tile(pos).walls[dir];
	}

	inline void set_wall(Pos3 pos, Dir dir, Wall wall) {
		grid.get(pos).walls[dir] = wall;
	}

	bool is_blocked(Pos3 pos, Dir dir) const;
	bool is_flat(Pos3 pos, Dir dir) const;
	bool has_cover(Pos3 pos, Dir dir) const;


	inline const std::vector<Unit>& get_units() const {
		return units;
	}

	inline const Unit* get_unit(Pos3 pos)const {
		return unit_grid.get(pos);
	}

	std::string create_unit(Pos3 pos, const std::string& name);
	Unit* get_unit(const std::string& name);
	void move(const Unit& unit, Pos3 pos);

private:
	Renderer* renderer = nullptr;
	Grid<Tile> grid;

	std::vector<Unit> units;
	std::unordered_map<std::string, Unit*> unit_map;
	Grid<Unit*> unit_grid;
};


#endif //SPENCE_MAP_H

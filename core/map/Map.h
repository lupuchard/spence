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

	/*inline bool has_tile(Pos2 pos) const {
		if (!in_bounds(pos.flat())) return false;
		const auto& tiles = get_tiles(pos.flat());
		return pos.z < tiles.size();
	}*/

	inline const Tile& get_tile(Pos2 pos) const {
		return grid.get(pos);
	}

	inline Wall get_wall(Pos2 pos, Dir dir) const {
		return get_tile(pos).walls[dir];
	}

	inline bool has_cover(Pos2 pos, Dir dir) const {
		const Tile& tile = get_tile(pos);
		return tile.walls[dir] == Wall::Blocking || tile.walls[dir] == Wall::Cover;
	}

	void set_wall(Pos2 pos, Dir dir, Wall wall);
	bool is_blocked(Pos2 pos, Dir dir) const;
	bool is_flat(Pos2 pos, Dir dir) const;

	inline const std::vector<std::unique_ptr<Unit>>& get_units() const {
		return units;
	}

	inline std::vector<std::unique_ptr<Unit>>& get_units() {
		return units;
	}

	inline const Unit* get_unit(Pos2 pos) const {
		return unit_grid.get(pos);
	}

	inline Unit* get_unit(Pos2 pos) {
		return unit_grid.get(pos);
	}

	Unit& create_unit(const UnitType& type, Side side, Pos2 pos);
	void move(const Unit& unit, Pos2 pos);

private:
	Renderer* renderer = nullptr;
	Grid<Tile> grid;

	std::vector<std::unique_ptr<Unit>> units;
	Grid<Unit*> unit_grid;
};


#endif //SPENCE_MAP_H

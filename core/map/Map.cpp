#include <assert.h>
#include <queue>
#include <algorithm>
#include "Map.h"

Map::Map(): grid(Pos2()), unit_grid(Pos2(), nullptr) { }

void Map::set_renderer(Renderer& r) {
	renderer = &r;
}

void Map::reset(Pos2 size) {
	grid = Grid<Tile>(size);
	unit_grid = Grid<Unit*>(size, nullptr);
	if (renderer) renderer->reset_grid(grid);
}

void Map::set_wall(Pos2 pos, Dir dir, Wall wall) {
	if (wall == Wall::Blocking || (wall == Wall::None && grid.get(pos).walls[dir] == Wall::Blocking)) {
		// Blocking must be symmetrical
		Pos2 adjacent_pos = Pos2(dir) + pos;
		if (in_bounds(adjacent_pos)) {
			grid.get(adjacent_pos).walls[flip(dir)] = wall;
		}
	}

	grid.get(pos).walls[dir] = wall;
}

bool Map::is_blocked(Pos2 pos, Dir dir) const {
	Wall wall = get_tile(pos).walls[dir];
	if (wall == Wall::Blocking) return true;
	Pos2 adjacent_pos = Pos2(dir) + pos;
	if (!in_bounds(adjacent_pos)) return true;
	return get_tile(adjacent_pos).walls[flip(dir)] == Wall::Blocking;
}

bool Map::is_flat(Pos2 pos, Dir dir) const {
	Wall wall = get_tile(pos).walls[dir];
	if (wall != Wall::None) return false;
	Pos2 adjacent_pos = Pos2(dir) + pos;
	if (!in_bounds(adjacent_pos)) return false;
	return get_tile(adjacent_pos).walls[flip(dir)] == Wall::None;
}

Unit& Map::create_unit(const UnitType& type, Side side, Pos2 pos) {
	units.push_back(std::make_unique<Unit>(type, side, pos));
	unit_grid.set(pos, units.back().get());
	return *units.back();
}

void Map::move(const Unit& unit, Pos2 pos) {
	Unit* mut_unit = unit_grid.get(unit.pos());
	assert(mut_unit != nullptr);
	unit_grid.set(unit.pos(), nullptr);
	mut_unit->set_pos(pos);
	unit_grid.set(pos, mut_unit);
}

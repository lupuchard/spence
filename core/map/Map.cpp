#include <cassert>
#include "Map.h"

Map::Map(): grid(Pos2()), unit_grid(Pos2(), nullptr), light_grid(Pos2(), 0) { }

void Map::set_renderer(Renderer& r) {
	renderer = &r;
}

void Map::reset(Pos2 size) {
	grid = Grid<Tile>(size);
	unit_grid = Grid<Unit*>(size, nullptr);
	light_grid = Grid<short>(size, 0);
	if (renderer) renderer->reset_grid(grid);
}

Wall Map::get_wall(Pos2 pos, Dir dir) const {
	const Tile& tile = grid.get(pos);
	switch (dir) {
		case Dir::North: return tile.north_wall;
		case Dir::West:  return tile.west_wall;
		case Dir::South: {
			Pos2 south_pos = pos + Pos2(0, 1);
			return in_bounds(south_pos) ? grid.get(south_pos).north_wall : Wall::None;
		}
		case Dir::East: {
			Pos2 east_pos = pos + Pos2(1, 0);
			return in_bounds(east_pos) ? grid.get(east_pos).west_wall : Wall::None;
		}
	}
	return Wall::None;
}

Wall& Map::get_wall(Pos2 pos, Dir dir) {
	static Wall wall_none = Wall::None;
	Tile& tile = grid.get(pos);
	switch (dir) {
		case Dir::North: return tile.north_wall;
		case Dir::West:  return tile.west_wall;
		case Dir::South: {
			Pos2 south_pos = pos + Pos2(0, 1);
			return in_bounds(south_pos) ? grid.get(south_pos).north_wall : wall_none;
		}
		case Dir::East: {
			Pos2 east_pos = pos + Pos2(1, 0);
			return in_bounds(east_pos) ? grid.get(east_pos).west_wall : wall_none;
		}
	}
	return wall_none;
}

void Map::set_wall(Pos2 pos, Dir dir, Wall wall) {
	get_wall(pos, dir) = wall;
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

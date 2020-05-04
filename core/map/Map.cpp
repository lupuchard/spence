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

void Map::set_wall(Pos3 pos, Dir dir, Wall wall) {
	if (wall == Wall::Blocking || (wall == Wall::None && grid.get(pos).walls[dir] == Wall::Blocking)) {
		// Blocking must be symmetrical
		Pos3 adjacent_pos = Pos3(Pos2(dir), pos.z) + pos;
		if (in_bounds(adjacent_pos.flat())) {
			grid.get(adjacent_pos).walls[flip(dir)] = wall;
		}
	}

	grid.get(pos).walls[dir] = wall;
}

bool Map::is_blocked(Pos3 pos, Dir dir) const {
	Wall wall = get_tile(pos).walls[dir];
	if (wall == Wall::Blocking) return true;
	Pos3 adjacent_pos = Pos3(Pos2(dir), 0) + pos;
	if (!in_bounds(adjacent_pos.flat())) return true;
	return get_tile(adjacent_pos).walls[flip(dir)] == Wall::Blocking;
}

bool Map::is_flat(Pos3 pos, Dir dir) const {
	Wall wall = get_tile(pos).walls[dir];
	if (wall != Wall::None) return false;
	Pos3 adjacent_pos = Pos3(Pos2(dir), 0) + pos;
	if (!in_bounds(adjacent_pos.flat())) return false;
	return get_tile(adjacent_pos).walls[flip(dir)] == Wall::None;
}

std::string Map::create_unit(Pos3 pos, const std::string& name) {
	auto iter = unit_map.find(name);
	std::string chosen_name = name;
	if (iter != unit_map.end()) {
		for (int i = 0; true; i++) {
			std::stringstream ss;
			ss << name << i;
			iter = unit_map.find(ss.str());
			if (iter == unit_map.end()) {
				chosen_name = ss.str();
				break;
			}
		}
	}
	units.emplace_back(pos, name);
	unit_map[chosen_name] = &units.back();
	unit_grid.set(pos, &units.back());
	if (renderer) renderer->add_unit(units.back());
	return chosen_name;
}

Unit* Map::get_unit(const std::string& name) {
	auto iter = unit_map.find(name);
	if (iter == unit_map.end()) {
		return nullptr;
	}
	return iter->second;
}

void Map::move(const Unit& unit, Pos3 pos) {
	Unit* mut_unit = unit_grid.get(unit.pos);
	assert(mut_unit != nullptr);
	unit_grid.set(unit.pos, nullptr);
	mut_unit->pos = pos;
	unit_grid.set(pos, mut_unit);
}

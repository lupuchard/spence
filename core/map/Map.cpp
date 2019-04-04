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

bool Map::has_cover(Pos3 pos, Dir dir) const {
	const Tile& tile = get_tile(pos);
	if (tile.walls[dir] == Wall::Blocking || tile.walls[dir] == Wall::InnerCover) {
		return true;
	}
	Pos3 adjacent_pos = Pos3(Pos2(dir), 0) + pos;
	if (!in_bounds(adjacent_pos.flat())) return false;
	const Tile& adjacent = get_tile(adjacent_pos);
	Dir flipped = flip(dir);
	return adjacent.walls[flipped] == Wall::Blocking || adjacent.walls[flipped] == Wall::OuterCover;
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

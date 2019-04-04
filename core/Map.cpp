#include <assert.h>
#include <list>
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

/*const Grid<Tile>& Map::get_grid() const {
	return grid;
}*/

Pos2 Map::get_size() const {
	return grid.get_size();
}

bool Map::in_bounds(Pos2 pos) const {
	return grid.in_bounds(pos);
}

const std::vector<Tile>& Map::get_tiles(Pos2 pos) const {
	return grid.get(pos);
}
bool Map::has_tile(Pos3 pos) const {
	if (!in_bounds(pos.flat())) return false;
	const auto& tiles = get_tiles(pos.flat());
	return pos.z < tiles.size();
}
const Tile& Map::get_tile(Pos3 pos) const {
	return grid.get(pos);
}
Wall Map::get_wall(Pos3 pos, Dir dir) const {
	return get_tile(pos).walls[dir];
}

/*void Map::set_tile(Pos3 pos, Tile tile) {
	grid.get(pos) = tile;
}
void Map::set_tile_status(Pos3 pos, Tile::Status status) {
	grid.get(pos).status = status;
}
void Map::set_tile_type(Pos3 pos, int16_t type) {
	grid.get(pos).type = type;
}*/
void Map::set_wall(Pos3 pos, Dir dir, Wall wall) {
	grid.get(pos).walls[dir] = wall;
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
bool Map::is_blocked(Pos3 pos, Dir dir) const {
	Wall wall = get_tile(pos).walls[dir];
	if (wall == Wall::Blocking) return true;
	Pos3 adjacent_pos = Pos3(Pos2(dir), 0) + pos;
	if (!in_bounds(adjacent_pos.flat())) return true;
	return get_tile(adjacent_pos).walls[flip(dir)] == Wall::Blocking;
}


// FIELD OF VIEW
struct FovState {
	Pos3 source;
	Pos2 quadrant, extent;
	int radius;
	const Map* map;
	Grid<char>* grid;
};
struct Line {
	Line() { }
	Line(Pos2 near, Pos2 far): near(near), far(far) { }
	int relative_slope(Pos2 pos) {
		return (far.y - near.y) * (far.x - pos.x) - (far.y - pos.y) * (far.x - near.x);
	}
	bool is_below(Pos2 pos)        { return relative_slope(pos) >  0; }
	bool is_below_or_has(Pos2 pos) { return relative_slope(pos) >= 0; }
	bool is_above(Pos2 pos)        { return relative_slope(pos) <  0; }
	bool is_above_or_has(Pos2 pos) { return relative_slope(pos) <= 0; }
	bool contains(Pos2 pos)       { return relative_slope(pos) == 0; }
	Pos2 near, far;
};
struct Bump {
	Bump(Pos2 loc = Pos2(), Bump* parent = nullptr): loc(loc), parent(parent) { }
	Pos2 loc;
	Bump* parent;
};
struct Field {
	Line steep, shallow;
	Bump*   steep_bump = nullptr;
	Bump* shallow_bump = nullptr;
};

using namespace std;

void add_shallow_bump(Pos2 pos, list<Field>::iterator& cur_field, list<Bump>& bumps) {
	cur_field->shallow.far = pos;
	bumps.push_back(Bump(pos, cur_field->shallow_bump));
	cur_field->shallow_bump = &bumps.back();
	for (Bump* bump = cur_field->steep_bump; bump != nullptr; bump = bump->parent) {
		if (cur_field->shallow.is_above(bump->loc)) {
			cur_field->shallow.near = bump->loc;
		}
	}
}
void add_steep_bump(Pos2 pos, list<Field>::iterator& cur_field, list<Bump>& bumps) {
	cur_field->steep.far = pos;
	bumps.push_back(Bump(pos, cur_field->steep_bump));
	cur_field->steep_bump = &bumps.back();
	for (Bump* bump = cur_field->shallow_bump; bump != nullptr; bump = bump->parent) {
		if (cur_field->steep.is_below(bump->loc)) {
			cur_field->steep.near = bump->loc;
		}
	}
}
list<Field>::iterator check_field(list<Field>::iterator cur_field, list<Field>& active_fields) {
	if (cur_field->shallow.contains(cur_field->steep.near) &&
		cur_field->shallow.contains(cur_field->steep.far) &&
		(cur_field->shallow.contains(Pos2(0, 1)) || cur_field->shallow.contains(Pos2(1, 0)))) {
		return active_fields.erase(cur_field);
	}
	return cur_field;
}
bool act_is_blocked(FovState& state, Pos2 pos) {
	Pos2 adjusted_pos = pos * state.quadrant + state.source.flat();
	if (!((state.quadrant.x * state.quadrant.y ==  1 && pos.x == 0 && pos.y != 0) ||
	      (state.quadrant.x * state.quadrant.y == -1 && pos.y == 0 && pos.x != 0))) {
		state.grid->set(Pos3(adjusted_pos, state.source.z), true);
	}
	for (Dir dir : (-pos).dirs()) {
		if (state.map->is_blocked(Pos3(pos, state.source.z), dir)) {
			return true;
		}
	}
	return false;
}
void visit_square(FovState& state, Pos2 dest, list<Field>::iterator& cur_field,
                  list<Bump>& steep_bumps, list<Bump>& shallow_bumps, list<Field> active_fields) {
	Pos2 top_left(dest.x, dest.y + 1);
	Pos2 bottom_right(dest.x + 1, dest.y);
	while (cur_field != active_fields.end() && cur_field->steep.is_below_or_has(bottom_right)) {
		++cur_field;
	}
	if (cur_field == active_fields.end())             return;
	if (cur_field->shallow.is_above_or_has(top_left)) return;
	if (!act_is_blocked(state, dest))                 return;

	if (cur_field->shallow.is_above(bottom_right) && cur_field->steep.is_below(top_left)) {
		cur_field = active_fields.erase(cur_field);
	} else if (cur_field->shallow.is_above(bottom_right)) {
		add_shallow_bump(top_left, cur_field, shallow_bumps);
		cur_field = check_field(cur_field, active_fields);
	} else if (cur_field->steep.is_below(top_left)) {
		add_steep_bump(bottom_right, cur_field, steep_bumps);
	} else {
		auto steeper_field = cur_field;
		auto shallower_field = active_fields.insert(cur_field, *cur_field);
		add_steep_bump(bottom_right, shallower_field, steep_bumps);
		check_field(shallower_field, active_fields);
		add_shallow_bump(top_left, steeper_field, shallow_bumps);
		cur_field = check_field(steeper_field, active_fields);
	}
}
void calc_fov_quadrant(FovState& state, Grid<char>& grid) {
	state.grid = &grid;
	state.extent = state.source.flat() + state.quadrant * state.radius;
	if (state.quadrant.x == 1) state.extent.x += 1;
	if (state.quadrant.y == 1) state.extent.y += 1;

	std::list<Bump> steep_bumps, shallow_bumps;
	std::list<Field> active_fields = { Field() };
	active_fields.back().shallow = Line(Pos2(0, 1), Pos2(state.extent.x, 0));
	active_fields.back().steep   = Line(Pos2(1, 0), Pos2(0, state.extent.y));
	Pos2 dest(0, 0);

	if (state.quadrant == Pos2(1, 1)) {
		act_is_blocked(state, dest);
	}
	auto cur_field = active_fields.begin();
	for (int i = 0; i < state.extent.x + state.extent.y && !active_fields.empty(); i++) {
		int start_j = std::max(0, i - state.extent.x);
		int   max_j = std::min(i, state.extent.y);
		for (int j = start_j; j <= max_j && cur_field != active_fields.end(); j++) {
			dest = Pos2(i - j, j);
			visit_square(state, dest, cur_field, steep_bumps, shallow_bumps, active_fields);
		}
	}
}

Grid<char> Map::fov(Pos3 pos, int radius) const {
	Pos2 top_left = (pos.flat() - Pos2(radius)).max(Pos2());
	Pos2 bot_rite = (pos.flat() + Pos2(radius)).max(get_size());
	Grid<char> fov_grid(bot_rite - top_left, false, top_left);

	FovState state;
	state.source = pos;
	state.radius = radius;
	state.map = this;
	Pos2 quadrants[4] = { Pos2(1, 1), Pos2(-1, 1), Pos2(-1, -1), Pos2(1, -1) };
	for (int i = 0; i < 4; i++) {
		state.quadrant = quadrants[i];
		calc_fov_quadrant(state, fov_grid);
	}

	// for upper:
	//   if on edge in direction of source
	// for lower (symmetric):
	//   if source is on edge, all lower in direction of edge
	//   otherwise, no lower visibility

	for (int z = 0; z < pos.z; z++) {
		state.source.z = z;
		for (int i = 0; i < 4; i++) {
			if (has_tile(Pos3(quadrants[i], pos.z) + pos)) continue;
			state.quadrant = quadrants[i];
			calc_fov_quadrant(state, fov_grid);
		}
	}
	for (int z = pos.z + 1; ; z++) {
		bool exit = true;
		for (int y = 0; y < fov_grid.get_size().y; y++) {
			for (int x = 0; x < fov_grid.get_size().x; x++) {
				if (has_tile(pos)) exit = false;
				Pos3 cur(Pos2(x, y) + fov_grid.get_offset(), z);
				if (!fov_grid.get(Pos3(cur.flat(), pos.z))) continue;
				bool blocked = true;
				for (Dir dir : (pos.flat() - cur.flat()).dirs()) {
					if (is_blocked(cur, dir) && get_tile(cur).status != Tile::None) continue;
					if (get_tile(cur + Pos3(Pos2(dir), 0)).status != Tile::None) continue;
					blocked = false;
					break;
				}
				if (blocked) fov_grid.set(cur, true);
			}
		}
		if (exit) break;
	}

	return fov_grid;
}

struct Compare {
	bool operator() (Map::PathNode* a, Map::PathNode* b) {
		return a->dist < b->dist;
	}
};
Map::PathMap Map::calc_pathmap(Pos3 pos, float radius, PathSettings& settings) const {
	int radius_i = (int)std::ceil(radius);
	Pos2 top_left = (pos.flat() - Pos2(radius_i)).max(Pos2());
	Pos2 bot_rite = (pos.flat() + Pos2(radius_i)).max(get_size());
	Grid<PathNode> path_grid(bot_rite - top_left, PathNode(), top_left);
	PathNode& start = path_grid.get(pos);
	start.pos = pos;
	start.dist = 0;
	std::priority_queue<PathNode*, std::vector<PathNode*>, Compare> active_q;
	active_q.push(&start);

	while (!active_q.empty()) {
		PathNode& current = *active_q.top();
		active_q.pop();
		if (current.state == PathNode::CLOSED) {
			continue;
		} else if (current.dist > radius) {
			current.state = PathNode::INACCESSABLE;
			continue;
		} else {
			current.state = PathNode::ACCESSABLE;
		}
		std::vector<std::pair<Pos3, float>> neighbors;
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				if (x == 0 && y == 0) continue;
				Pos3 pos0 = Pos3(x, y, 0) + current.pos;
				if (!path_grid.in_bounds(pos0)) continue;
				std::vector<Dir> dirs = Pos2(x, y).dirs();
				if (pos0.z > 0 && !has_tile(pos0)) {
					if (dirs.size() == 1) {
						pos0.z--;
						neighbors.emplace_back(pos0, settings.drop_cost);
						if (has_cover(current.pos, dirs[0])) {
							neighbors.back().second += settings.step_cost;
						}
					}
					continue;
				}
				bool blocked = false;
				bool cover   = false;
				for (Dir dir : dirs) {
					if (is_blocked(current.pos, dir)) {
						blocked = true;
						break;
					} else if (!is_flat(current.pos, dir)) {
						if (dirs.size() == 2) {
							blocked = true;
							break;
						}
						cover = true;
					}
				}
				if (dirs.size() == 2 && !blocked) {
					// check if cutting corners (not allowed)
					for (Dir dir : dirs) {
						if (!is_flat(pos0, flip(dir))) {
							blocked = true;
							break;
						}
					}
				}
				if (blocked) {
					if (dirs.size() == 1 && get_wall(current.pos, dirs[0]) == Wall::Climbable) {
						pos0.z++;
						neighbors.emplace_back(pos0, settings.climb_cost);
					}
					continue;
				}
				if (cover) {
					neighbors.emplace_back(pos0, settings.step_cost);
				} else if (dirs.size() == 1) {
					neighbors.emplace_back(pos0, settings.ortho_cost);
				} else {
					neighbors.emplace_back(pos0, settings.diag_cost);
				}
			}
		}
		for (auto neighbor : neighbors) {
			auto& neighbor_node = path_grid.get(neighbor.first);
			if (neighbor_node.state == PathNode::CLOSED) continue;
			float alt = current.dist + neighbor.second;
			if (alt < neighbor_node.dist) {
				neighbor_node.pos = neighbor.first;
				neighbor_node.dist = alt;
				neighbor_node.parent = current.pos;
				active_q.push(&neighbor_node);
			}
		}
	}
	return PathMap(pos, path_grid);
}
bool Map::PathMap::can_access(Pos3 pos) {
	if (!grid.in_bounds(pos)) return false;
	return grid.get(pos).state == PathNode::ACCESSABLE;
}
std::vector<Pos3> Map::to(PathMap& pathmap, Pos3 dest) const {
	if (dest.z == -1) return std::vector<Pos3>(1, dest);
	Pos3 prev = dest;
	Pos3 cur = pathmap.grid.get(dest).parent;
	if (cur.z == -1) return std::vector<Pos3> { cur, prev };
	while (true) {
		Pos3 next = pathmap.grid.get(cur).parent;
		if (next.z == -1) break;
		if (!in_line(prev, cur, next)) {
			if (is_walkable(prev, next)) {
				pathmap.grid.get(prev).parent = next;
			} else {
				prev = pathmap.grid.get(prev).parent;
				if (prev == cur) cur = next;
				continue;
			}
		}
		cur = next;
	}

	std::vector<Pos3> path;
	for (Pos3 pos = dest; pos.z != -1; pos = pathmap.grid.get(pos).parent) {
		path.push_back(pos);
	}
	std::reverse(path.begin(), path.end());
	return path;
}
bool Map::is_walkable(Pos3 start, Pos3 end) const {
	if (start.z != end.z) return false;
	Pos2 diff = (end - start).flat();
	if (abs(diff.x) <= 1 && abs(diff.y) <= 1) return false;
	auto dirs = diff.dirs();
	if (dirs.size() == 1) {
		return test_walk(start.flat(), end.flat() - dirs[0], end.z, dirs);
	}
	return test_walk(start.flat() + dirs[0], end.flat() - dirs[1], end.z,
	                 std::vector<Dir> { flip(dirs[0]), dirs[1] }) &&
	       test_walk(start.flat() + dirs[1], end.flat() - dirs[0], end.z,
	                 std::vector<Dir> { flip(dirs[1]), dirs[0] });


	/*if (start.z != end.z) return false;
	Pos2 diff = (end - start).flat();
	if (diff.x == 0 || diff.y == 0) return test_area(start.flat(), end.flat(), start.z);
	Pos2 cur = start.flat();
	while (cur != end.flat()) {
		int major = max(abs(diff.x), abs(diff.y));
		int minor = min(abs(diff.x), abs(diff.y));
		int step  = major / minor;
		int below = major % minor;
		auto dirs = diff.dirs();
		std::vector<Pos2> dirp = { Pos2(dirs[0]), Pos2(dirs[1]) };
		if (below > 0) {
			step++;
			if (!test_area(cur + dirp[1] * 2 + dirp[0] * (step - below),
			               cur + dirp[1] + dirp[0] * step, end.z)) return false;
		}
		if (!test_area(cur + dirp[0] * step, cur + dirp[1], end.z)) return false;
		cur += dirp[0] * step + dirp[1];
		diff = end.flat() - cur;
	}
	return true;*/

}
bool Map::test_walk(Pos2 start, Pos2 end, int z, std::vector<Dir> to_check) const {
	Pos2 d = (end - start).abs();
	Pos2 cur = start;
	int n = 1 + d.x + d.y;
	Pos2 inc = Pos2((end.x > start.x) ? 1 : -1, (end.y > start.y) ? 1 : -1);
	int error = d.x - d.y;
	d *= 2;
	for (; n > 0; n--) {
		for (auto dir : to_check) {
			if (!is_flat(Pos3(cur, z), dir)) return false;
		}
		if (error > 0) {
			cur.x += inc.x;
			error -= d.y;
		} else {
			cur.y += inc.y;
			error += d.y;
		}
	}
	return true;
}
bool Map::in_line(Pos3 a, Pos3 b, Pos3 c) const {
	if (a.z != b.z || b.z != c.z) return false;
	//if (a == b || b == c) return true;
	Pos3 ab = a - b;
	Pos3 ac = a - c;
	if (ab.y == 0) return ac.y == 0;
	return (ab.x / ab.y == ac.x / ac.y && ab.x % ab.y == ac.x % ac.y);
}
bool Map::test_area(Pos2 corner1, Pos2 corner2, int height) const {
	if (corner1.x > corner2.x) std::swap(corner1.x, corner2.x);
	if (corner1.y > corner2.y) std::swap(corner1.y, corner2.y);
	for (int x = corner1.x; x <= corner2.x; x++) {
		for (int y = corner1.y; y <= corner2.y; y++) {
			if (x < corner2.x && !is_flat(Pos3(x, y, height), Dir::East))  return false;
			if (y < corner2.y && !is_flat(Pos3(x, y, height), Dir::South)) return false;
		}
	}
	return true;
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
	units.push_back(Unit(pos, name));
	unit_map[chosen_name] = &units.back();
	unit_grid.set(pos, &units.back());
	if (renderer) renderer->add_unit(units.back());
	return chosen_name;
}

const std::vector<Unit>& Map::get_units() const {
	return units;
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
const Unit* Map::get_unit(Pos3 pos) const {
	return unit_grid.get(pos);
}

#include "Fov.h"
#include <list>

struct FovState {
	Pos2 source;
	Pos2 quadrant, extent;
	int radius = 0;
	const Map* map = nullptr;
	Grid<char>* grid = nullptr;
};

struct Line {
	Line() = default;
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


void add_shallow_bump(Pos2 pos, std::list<Field>::iterator& cur_field, std::list<Bump>& bumps) {
	cur_field->shallow.far = pos;
	bumps.emplace_back(pos, cur_field->shallow_bump);
	cur_field->shallow_bump = &bumps.back();
	for (Bump* bump = cur_field->steep_bump; bump != nullptr; bump = bump->parent) {
		if (cur_field->shallow.is_above(bump->loc)) {
			cur_field->shallow.near = bump->loc;
		}
	}
}

void add_steep_bump(Pos2 pos, std::list<Field>::iterator& cur_field, std::list<Bump>& bumps) {
	cur_field->steep.far = pos;
	bumps.emplace_back(pos, cur_field->steep_bump);
	cur_field->steep_bump = &bumps.back();
	for (Bump* bump = cur_field->shallow_bump; bump != nullptr; bump = bump->parent) {
		if (cur_field->steep.is_below(bump->loc)) {
			cur_field->steep.near = bump->loc;
		}
	}
}


std::list<Field>::iterator check_field(std::list<Field>::iterator cur_field, std::list<Field>& active_fields) {
	if (cur_field->shallow.contains(cur_field->steep.near) &&
	    cur_field->shallow.contains(cur_field->steep.far) &&
	    (cur_field->shallow.contains(Pos2(0, 1)) || cur_field->shallow.contains(Pos2(1, 0)))) {
		return active_fields.erase(cur_field);
	}
	return cur_field;
}

bool act_is_blocked(FovState& state, Pos2 pos) {
	Pos2 adjusted_pos = pos * state.quadrant + state.source;
	if (!((state.quadrant.x * state.quadrant.y ==  1 && pos.x == 0 && pos.y != 0) ||
	      (state.quadrant.x * state.quadrant.y == -1 && pos.y == 0 && pos.x != 0))) {
		state.grid->set(adjusted_pos, true);
	}
	for (Dir dir : (-pos).dirs()) {
		if (state.map->is_blocked(pos, dir)) {
			return true;
		}
	}
	return false;
}

void visit_square(FovState& state, Pos2 dest, std::list<Field>::iterator& cur_field,
                  std::list<Bump>& steep_bumps, std::list<Bump>& shallow_bumps, std::list<Field> active_fields) {
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
	state.extent = state.source + state.quadrant * state.radius;
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


Grid<char> Fov::calc(const Map& map, Pos2 pos, int radius) {
	Pos2 top_left = (pos - Pos2(radius)).max(Pos2());
	Pos2 bot_rite = (pos + Pos2(radius)).max(map.get_size());
	Grid<char> fov_grid(bot_rite - top_left, false, top_left);

	FovState state;
	state.source = pos;
	state.radius = radius;
	state.map = &map;
	Pos2 quadrants[4] = { Pos2(1, 1), Pos2(-1, 1), Pos2(-1, -1), Pos2(1, -1) };
	for (auto quadrant : quadrants) {
		state.quadrant = quadrant;
		calc_fov_quadrant(state, fov_grid);
	}

	// for upper:
	//   if on edge in direction of source
	// for lower (symmetric):
	//   if source is on edge, all lower in direction of edge
	//   otherwise, no lower visibility

	for (auto quadrant : quadrants) {
		if (map.in_bounds(quadrant + pos)) continue;
		state.quadrant = quadrant;
		calc_fov_quadrant(state, fov_grid);
	}
	for (int y = 0; y < fov_grid.get_size().y; y++) {
		for (int x = 0; x < fov_grid.get_size().x; x++) {
			Pos2 cur(Pos2(x, y) + fov_grid.get_offset());
			if (!fov_grid.get(cur)) continue;
			bool blocked = true;
			for (Dir dir : (pos - cur).dirs()) {
				if (map.is_blocked(cur, dir) && map.get_tile(cur).status != Tile::None) continue;
				if (map.get_tile(cur + Pos2(dir)).status != Tile::None) continue;
				blocked = false;
				break;
			}
			if (blocked) fov_grid.set(cur, true);
		}
	}

	return fov_grid;
}

#include "Fov.h"
#include <list>
#include <algorithm>
#include <limits>

const int MAX_INT = std::numeric_limits<short>::max();

struct FovState {
	Pos2 source;
	Pos2 quadrant;//, extent;
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
	if (pos.sqr_length() > state.radius * state.radius) return true;

	Pos2 adjusted_pos = pos * state.quadrant + state.source;
	if (!state.grid->in_bounds(adjusted_pos)) return true;

	for (Dir dir : (-(pos * state.quadrant)).dirs()) {
		if (state.map->get_wall(adjusted_pos, dir) == Wall::Blocking) {
			return true;
		}
	}

	//if (!((state.quadrant.x * state.quadrant.y ==  1 && pos.x == 0 && pos.y != 0) ||
	//      (state.quadrant.x * state.quadrant.y == -1 && pos.y == 0 && pos.x != 0))) {
		state.grid->set(adjusted_pos, true);
	//}

	return false;
}

void visit_square(FovState& state, Pos2 dest, std::list<Field>::iterator& cur_field,
                  std::list<Bump>& steep_bumps, std::list<Bump>& shallow_bumps, std::list<Field>& active_fields) {
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
	//state.extent = state.source + state.quadrant * state.radius;
	//if (state.quadrant.x == 1) state.extent.x += 1;
	//if (state.quadrant.y == 1) state.extent.y += 1;

	std::list<Bump> steep_bumps, shallow_bumps;
	std::list<Field> active_fields = { Field() };
	active_fields.back().shallow = Line(Pos2(0, 1), Pos2(MAX_INT, 0));
	active_fields.back().steep   = Line(Pos2(1, 0), Pos2(0, MAX_INT));
	Pos2 dest(0, 0);

	if (state.quadrant == Pos2(1, 1)) {
		act_is_blocked(state, dest);
	}
	auto cur_field = active_fields.begin();
	for (int i = 1; i < (state.radius + 1) * (state.radius + 1) && !active_fields.empty(); i++) {
		for (int j = 0; j <= i; j++) {
			dest = Pos2(i - j, j);
			visit_square(state, dest, cur_field, steep_bumps, shallow_bumps, active_fields);
		}
	}
}

bool ray_cast(const Map& map, Vec2 from, Vec2 to) {
	double x0 = from.x;
	double y0 = from.y;
	double x1 = to.x;
	double y1 = to.y;

	double dx = fabs(x1 - x0);
	double dy = fabs(y1 - y0);

	int x = int(floor(x0));
	int y = int(floor(y0));

	int n = 0;//1;
	int x_inc, y_inc;
	double error;

	if (dx == 0)
	{
		x_inc = 0;
		error = std::numeric_limits<double>::infinity();
	}
	else if (x1 > x0)
	{
		x_inc = 1;
		n += int(floor(x1)) - x;
		error = (floor(x0) + 1 - x0) * dy;
	}
	else
	{
		x_inc = -1;
		n += x - int(floor(x1));
		error = (x0 - floor(x0)) * dy;
	}

	if (dy == 0)
	{
		y_inc = 0;
		error -= std::numeric_limits<double>::infinity();
	}
	else if (y1 > y0)
	{
		y_inc = 1;
		n += int(floor(y1)) - y;
		error -= (floor(y0) + 1 - y0) * dx;
	}
	else
	{
		y_inc = -1;
		n += y - int(floor(y1));
		error -= (y0 - floor(y0)) * dx;
	}

	for (; n > 0; --n)
	{
		//visit(x, y);

		if (error > 0)
		{
			y += y_inc;
			error -= dx;
			if (map.get_wall(Pos2(x, y), y_inc > 0 ? Dir::North : Dir::South) == Wall::Blocking) return false;
		}
		else
		{
			x += x_inc;
			error += dy;
			if (map.get_wall(Pos2(x, y), x_inc > 0 ? Dir::West : Dir::East) == Wall::Blocking) return false;
		}
	}

	/*Vec2 diff = to - from;
	Pos2 cur = from.floor();

	int n = 1;
	Pos2 inc(0, 0);
	double error = 0;

	if (diff.x == 0) {
		error = std::numeric_limits<double>::infinity();
	} else if (to.x > from.x) {
		inc.x = 1;
		n += (int)to.x - cur.x;
		error = (std::floor(from.x) + 1 - from.x) * diff.y;
	} else {
		inc.x = -1;
		n += cur.x - (int)to.x;
		error = (from.x - std::floor(from.x)) * diff.y;
	}

	if (diff.y == 0) {
		error = -std::numeric_limits<double>::infinity();
	} else if (to.y > from.y) {
		inc.y = 1;
		n += (int)to.y - cur.y;
		error -= (std::floor(from.y) + 1 - from.y) * diff.x;
	} else {
		inc.x = -1;
		n += cur.y - (int)to.y;
		error -= (from.y - std::floor(from.y)) * diff.x;
	}

	for (; n > 0; n--) {
		if (error > 0) {
			cur.y += inc.y;
			error -= diff.x;
			if (map.get_wall(cur, inc.y > 0 ? Dir::North : Dir::South) == Wall::Blocking) return false;
		} else {
			cur.x += inc.x;
			error += diff.y;
			if (map.get_wall(cur, inc.x > 0 ? Dir::West : Dir::East) == Wall::Blocking) return false;
		}
	}*/

	return true;
}

bool ray_cast2(const Map& map, Vec2 fr, Vec2 to) {
	Vec2 diff = to - fr;
	Pos2 dir(diff.x > 0 ? 1 : -1, diff.y > 0 ? 1 : -1);
	diff = diff.abs();

	//Pos2 origin = Pos2(dir.x > 0 ? std::ceil(fr.x) : std::floor(fr.x), dir.y > 0 ? std::ceil(fr.y) : std::floor(fr.y));
	Vec2 origin = fr;
	Vec2 cur(0, 0);
	//Vec2 goal(std::floor(diff.x), std::floor(diff.y));
	Vec2 goal = diff;
	int num_steps = std::ceil(std::max(goal.x, goal.y));
	Vec2 inc = goal / num_steps;

	for (int i = 0; i < num_steps; i++) {
		Vec2 next = cur + inc;
		Pos2 cur_pos = (cur * Vec2(dir) + origin).round();
		Pos2 next_pos = (next * Vec2(dir) + origin).round();

		if (next_pos.x != cur_pos.x) {
			if (map.get_wall(next_pos, dir.x > 0 ? Dir::West : Dir::East) == Wall::Blocking) return false;
		}
		if (next_pos.y != cur_pos.y) {
			if (map.get_wall(next_pos, dir.y > 0 ? Dir::North : Dir::South) == Wall::Blocking) return false;
		}

		/*if ((int)next.x > (int)cur.x) {
			if (map.get_wall(Pos2(next) * dir + origin, dir.x > 0 ? Dir::West : Dir::East) == Wall::Blocking) return false;
		}
		if ((int)next.y > (int)cur.y) {
			if (map.get_wall(Pos2(next) * dir + origin, dir.y > 0 ? Dir::North : Dir::South) == Wall::Blocking) return false;
		}*/
		cur = next;

		/*if (cur_diff.y >= diff.y || cur_diff.y / cur_diff.x > diff.y / diff.x) {
			cur.x++;

		} else {
			cur.y++;
			if (map.get_wall(cur * dir + origin, dir.y > 0 ? Dir::North : Dir::South) == Wall::Blocking) return false;
		}*/
	}

	/*Pos2 curr = Pos2(dir.x > 0 ? std::ceil(fr.x) : std::floor(fr.x), dir.y > 0 ? std::ceil(fr.y) : std::floor(fr.y));
	Pos2 goal = Pos2(dir.x > 0 ? std::ceil(to.x) : std::floor(to.x), dir.y > 0 ? std::ceil(to.y) : std::floor(to.y));

	while (curr != goal) {
		Vec2 cur_diff = Vec2(curr) - fr;
		if (std::abs(cur_diff.y / cur_diff.x) > std::abs(diff.y / diff.x)) {
			curr += Vec2(dir.x, 0);
			if (map.get_wall(curr, dir.x > 0 ? Dir::West : Dir::East) == Wall::Blocking) {
				return false;
			}
		} else {
			curr += Vec2(0, dir.y);
			if (map.get_wall(curr, dir.y > 0 ? Dir::North : Dir::South) == Wall::Blocking) {
				return false;
			}
		}
	}*/

	return true;
}

Grid<char> Fov::calc(const Map& map, Pos2 pos, int radius) {
	/*Pos2 top_left = (pos - Pos2(radius)).max(Pos2());
	Pos2 bot_rite = (pos + Pos2(radius)).max(map.get_size());
	Grid<char> fov_grid(bot_rite - top_left, false, top_left);
	for (int y = pos.y - radius; y <= pos.y + radius; y++) {
		for (int x = pos.x - radius; x <= pos.x + radius; x++) {
			if (map.in_bounds(Pos2(x, y))) {
				fov_grid.set(Pos2(x, y), ray_cast(map, Vec2(pos), Vec2(x, y)));
			}
		}
	}
	return fov_grid;*/

	Pos2 top_left = (pos - Pos2(radius)).max(Pos2());
	Pos2 bot_rite = (pos + Pos2(radius)).max(map.get_size());
	//Grid<char> north_wall_fov(bot_rite - top_left, false, top_left);
	//Grid<char> west_wall_fov(bot_rite - top_left, false, top_left);
	Grid<char> fov_grid(bot_rite - top_left, false, top_left);
	for (int i = 0; i < 4; i++) {
		//if (map.get_wall(pos, (Dir)i) == Wall::Blocking) continue;
		Vec2 from = Vec2(pos) + Vec2(0.5, 0.5);
		switch ((Dir)i) {
			case Dir::North: from.y -= 0.4; break;
			case Dir::South: from.y += 0.4; break;
			case Dir::West:  from.x -= 0.4; break;
			case Dir::East:  from.x += 0.4; break;
		}

		for (int y = pos.y - radius; y <= pos.y + radius; y++) {
			for (int x = pos.x - radius; x <= pos.x + radius; x++) {

				/*if (x == pos.x || y == pos.y) {
					fov_grid.set(Pos2(x, y), ray_cast(map, from, Vec2(x, y)));
				} else {
					if (map.get_wall(Pos2(x, y), Dir::North) != Wall::Blocking && ray_cast(map, from, Vec2(x, y - 0.5))) {
						fov_grid.set(Pos2(x, y), true);
						fov_grid.set(Pos2(x, y - 1), true);
					}

					if (map.get_wall(Pos2(x, y), Dir::West) != Wall::Blocking && ray_cast(map, from, Vec2(x - 0.5, y))) {
						fov_grid.set(Pos2(x, y), true);
						fov_grid.set(Pos2(x - 1, y), true);
					}
				}*/

				if (ray_cast(map, from, Vec2(x + 0.1, y + 0.5)) ||
					ray_cast(map, from, Vec2(x + 0.9, y + 0.5)) ||
					ray_cast(map, from, Vec2(x + 0.5, y + 0.1)) ||
					ray_cast(map, from, Vec2(x + 0.5, y + 0.9))) {
					fov_grid.set(Pos2(x, y), true);
				}

				//north_wall_fov.set(Pos2(x, y), ray_cast(map, from, Vec2(pos.x, pos.y - 0.5)));
				//west_wall_fov.set(Pos2(x, y), ray_cast(map, from, Vec2(pos.x - 0.5, pos.y)));
			}
		}
	}

	return fov_grid;


	/*Pos2 top_left = (pos - Pos2(radius)).max(Pos2());
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
	}*/

	// for upper:
	//   if on edge in direction of source
	// for lower (symmetric):
	//   if source is on edge, all lower in direction of edge
	//   otherwise, no lower visibility

	/*for (auto quadrant : quadrants) {
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
				if (map.get_wall(cur, dir) == Wall::Blocking) continue;
				blocked = false;
				break;
			}
			if (blocked) fov_grid.set(cur, true);
		}
	}*/

	//return fov_grid;
}

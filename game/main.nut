
enum Dir { North, South, East, West }
enum Wall { None, Climbable, Cover, Blocking }
enum Side { None, You, Enemy }
enum Info { Name, B1, AP, Stamina, B2, Str, Mov, Aim }

local turn = Side.You;
local units = [];
local unit_map = {};

local selected_unit = null;

class Unit {
	constructor(unit_id, unit_side) {
		id = unit_id;
		side = unit_side;
	}

	id = null;
	str = 12;
	mov = 12;
	aim = 12;
	ap = 0;
	stamina = 3;
	side = Side.None;
}

function spawn_unit(pos, id, side) {
	_create_unit(pos, id);
	local unit = Unit(id, side);
	unit_map[id] <- unit;
	units.push(unit);
	return unit;
}

const MAP_WIDTH = 50;
const MAP_HEIGHT = 50;

function draw_rect_line(length, dim, offset_x, offset_y, dir, selection) {
	for (local i = 0; i < length; i++) {
		local r = rand() % 10;
		if (r == 0) {
			continue;
		} else if (r == 1) {
			selection = (selection == Wall.Cover ? Wall.Blocking : Wall.Cover);
		}

		local x = dim ? i : 0;
		local y = dim ? 0 : i;
		_set_wall([x + offset_x, y + offset_y, 0], dir, selection);
	}

	return selection;
}

function gen_rect() {
	local wid = rand() % 10 + 2;
	local hei = rand() % 10 + 2;
	local lef = rand() % (MAP_WIDTH - wid);
	local top = rand() % (MAP_HEIGHT - hei);

	local selection = rand() % 2 ? Wall.Blocking : Wall.Cover;

	selection = draw_rect_line(wid, true, lef, top, Dir.North, selection);
	selection = draw_rect_line(hei, false, lef, top, Dir.West, selection);
	selection = draw_rect_line(hei, false, lef + wid, top, Dir.West, selection);
	selection = draw_rect_line(wid, true, lef, top + hei, Dir.North, selection);
}

function _init() {
	_reset_map([MAP_WIDTH, MAP_HEIGHT]);
	spawn_unit([10, 10, 0], "paul", Side.You);

	local size = _map_size();

	for (local i = 0; i < 14; i++) {
		gen_rect();
	}

	for (local y = 0; y < size[1]; y++) {
		for (local x = 0; x < size[0]; x++) {
			for (local dir = 0; dir < 4; ++dir) {
				local r = rand() % 200;
				if (r == 0) {
					_set_wall([x, y, 0], dir, Wall.Blocking);
				} else if (r == 1) {
					_set_wall([x, y, 0], dir, Wall.Cover);
				}
			}
		}
	}

	init_turn(Side.You);
}

function _on_select(unit_id) {
	if (unit_id == null) {
		_clear_ui();
		selected_unit = null;
	} else {
		local unit = unit_map[unit_id];
		selected_unit = unit;
		local max_dist = unit.ap + (unit.stamina > 0 ? 1 : 0);
		_set_move_radius(unit_id, unit.mov * (max_dist / 4.), max_dist);
		update_unit_info();
	}
}

function _on_move(unit_id, pos, segment) {
	local move_cost = segment + 1;
	local unit = unit_map[unit_id];
	if (move_cost > unit.ap) {
		unit.stamina -= 1;
		move_cost--;
	}

	unit.ap -= move_cost;

	_set_pos(unit_id, pos);

	update_unit_info();
	update();
}

function update() {
	for (local i = 0; i < units.len(); i++) {
		local unit = units[i];
		if (unit.side == turn && unit.ap > 0) {
			return;
		}
	}

	init_turn(turn == Side.You ? Side.Enemy : Side.You);
}

function init_turn(new_turn) {
	turn = new_turn;

	for (local i = 0; i < units.len(); i++) {
		local unit = units[i];
		if (unit.side == turn) {
			unit.ap = 3;
		}
	}

	update_unit_info();

	if (turn == Side.Enemy) {
		enemy_turn();
	}
}

function update_unit_info() {
	if (selected_unit == null) {
		_clear_ui();
	} else {
		local unit = selected_unit;
		_set_ui_entry(Info.Name, unit.id);
		_set_ui_entry(Info.AP, "AP: " + unit.ap.tostring());
		_set_ui_entry(Info.Stamina, "Stamina: " + unit.stamina.tostring());
		_set_ui_entry(Info.Str, "Str: " + unit.str.tostring());
		_set_ui_entry(Info.Mov, "Mov: " + unit.mov.tostring());
		_set_ui_entry(Info.Aim, "Aim: " + unit.aim.tostring());
	}
}

function enemy_turn() {
	update();
}

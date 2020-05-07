#include "Game.h"
#include "Weapon.h"

const int MAP_WIDTH = 50;
const int MAP_HEIGHT = 50;

void Game::init() {
	map.reset(Pos2(MAP_WIDTH, MAP_HEIGHT));

	Weapon blunderbuss(3, 5, RangeType::Short);
	Weapon musket(3, 4, RangeType::Long);
	Weapon crossbow(2, 4, RangeType::Long, true);
	Weapon sword(3, 4, RangeType::Melee, true);

	unit_types.push_back(std::make_unique<UnitType>("Vanguard", 6, 6, 7));
	map.create_unit(*unit_types.back(), Side::You, Pos2(10, 10));
	unit_types.push_back(std::make_unique<UnitType>("Assassin", 6, 7, 6));
	map.create_unit(*unit_types.back(), Side::You, Pos2(10, 11));
	unit_types.push_back(std::make_unique<UnitType>("Hunter", 7, 6, 6));
	map.create_unit(*unit_types.back(), Side::You, Pos2(11, 10));

	unit_types.push_back(std::make_unique<UnitType>("Newt", 5, 6, 3));
	map.create_unit(*unit_types.back(), Side::Enemy, Pos2(40, 40));
	map.create_unit(*unit_types.back(), Side::Enemy, Pos2(40, 41));
	unit_types.push_back(std::make_unique<UnitType>("Salamander", 6, 6, 5));
	map.create_unit(*unit_types.back(), Side::Enemy, Pos2(41, 40));

	for (int i = 0; i < 14; i++) {
		gen_rect();
	}

	Pos2 size = map.get_size();

	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			for (int dir = 0; dir < 4; ++dir) {
				int r = rand() % 200;
				if (r == 0) {
					map.set_wall(Pos2(x, y), (Dir)dir, Wall::Blocking);
				} else if (r == 1) {
					map.set_wall(Pos2(x, y), (Dir)dir, Wall::Cover);
				}
			}
		}
	}

	init_turn(Side::You);
}

void Game::on_select(Unit* unit) {
	if (unit == nullptr) {
		ui.clear();
		selected_unit = nullptr;
	} else {
		selected_unit = unit;
		int move_segments = unit->ap() + (unit->stamina() > 0 ? 1 : 0);
		unit->set_move((float)unit->type().mov * ((float)move_segments / 2.f), move_segments);
		update_unit_info();
	}
}

void Game::on_move(Unit& unit, Pos2 pos, int segment) {
	int move_cost = segment + 1;
	if (move_cost > unit.ap()) {
		unit.use_stamina(1);
		move_cost--;
	}

	unit.use_ap(move_cost);
	map.move(unit, pos);

	update_unit_info();
	update();
}

void Game::init_turn(Side new_turn) {
	turn = new_turn;

	for (auto& unit : map.get_units()) {
		if (unit->side() == turn) {
			unit->reset_ap();
		}
	}

	update_unit_info();

	if (turn == Side::Enemy) {
		enemy_turn();
	}
}

void Game::enemy_turn() {
	for (auto& unit : map.get_units()) {
		if (unit->side() == turn) {
			unit->use_ap(3);
		}
	}

	update();
}

void Game::update() {
	for (auto& unit : map.get_units()) {
		if (unit->side() == turn && unit->ap() > 0) {
			return;
		}
	}

	init_turn(turn == Side::You ? Side::Enemy : Side::You);
}

void Game::update_unit_info() {
	if (selected_unit == nullptr) {
		ui.clear();
	} else {
		ui.set_entry(NAME, selected_unit->type().name);
		ui.set_entry(AP, "AP: " + std::to_string(selected_unit->ap()));
		ui.set_entry(STAMINA, "Stamina: " + std::to_string(selected_unit->stamina()));
		//ui.set_entry(STR, "Str: " + std::to_string(selected_unit->type().str));
		ui.set_entry(MOV, "Mov: " + std::to_string(selected_unit->type().mov));
		ui.set_entry(AIM, "Aim: " + std::to_string(selected_unit->type().aim));
	}
}

Wall Game::draw_rect_line(int length, bool dim, Pos2 offset, Dir dir, Wall selection) {
	for (int i = 0; i < length; i++) {
		int r = rand() % 10;
		if (r == 0) {
			continue;
		} else if (r == 1) {
			selection = (selection == Wall::Cover ? Wall::Blocking : Wall::Cover);
		}

		int x = dim ? i : 0;
		int y = dim ? 0 : i;
		map.set_wall(Pos2(x + offset.x, y + offset.y), dir, selection);
	}

	return selection;
}

void Game::gen_rect() {
	int wid = rand() % 10 + 2;
	int hei = rand() % 10 + 2;
	Pos2 offset(rand() % (MAP_WIDTH - wid), rand() % (MAP_HEIGHT - hei));

	Wall selection = rand() % 2 ? Wall::Blocking : Wall::Cover;

	draw_rect_line(wid, true, offset, Dir::North, selection);
	draw_rect_line(hei, false, offset, Dir::West, selection);
	draw_rect_line(hei, false, Pos2(offset.x + wid, offset.y), Dir::West, selection);
	draw_rect_line(wid, true, Pos2(offset.x, offset.y + hei), Dir::North, selection);
}

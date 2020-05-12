#include <map/Fov.h>
#include "Game.h"

const int MAP_WIDTH = 50;
const int MAP_HEIGHT = 50;
const int SIGHT_RADIUS = 12;

void Game::init() {
	map.reset(Pos2(MAP_WIDTH, MAP_HEIGHT));

	Weapon& blunderbuss = create_weapon("Blunderbuss", 3, 5, RangeType::Short);
	Weapon& musket = create_weapon("Musket", 3, 4, RangeType::Long);
	Weapon& crossbow = create_weapon("Crossbow", 2, 4, RangeType::Long, true);
	Weapon& sword = create_weapon("Sword", 3, 4, RangeType::Melee, true);
	Weapon& dagger = create_weapon("Dagger", 2, 3, RangeType::Melee, true);

	Unit& vanguard = map.create_unit(create_unit_type("Vanguard", 6, 6, 7), Side::You, Pos2(10, 10));
	vanguard.add_weapon(sword);
	vanguard.add_weapon(blunderbuss);

	Unit& assassin = map.create_unit(create_unit_type("Assassin", 7, 6, 6), Side::You, Pos2(10, 11));
	assassin.add_weapon(dagger);
	assassin.add_weapon(crossbow);

	Unit& hunter = map.create_unit(create_unit_type("Hunter", 6, 7, 6), Side::You, Pos2(11, 10));
	hunter.add_weapon(musket);

	UnitType& newt = create_unit_type("Newt", 5, 6, 3);
	UnitType& salamander = create_unit_type("Salamander", 6, 6, 5);
	map.create_unit(newt, Side::Enemy, Pos2(40, 40));
	map.create_unit(newt, Side::Enemy, Pos2(40, 41));
	map.create_unit(salamander, Side::Enemy, Pos2(41, 40));


	Pos2 size = map.get_size();

	for (int i = 0; i < 14; i++) {
		gen_rect();
	}
	for (int i = 0; i < 4; i++) {
		gen_light_circle(Pos2(rando.rand(0, size.x), rando.rand(0, size.y)), rando.rand(2, 6));
	}

	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			for (int dir = 0; dir < 4; ++dir) {
				int r = rando.rand(0, 200);
				if (r == 0) {
					map.set_wall(Pos2(x, y), (Dir)dir, Wall::Blocking);
				} else if (r == 1) {
					map.set_wall(Pos2(x, y), (Dir)dir, Wall::Cover);
				}
			}
		}
	}

	vanguard.set_fov(Fov::calc(map, vanguard.pos(), SIGHT_RADIUS));
	assassin.set_fov(Fov::calc(map, assassin.pos(), SIGHT_RADIUS));
	hunter.set_fov(Fov::calc(map, hunter.pos(), SIGHT_RADIUS));
	init_turn(Side::You);
}

void Game::on_select(Unit* unit) {
	if (unit == nullptr) {
		ui.clear();
		selected_unit = nullptr;
	} else {
		selected_unit = unit;
		update_unit_info();
	}
}

void Game::on_action(Action action) {
	if (action.type == Action::Type::Move) {
		on_move(*action.unit, action.pos, action.segment);
	} else {
		on_attack(*action.unit, *action.weapon, action.pos);
	}
}

void Game::on_attack(Unit& unit, Weapon& weapon, Pos2 pos) {

}

int Game::get_probability(Unit& unit, Weapon& weapon, Unit& target) {
	int probability = 80 + unit.type().aim * 2;

	double distance = (unit.pos() - target.pos()).length();
	switch (weapon.range) {
		case Melee:  probability = distance > 1 ? 0 : probability; break;
		case Short:  probability += (int)(distance * -6) + 20; break;
		case Medium: probability += (int)(distance * -4) + 10; break;
		case Long:   probability += (int)(distance * -2); break;
	}

	auto dirs = (unit.pos() - target.pos()).dirs();
	for (Dir dir : dirs) {
		if (map.has_cover(target.pos(), dir)) {
			probability -= 30;
			break;
		}
	}

	if (!map.is_lit(target.pos())) {
		probability -= 20;
	}

	// TODO: step out, good angles?

	return std::max(probability, 0);
}

void Game::on_move(Unit& unit, Pos2 pos, int segment) {
	int move_cost = segment + 1;
	if (move_cost > unit.ap()) {
		unit.use_stamina(1);
		move_cost--;
	}

	unit.modify_ap(-move_cost);
	map.move(unit, pos);
	unit.set_fov(Fov::calc(map, unit.pos(), SIGHT_RADIUS));

	update_unit_info();
	update();
}

void Game::init_turn(Side new_turn) {
	turn = new_turn;

	for (auto& unit : map.get_units()) {
		if (unit->side() == turn) {
			unit->set_ap(3);
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
			unit->set_ap(0);
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
	ui.clear();
	if (selected_unit == nullptr) return;

	ui.set_entry(NAME, selected_unit->type().name);
	ui.set_entry(HP, "HP: " + std::to_string(selected_unit->hp()));
	ui.set_entry(AP, "AP: " + std::to_string(selected_unit->ap()));
	ui.set_entry(STAMINA, "Stamina: " + std::to_string(selected_unit->stamina()));
	ui.set_entry(MOV, "Mov: " + std::to_string(selected_unit->type().mov));
	ui.set_entry(AIM, "Aim: " + std::to_string(selected_unit->type().aim));

	auto& unit_weapons = selected_unit->get_weapons();
	for (size_t i = 0; i < unit_weapons.size(); i++) {
		ui.set_entry(WEAPONS + i, unit_weapons[i]->name, Action(*selected_unit, *unit_weapons[i]));
	}
}

Wall Game::draw_rect_line(int length, bool dim, Pos2 offset, Dir dir, Wall selection) {
	for (int i = 0; i < length; i++) {
		int r = rando.rand(0, 10);
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
	int wid = rando.rand(2, 12);
	int hei = rando.rand(2, 12);
	Pos2 offset(rando.rand(0, MAP_WIDTH - wid), rando.rand(0, MAP_HEIGHT - hei));

	Wall selection = rando.rand(0, 2) ? Wall::Blocking : Wall::Cover;

	draw_rect_line(wid, true, offset, Dir::North, selection);
	draw_rect_line(hei, false, offset, Dir::West, selection);
	draw_rect_line(hei, false, Pos2(offset.x + wid, offset.y), Dir::West, selection);
	draw_rect_line(wid, true, Pos2(offset.x, offset.y + hei), Dir::North, selection);
}

void Game::gen_light_circle(Pos2 pos, int radius) {
	for (int y = pos.y - radius; y <= pos.y + radius; y++) {
		for (int x = pos.x - radius; x <= pos.x + radius; x++) {
			Pos2 pos0(x, y);
			if (map.in_bounds(pos0) && (pos - pos0).sqr_length() < radius * radius) {
				map.add_light(pos0);
			}
		}
	}
}

UnitType& Game::create_unit_type(std::string name, int mov, int aim, int hp) {
	unit_types.push_back(std::make_unique<UnitType>(std::move(name), mov, aim, hp));
	return *unit_types.back();
}

Weapon& Game::create_weapon(std::string name, int min_damage, int max_damage, RangeType range, bool silent) {
	weapons.push_back(std::make_unique<Weapon>(std::move(name), min_damage, max_damage, range, silent));
	return *weapons.back();
}

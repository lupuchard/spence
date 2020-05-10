#ifndef SPENCE_GAME_H
#define SPENCE_GAME_H

#include "Rando.h"
#include "IEventHandler.h"
#include "UI.h"
#include "Map.h"
#include "Weapon.h"

class Game : public IEventHandler {
public:
	Game(Map& map, UI& ui): map(map), ui(ui), rando(time(nullptr)) { }

	void init();
	void on_select(Unit* unit) override;
	void on_action(Action action) override;
	int get_probability(Unit& unit, Weapon& weapon, Unit& target) override;

private:
	enum Info { NAME, B1, HP, AP, STAMINA, B2, STR, MOV, AIM, B3, WEAPONS };
	void update_unit_info();
	void on_move(Unit& unit, Pos2 pos, int segment);
	void on_attack(Unit& unit, Weapon& weapon, Pos2 pos);

	void init_turn(Side new_turn);
	void enemy_turn();
	void update();

	Wall draw_rect_line(int length, bool dim, Pos2 offset, Dir dir, Wall selection);
	void gen_rect();
	void gen_light_circle(Pos2 pos, int radius);

	UnitType& create_unit_type(std::string name, int mov, int aim, int hp);
	Weapon& create_weapon(std::string name, int min_damage, int max_damage, RangeType range, bool silent = false);

	Map& map;
	UI& ui;
	Rando rando;

	Side turn;
	Unit* selected_unit = nullptr;

	std::vector<std::unique_ptr<UnitType>> unit_types;
	std::vector<std::unique_ptr<Weapon>> weapons;
};

#endif //SPENCE_GAME_H

#ifndef SPENCE_GAME_H
#define SPENCE_GAME_H

#include "../IEventHandler.h"
#include "../UI.h"
#include "../map/Map.h"

class Game : public IEventHandler {
public:
	Game(Map& map, UI& ui): map(map), ui(ui) { }

	void init();
	void on_select(Unit* unit) override;
	void on_move(Unit& unit, Pos2 pos, int segment) override;

private:
	enum Info { NAME, B1, AP, STAMINA, B2, STR, MOV, AIM };
	void update_unit_info();

	void init_turn(Side new_turn);
	void enemy_turn();
	void update();

	Wall draw_rect_line(int length, bool dim, Pos2 offset, Dir dir, Wall selection);
	void gen_rect();

	Map& map;
	UI& ui;

	Side turn;
	Unit* selected_unit = nullptr;

	std::vector<std::unique_ptr<UnitType>> unit_types;
};

#endif //SPENCE_GAME_H

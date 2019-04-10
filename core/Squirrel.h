#ifndef SPENCE_SQUIRREL_H
#define SPENCE_SQUIRREL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "SQUIRREL3/include/squirrel.h"
#include "map/Map.h"
#include "UI.h"

class Squirrel {
public:
	Squirrel(Map& map, UI& ui);
	~Squirrel();
	Squirrel(const Squirrel&) = delete;
	Squirrel& operator=(const Squirrel&) = delete;

	void init();
	void on_select(const Unit* unit);
	void on_move(const Unit& unit, Pos3 pos, int segment);

private:
	SQInteger declare_func(const std::string& name);
	void prepare_call(SQInteger func);
	void complete_call(SQInteger num_params);
	void register_func(const std::string& name, SQFUNCTION func);

	HSQUIRRELVM v;
	SQInteger root_table, state_table;
	SQInteger init_func, select_func, move_func;

	Map& map;
	UI& ui;
};


#endif //SPENCE_SQUIRREL_H

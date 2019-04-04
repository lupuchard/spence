#ifndef SPENCE_SQUIRREL_H
#define SPENCE_SQUIRREL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "SQUIRREL3/include/squirrel.h"
#include "Map.h"

class Squirrel {
public:
	Squirrel(Map& map);
	~Squirrel();
	Squirrel(const Squirrel&) = delete;
	Squirrel& operator=(const Squirrel&) = delete;

	void init();
	void on_select(const Unit& unit);
	Map& map;

private:
	SQInteger declare_func(const std::string name);
	void prepare_call(SQInteger func);
	void complete_call(SQInteger num_params);
	void register_func(const std::string& name, SQFUNCTION func);

	HSQUIRRELVM v;
	SQInteger root_table, state_table;
	SQInteger init_func, select_func;
};


#endif //SPENCE_SQUIRREL_H

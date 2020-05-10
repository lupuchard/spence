#ifndef SPENCE_IEVENTHANDLER_H
#define SPENCE_IEVENTHANDLER_H

#include "Unit.h"

struct Action {
	enum class Type {
		None,
		Move,
		Attack
	};

	Action(): type(Type::None) { }
	Action(Unit& unit, Pos2 pos, int segment):
		unit(&unit), type(Type::Move), pos(pos), segment(segment) { }
	Action(Unit& unit, Weapon& weapon, const Unit* target = nullptr):
		unit(&unit), type(Type::Attack), pos(target == nullptr ? Pos2() : target->pos()), weapon(&weapon) { }


	Type type;

	Unit* unit = nullptr;
	Pos2 pos;
	int segment = 0;
	Weapon* weapon = nullptr;
};

class IEventHandler {
public:
	virtual void on_select(Unit* unit) = 0;
	virtual void on_action(Action action) = 0;
	virtual int get_probability(Unit& unit, Weapon& weapon, Unit& target) = 0;
};

#endif //SPENCE_IEVENTHANDLER_H

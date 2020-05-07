#ifndef SPENCE_IEVENTHANDLER_H
#define SPENCE_IEVENTHANDLER_H

#include "Unit.h"

class IEventHandler {
public:
	virtual void on_select(Unit* unit) = 0;
	virtual void on_move(Unit& unit, Pos2 pos, int segment) = 0;
};

#endif //SPENCE_IEVENTHANDLER_H

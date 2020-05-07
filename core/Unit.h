#ifndef SPENCE_UNIT_H
#define SPENCE_UNIT_H

#include "map/Vec.h"
#include <set>
#include <unordered_map>

enum class Side { None, You, Enemy };

struct UnitType {
	UnitType(std::string name, int mov, int aim, int hp):
		name(std::move(name)), mov(mov), aim(aim), hp(hp) { }
	std::string name;
	int mov;
	int aim;
	int hp;
};

struct Action {
	Action(std::string name, std::string callback, int pos):
		name(std::move(name)), callback(std::move(callback)), pos(pos) { }
	std::string name;
	std::string callback;
	int pos;
};
inline bool operator<(const Action& lhs, const Action& rhs) {
	if (lhs.pos < rhs.pos) return true;
	return lhs.name < rhs.name;
}
inline bool operator==(const Action& lhs, const Action& rhs) {
	return lhs.name == rhs.name;
}
class Unit {
public:
	Unit(const UnitType& type, Side side, Pos2 pos): _type(type), _side(side), _pos(pos) { }

	inline const UnitType& type() const {
		return _type;
	}

	inline Side side() const {
		return _side;
	}

	inline Pos2 pos() const {
		return _pos;
	}
	inline void set_pos(Pos2 pos) {
		_pos = pos;
	}

	inline float move_radius() const {
		return _move_radius;
	}
	inline int move_segments() const {
		return _move_segments;
	}
	inline void set_move(float move_radius, int move_segments) {
		_move_radius = move_radius;
		_move_segments = move_segments;
	}

	inline int stamina() const {
		return _stamina;
	}
	inline void use_stamina(int amount) {
		_stamina -= amount;
	}

	inline int ap() const {
		return _ap;
	}
	inline void use_ap(int amonut) {
		_ap -= amonut;
	}
	inline void reset_ap() {
		_ap = 3;
	}

	inline void add_action(std::string name, std::string callback, int idx = 0) {
		auto iter = action_map.find(name);
		if (iter != action_map.end()) {
			actions.erase(*iter->second);
		}
		const Action* new_action = &*actions.insert(Action(name, std::move(callback), idx)).first;
		action_map[std::move(name)] = new_action;
	}
	inline const std::set<Action>& get_actions(const std::string& action) const {
		return actions;
	}
	inline void remove_action(const std::string& name) {
		auto iter = action_map.find(name);
		if (iter != action_map.end()) {
			actions.erase(*iter->second);
			action_map.erase(iter);
		}
	}
	inline void clear_actions() {
		action_map.clear();
		actions.clear();
	}

private:
	const UnitType& _type;
	Side _side = Side::None;

	float _move_radius = 0;
	int _move_segments = 0;

	Pos2 _pos;

	int _ap = 0;
	int _stamina = 3;

	std::vector<std::string> info;
	std::set<Action> actions;
	std::unordered_map<std::string, const Action*> action_map;
};


#endif //SPENCE_UNIT_H

#ifndef SPENCE_UNIT_H
#define SPENCE_UNIT_H

#include "map/Vec.h"
#include <set>

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
	Unit(Pos3 pos, std::string name): pos(pos), n(std::move(name)) { }
	Pos3 pos;
	float move_radius = 0;
	int move_segments = 0;
	std::vector<std::string> info;

	inline const std::string& name() const {
		return n;
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
	std::string n;
	std::set<Action> actions;
	std::unordered_map<std::string, const Action*> action_map;
};


#endif //SPENCE_UNIT_H

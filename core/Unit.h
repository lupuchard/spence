#ifndef SPENCE_UNIT_H
#define SPENCE_UNIT_H

#include "Vec.h"
#include "Weapon.h"
#include "Grid.h"
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

class Unit {
public:
	Unit(const UnitType& type, Side side, Pos2 pos): _type(type), _side(side), _pos(pos), _hp(type.hp), fov(Pos2()) { }

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

	inline int stamina() const {
		return _stamina;
	}
	inline void use_stamina(int amount) {
		_stamina -= amount;
	}

	inline int hp() const {
		return _hp;
	}

	inline int ap() const {
		return _ap;
	}
	inline void modify_ap(int amount) {
		set_ap(_ap + amount);
	}
	inline void set_ap(int amount) {
		_ap = amount;
		update_move();
	}

	inline void add_weapon(Weapon& weapon) {
		_weapons.push_back(&weapon);
	}
	inline const std::vector<Weapon*> get_weapons() const {
		return _weapons;
	}

	inline void set_fov(Grid<char> fov_grid) {
		fov = std::move(fov_grid);
	}
	inline bool can_see(Pos2 pos) const {
		return fov.get(pos);
	}

private:
	inline void update_move() {
		_move_segments = _ap + (_stamina > 0 ? 1 : 0);
		_move_radius = (float)_type.mov * ((float)_move_segments / 2.f);
	}

	const UnitType& _type;
	Side _side = Side::None;

	std::vector<Weapon*> _weapons;

	float _move_radius = 0;
	int _move_segments = 0;

	Pos2 _pos;

	int _hp = 0;
	int _ap = 0;
	int _stamina = 3;

	Grid<char> fov;
};


#endif //SPENCE_UNIT_H

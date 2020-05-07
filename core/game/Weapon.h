#ifndef SPENCE_WEAPON_H
#define SPENCE_WEAPON_H

enum RangeType {
	Melee, Short, Medium, Long
};

struct Weapon {
	Weapon(int min_damage, int max_damage, RangeType range, bool silent = false):
		min_damage(min_damage), max_damage(max_damage), range(range), silent(silent) { }

	int min_damage;
	int max_damage;
	RangeType range;
	bool silent;
};

#endif //SPENCE_WEAPON_H

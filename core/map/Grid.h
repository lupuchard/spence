#ifndef SPENCE_GRID_H
#define SPENCE_GRID_H

#include <cassert>
#include <vector>
#include "Vec.h"

#define MAX_HEIGHT 255

/// 3D Grid (fixed width/length, unlimited height)
template<typename T>
class Grid {
public:
	explicit Grid(Pos2 size, T def = T(), Pos2 offset = Pos2()): size(size), def(def), offset(offset) {
		assert(size.x >= 0 && size.y >= 0);
		cells.resize((size_t)size.x * (size_t)size.y);
	}
	Pos2 get_size()   const { return size; }
	Pos2 get_offset() const { return offset; }
	bool in_bounds(Pos2 pos) const {
		return pos.x >= offset.x && pos.x < size.x + offset.x &&
		       pos.y >= offset.y && pos.y < size.y + offset.y;
	}
	bool in_bounds(Pos3 pos) const {
		return pos.x >= offset.x && pos.x < size.x + offset.x &&
		       pos.y >= offset.y && pos.y < size.y + offset.y &&
	           pos.z >= 0        && pos.z <= MAX_HEIGHT;
	}

	const std::vector<T>& get(Pos2 pos) const {
		assert(in_bounds(pos));
		return cells[(pos - offset).idx(size.x)];
	}
	T& get(Pos3 pos) {
		assert(in_bounds(pos));
		auto& possible_cells = cells[(pos.flat() - offset).idx(size.x)];
		while (possible_cells.size() <= pos.z) {
			possible_cells.push_back(def);
		}
		return possible_cells[pos.z];
	}
	const T& get(Pos3 pos) const {
		if (!in_bounds(pos)) return def;
		auto& possible_cells = cells[(pos.flat() - offset).idx(size.x)];
		if (possible_cells.size() <= pos.z) {
			return def;
		}
		return possible_cells[pos.z];
	}

	      T& operator[](Pos3 pos)       { return get(pos); }
	const T& operator[](Pos3 pos) const { return get(pos); }

	void set(Pos3 pos, T val) {
		get(pos) = val;
	}

private:
	Pos2 size, offset;
	T def;
	std::vector<std::vector<T>> cells;
};

#endif //SPENCE_GRID_H

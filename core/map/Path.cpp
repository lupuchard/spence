#include "Path.h"
#include <queue>
#include <algorithm>

struct Compare {
	bool operator() (PathNode* a, PathNode* b) {
		return a->dist < b->dist;
	}
};

PathMap Path::calc(const Map& map, Pos3 pos, float radius, PathSettings& settings) {
	int radius_i = (int)std::ceil(radius);
	Pos2 top_left = (pos.flat() - Pos2(radius_i)).max(Pos2());
	Pos2 bot_rite = (pos.flat() + Pos2(radius_i)).max(map.get_size());
	Grid<PathNode> path_grid(bot_rite - top_left, PathNode(), top_left);
	PathNode& start = path_grid.get(pos);
	start.pos = pos;
	start.dist = 0;
	std::priority_queue<PathNode*, std::vector<PathNode*>, Compare> active_q;
	active_q.push(&start);

	while (!active_q.empty()) {
		PathNode& current = *active_q.top();
		active_q.pop();
		if (current.state == PathNode::CLOSED) {
			continue;
		} else if (current.dist > radius) {
			current.state = PathNode::INACCESSABLE;
			continue;
		} else {
			current.state = PathNode::ACCESSABLE;
		}
		std::vector<std::pair<Pos3, float>> neighbors;
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				if (x == 0 && y == 0) continue;
				Pos3 pos0 = Pos3(x, y, 0) + current.pos;
				if (!path_grid.in_bounds(pos0)) continue;
				std::vector<Dir> dirs = Pos2(x, y).dirs();
				if (pos0.z > 0 && !map.has_tile(pos0)) {
					if (dirs.size() == 1) {
						pos0.z--;
						neighbors.emplace_back(pos0, settings.drop_cost);
						if (map.has_cover(current.pos, dirs[0])) {
							neighbors.back().second += settings.step_cost;
						}
					}
					continue;
				}
				bool blocked = false;
				bool cover   = false;
				for (Dir dir : dirs) {
					if (map.is_blocked(current.pos, dir)) {
						blocked = true;
						break;
					} else if (!map.is_flat(current.pos, dir)) {
						if (dirs.size() == 2) {
							blocked = true;
							break;
						}
						cover = true;
					}
				}
				if (dirs.size() == 2 && !blocked) {
					// check if cutting corners (not allowed)
					for (Dir dir : dirs) {
						if (!map.is_flat(pos0, flip(dir))) {
							blocked = true;
							break;
						}
					}
				}
				if (blocked) {
					if (dirs.size() == 1 && map.get_wall(current.pos, dirs[0]) == Wall::Climbable) {
						pos0.z++;
						neighbors.emplace_back(pos0, settings.climb_cost);
					}
					continue;
				}
				if (cover) {
					neighbors.emplace_back(pos0, settings.step_cost);
				} else if (dirs.size() == 1) {
					neighbors.emplace_back(pos0, settings.ortho_cost);
				} else {
					neighbors.emplace_back(pos0, settings.diag_cost);
				}
			}
		}
		for (auto neighbor : neighbors) {
			auto& neighbor_node = path_grid.get(neighbor.first);
			if (neighbor_node.state == PathNode::CLOSED) continue;
			float alt = current.dist + neighbor.second;
			if (alt < neighbor_node.dist) {
				neighbor_node.pos = neighbor.first;
				neighbor_node.dist = alt;
				neighbor_node.parent = current.pos;
				active_q.push(&neighbor_node);
			}
		}
	}
	return PathMap(pos, path_grid);
}

bool PathMap::can_access(Pos3 pos) {
	if (!grid.in_bounds(pos)) return false;
	return grid.get(pos).state == PathNode::ACCESSABLE;
}

std::vector<Pos3> Path::to(PathMap& pathmap, Pos3 dest) {
	if (dest.z == -1) return std::vector<Pos3>(1, dest);
	Pos3 prev = dest;
	Pos3 cur = pathmap.grid.get(dest).parent;
	if (cur.z == -1) return std::vector<Pos3> { cur, prev };
	while (true) {
		Pos3 next = pathmap.grid.get(cur).parent;
		if (next.z == -1) break;
		if (!in_line(prev, cur, next)) {
			prev = pathmap.grid.get(prev).parent;
			if (prev == cur) cur = next;
			continue;
		}
		cur = next;
	}

	std::vector<Pos3> path;
	for (Pos3 pos = dest; pos.z != -1; pos = pathmap.grid.get(pos).parent) {
		path.push_back(pos);
	}
	std::reverse(path.begin(), path.end());
	return path;
}

bool Path::in_line(Pos3 a, Pos3 b, Pos3 c) {
	if (a.z != b.z || b.z != c.z) return false;
	Pos3 ab = a - b;
	Pos3 ac = a - c;
	if (ab.y == 0) return ac.y == 0;
	return (ab.x / ab.y == ac.x / ac.y && ab.x % ab.y == ac.x % ac.y);
}

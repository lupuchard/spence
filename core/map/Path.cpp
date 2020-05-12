#include "Path.h"
#include <queue>
#include <algorithm>

struct Compare {
	bool operator() (PathNode* a, PathNode* b) {
		return a->dist < b->dist;
	}
};

int calc_segment(float dist, float radius, int num_segments) {
	return std::max(std::min((int)(((dist - 0.5) / radius) * num_segments), num_segments - 1), 0);
}

PathMap Path::calc(const Map& map, Pos2 pos, float radius, PathSettings& settings, int num_segments) {
	radius++;
	int radius_i = (int)std::ceil(radius);
	Pos2 top_left = (pos - Pos2(radius_i)).max(Pos2());
	Pos2 bot_rite = (pos + Pos2(radius_i)).max(map.get_size());
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
		std::vector<std::pair<Pos2, float>> neighbors;
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				if (x == 0 && y == 0) continue;
				Pos2 pos0 = Pos2(x, y) + current.pos;
				if (!path_grid.in_bounds(pos0)) continue;
				std::vector<Dir> dirs = Pos2(x, y).dirs();
				bool blocked = false;
				bool cover   = false;
				for (Dir dir : dirs) {
					Wall wall = map.get_wall(current.pos, dir);
					if (wall == Wall::Blocking) {
						blocked = true;
						break;
					} else if (wall == Wall::Cover) {
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
						if (map.get_wall(pos0, flip(dir)) != Wall::None) {
							blocked = true;
							break;
						}
					}
				}
				if (blocked) {
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
				neighbor_node.segment = calc_segment(alt, radius, num_segments);
				neighbor_node.parent = current.pos;
				active_q.push(&neighbor_node);
			}
		}
	}
	return PathMap(pos, path_grid);
}

bool PathMap::can_access(Pos2 pos) {
	if (!grid.in_bounds(pos)) return false;
	return grid.get(pos).state == PathNode::ACCESSABLE;
}

PathNode* PathMap::get_node(Pos2 pos) {
	if (!grid.in_bounds(pos)) return nullptr;
	return &grid.get(pos);
}

std::vector<Pos2> Path::to(PathMap& pathmap, Pos2 dest) {
	if (dest.y == -1) return std::vector<Pos2>(1, dest);
	Pos2 prev = dest;
	Pos2 cur = pathmap.grid.get(dest).parent;
	if (cur.y == -1) return std::vector<Pos2> { cur, prev };
	while (true) {
		Pos2 next = pathmap.grid.get(cur).parent;
		if (next.y == -1) break;
		if (!in_line(prev, cur, next)) {
			prev = pathmap.grid.get(prev).parent;
			if (prev == cur) cur = next;
			continue;
		}
		cur = next;
	}

	std::vector<Pos2> path;
	for (Pos2 pos = dest; pos.y != -1; pos = pathmap.grid.get(pos).parent) {
		path.push_back(pos);
	}
	std::reverse(path.begin(), path.end());
	return path;
}

bool Path::in_line(Pos2 a, Pos2 b, Pos2 c) {
	Pos2 ab = a - b;
	Pos2 ac = a - c;
	if (ab.y == 0) return ac.y == 0;
	return (ab.x / ab.y == ac.x / ac.y && ab.x % ab.y == ac.x % ac.y);
}

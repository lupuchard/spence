#include "SFMLRenderer.h"

SFMLRenderer::SFMLRenderer(Pos2 screen_size, float tile_size, const Map& map, Squirrel& squirrel):
		map(map), squirrel(squirrel), tile_size(tile_size) {
	std::string title = "is game";
	sf::String sf_title = sf::String(title);
	window.create(sf::VideoMode((unsigned)screen_size.x, (unsigned)screen_size.y), sf_title);
}

Vec2 SFMLRenderer::render_pos() {
	Vec2 offset = Vec2(window.getSize().x, window.getSize().y) / tile_size / 2;
	return -Vec2(map.get_size()) / 2 + pos + offset;
}

void SFMLRenderer::render() {
	if (paused) return;
	window.clear();
	Pos2 size = map.get_size();
	Vec2 r = render_pos();

	if (selected) {
		for (int y = 0; y < size.y; y++) {
			for (int x = 0; x < size.x; x++) {
				if (!path_map.can_access(Pos3(x, y, height))) continue;
				draw_rect(Vec2(x, y) + r, Vec2(1, 1), sf::Color(63, 63, 0));
			}
		}
		for (int i = 0; i < (int)path.size() - 1; i++) {
			if (path[i].z != height || path[i + 1].z != height) continue;
			draw_line_rounded(Vec2(path[i    ].flat()) + r + 0.5,
			                  Vec2(path[i + 1].flat()) + r + 0.5,
			                  0.1, sf::Color::Yellow);
		}
	}

	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			const Tile& tile = map.get_tile(Pos3(x, y, height));
			Vec2 render_pos0 = r + Vec2(x, y);
			for (Dir dir = (Dir)0; dir < 4; ++dir) {
				Wall wall = tile.walls[dir];
				if (wall != Wall::None) {
					Vec2 start(Pos2(dir).x ==  1, Pos2(dir).y ==  1);
					Vec2   end(Pos2(dir).x != -1, Pos2(dir).y != -1);
					sf::Color color = sf::Color::White;
					float thickness = 0.1;
					if (wall == Wall::InnerCover || wall == Wall::OuterCover) {
						thickness = 0.05;
						color = sf::Color(127, 127, 127);
					} else if (wall == Wall::Climbable) {
						color = sf::Color::Green;
					}
					draw_line(start + render_pos0, end + render_pos0, thickness, color);
				}
			}
		}
	}

	for (auto pair : units) {
		sf::Color color = selected ? sf::Color(255, 127, 127) : sf::Color::Red;
		float radius = hovering ? 0.35f : 0.3f;
		draw_circle(Vec2(pair.first->pos.flat()) + 0.5 + r, radius, color);
	}

	window.display();
}

void SFMLRenderer::reset_grid(const Grid<Tile>& g) {
	pos = Vec2(0, 0);
	height = 0;
}

void SFMLRenderer::draw_line(Vec2 start, Vec2 end, float thickness, sf::Color color) {
	float length = (float)(end - start).length();
	float angle  = (float)((end - start).angle() * 360 / TAU);
	sf::RectangleShape line(sf::Vector2f((thickness + length) * tile_size, thickness * tile_size));
	line.setOrigin(thickness / 2 * tile_size, thickness / 2 * tile_size);
	line.setPosition((float)start.x * tile_size, (float)start.y * tile_size);
	line.setRotation(angle);
	line.setFillColor(color);
	window.draw(line);
}
void SFMLRenderer::draw_line_rounded(Vec2 start, Vec2 end, float thickness, sf::Color color) {
	float length = (float)(end - start).length();
	float angle  = (float)((end - start).angle() * 360 / TAU);
	sf::RectangleShape line(sf::Vector2f(length * tile_size, thickness * tile_size));
	line.setOrigin(0, thickness / 2 * tile_size);
	line.setPosition((float)start.x * tile_size, (float)start.y * tile_size);
	line.setRotation(angle);
	line.setFillColor(color);
	draw_circle(start, thickness / 2, color);
	draw_circle(end, thickness / 2, color);
	window.draw(line);
}
void SFMLRenderer::draw_circle(Vec2 pos, float radius, sf::Color color) {
	radius *= tile_size;
	pos *= tile_size;
	sf::CircleShape circle(radius);
	circle.setOrigin(radius, radius);
	circle.setPosition((float)pos.x, (float)pos.y);
	circle.setFillColor(color);
	window.draw(circle);
}
void SFMLRenderer::draw_rect(Vec2 pos, Vec2 size, sf::Color color) {
	sf::RectangleShape rect(sf::Vector2f((float)size.x, (float)size.y) * tile_size);
	rect.setPosition(sf::Vector2f((float)pos.x, (float)pos.y) * tile_size);
	rect.setFillColor(color);
	window.draw(rect);
}

void SFMLRenderer::pause() {
	paused = true;
}
void SFMLRenderer::unpause() {
	paused = false;
}

sf::RenderWindow& SFMLRenderer::get_window() {
	return window;
}

void SFMLRenderer::mouse_move(Pos2 mouse_pos) {
	if (dragging) {
		Vec2 dist = Vec2(mouse_pos - prev_mouse_pos) / tile_size;
		if (!dragged && dist.length() < MIN_DRAG) return;
		pos += dist;
		prev_mouse_pos = mouse_pos;
		dragged = true;
	}
	map_mouse_pos = Pos3(Vec2(mouse_pos) / tile_size - render_pos(), height);
	hovering = map.get_unit(map_mouse_pos);
	if (selected) {
		if (path_map.can_access(map_mouse_pos)) {
			path = map.to(path_map, map_mouse_pos);
		} else {
			path.clear();
		}
	}
}
void SFMLRenderer::mouse_press(Pos2 mouse_pos, Mouse mouse) {
	if (mouse == Mouse::LEFT) {
		dragging = true;
		dragged = false;
		prev_mouse_pos = mouse_pos;
	}
}
void SFMLRenderer::mouse_release(Pos2, Mouse mouse) {
	if (mouse == Mouse::LEFT) {
		if (!dragged) {
			if (selected == hovering || hovering == nullptr) {
				selected = nullptr;
			} else {
				selected = hovering;
				squirrel.on_select(*selected);
				auto settings = Map::PathSettings();
				settings.diag_cost = 1.4;
				settings.step_cost = 2;
				path_map = map.calc_pathmap(selected->pos, selected->move_radius, settings);
			}
		}
		dragging = false;
	}
}
void SFMLRenderer::mouse_scroll(float amount) {
	fheight -= amount;
	height = std::max((int)fheight, 0);
}

void SFMLRenderer::add_unit(const Unit& unit) {
	units[&unit] = UnitGraphic(Vec3(unit.pos));
}
void SFMLRenderer::remove_unit(const Unit& unit) {
	units.erase(&unit);
}

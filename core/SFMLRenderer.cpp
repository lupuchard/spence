#include "SFMLRenderer.h"

#define RIGHT_PANE_WIDTH 200
#define INFO_MARGIN 20
#define INFO_HEIGHT 20

const sf::Color RED(255, 0, 0);
const sf::Color ORANGE(255, 127, 0);
const sf::Color YELLOW(255, 255, 0);
const sf::Color CHARTREUSE(127, 255, 0);
const sf::Color GREEN(0, 255, 0);
const sf::Color SPRING(0, 255, 127);
const sf::Color CYAN(0, 255, 255);
const sf::Color AZURE(0, 127, 255);
const sf::Color BLUE(0, 0, 255);
const sf::Color VIOLET(127, 0, 255);
const sf::Color MAGENTA(255, 0, 255);
const sf::Color ROSE(255, 0, 127);

sf::Color darken(const sf::Color& color) {
	return sf::Color(color.r / 2, color.g / 2, color.b / 2);
}

SFMLRenderer::SFMLRenderer(Pos2 screen_size, float tile_size, const Map& map, UI& ui, Squirrel& squirrel):
	squirrel(squirrel), ui(ui), map(map), tile_size(tile_size) {

	if (!font.loadFromFile("resources/Slabo27px-Regular.ttf")) {
		std::cout << "Couldn't load font" << std::endl;
	}

	sf::String sf_title = sf::String("is game");
	window.create(sf::VideoMode((unsigned)screen_size.x, (unsigned)screen_size.y), sf_title);
	ui_texture.create(RIGHT_PANE_WIDTH, screen_size.y);
}

Vec2 SFMLRenderer::render_pos() {
	Vec2 offset = Vec2(window.getSize().x, window.getSize().y) / tile_size / 2;
	return -Vec2(map.get_size()) / 2 + pos + offset;
}

sf::Color get_segment_color(int segment, int num_segments) {
	const static sf::Color segment_colors[] = { MAGENTA, VIOLET, BLUE, AZURE, YELLOW, darken(YELLOW) };
	sf::Color color = segment_colors[std::min(std::max(segment + (6 - num_segments), 0), 5)];
	return darken(darken(color));
}

void SFMLRenderer::render() {
	if (paused) return;
	window.clear();
	Pos2 size = map.get_size();
	Vec2 r = render_pos();

	draw_rect(Vec2(0, 0) + r, size, sf::Color(15, 15, 15));

	if (selected) {
		float max = 0;
		float min = 99999;
		for (int y = 0; y < size.y; y++) {
			for (int x = 0; x < size.x; x++) {
				PathNode* path_node = path_map.get_node(Pos3(x, y, height));
				if (path_node == nullptr || path_node->state != PathNode::ACCESSABLE) continue;
				draw_rect(Vec2(x, y) + r, Vec2(1, 1), get_segment_color(path_node->segment, selected->move_segments));
				if (path_node->dist > max) max = path_node->dist;
				if (path_node->dist < min) min = path_node->dist;
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

	if (ui.has_changed()) {
		ui_texture.clear(sf::Color(31, 31, 31));
		for (size_t i = 0; i < ui.num_entries(); i++) {
			sf::Text text(ui.get_text(i), font, INFO_HEIGHT);
			text.setPosition(INFO_MARGIN, i * INFO_HEIGHT + INFO_MARGIN);
			text.setColor(sf::Color(223, 223, 223));
			ui_texture.draw(text);
		}
		ui_texture.display();
	}

	sf::Sprite ui_sprite(ui_texture.getTexture());
	ui_sprite.setPosition(window.getSize().x - RIGHT_PANE_WIDTH, 0);
	window.draw(ui_sprite);

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
			path = Path::to(path_map, map_mouse_pos);
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
	} else if (mouse == Mouse::RIGHT && !dragging) {
		PathNode* path_node = path_map.get_node(map_mouse_pos);
		if (path_node == nullptr || path_node->state != PathNode::ACCESSABLE) return;
		squirrel.on_move(*selected, map_mouse_pos, path_node->segment);
		prev_mouse_pos = mouse_pos;
		selected = nullptr;
	}
}
void SFMLRenderer::mouse_release(Pos2, Mouse mouse) {
	if (mouse == Mouse::LEFT) {
		if (!dragged) {
			if (selected == hovering || hovering == nullptr) {
				selected = nullptr;
				squirrel.on_select(nullptr);
			} else {
				selected = hovering;
				squirrel.on_select(selected);
				PathSettings settings;
				settings.diag_cost = 1.4;
				settings.step_cost = 2;
				path_map = Path::calc(map, selected->pos, selected->move_radius, settings, selected->move_segments);
			}
		}
		dragging = false;
	}
}
void SFMLRenderer::mouse_scroll(float amount) {
	if (amount > 0) {
		tile_size *= 2;
	} else {
		tile_size /= 2;
	}
	//fheight -= amount;
	//height = std::max((int)fheight, 0);
}

void SFMLRenderer::add_unit(const Unit& unit) {
	units[&unit] = UnitGraphic(Vec3(unit.pos));
}
void SFMLRenderer::remove_unit(const Unit& unit) {
	units.erase(&unit);
}

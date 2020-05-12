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

sf::Color transparent(const sf::Color& color, uint8_t alpha) {
	return sf::Color(color.r, color.g, color.b, alpha);
}

SFMLRenderer::SFMLRenderer(Pos2 screen_size, float tile_size, Map& map, UI& ui, IEventHandler& handler):
	vertex_arr(sf::PrimitiveType::Triangles), ui(ui), map(map), handler(handler), tile_size(tile_size) {

	if (!font.loadFromFile("resources/Slabo27px-Regular.ttf")) {
		std::cout << "Couldn't load font" << std::endl;
	}

	sf::String sf_title = sf::String("is game");
	window.create(sf::VideoMode((unsigned)screen_size.x, (unsigned)screen_size.y), sf_title);
	ui_texture.create(RIGHT_PANE_WIDTH, screen_size.y);

	prev_frame = std::chrono::steady_clock::now();
}

void SFMLRenderer::update_render_pos() {
	Vec2 offset = Vec2(window.getSize().x, window.getSize().y) / tile_size / 2;
	render_pos = -Vec2(map.get_size()) / 2 + gridPos + offset;
}

sf::Color get_segment_color(int segment, int num_segments) {
	const static sf::Color segment_colors[] = { BLUE, AZURE, ORANGE, darken(ORANGE) };
	sf::Color color = segment_colors[segment];
	return darken(darken(color));
}

void SFMLRenderer::render() {
	if (paused) return;

	window.clear();
	draw_rect(Vec2(0, 0), map.get_size(), sf::Color(31, 31, 31));
	render_movement();
	render_cover();
	render_units();
	render_light();
	render_fov();
	render_ui();
	window.display();
}

void SFMLRenderer::render_cover() {
	Pos2 size = map.get_size();
	vertex_arr.clear();
	sf::Color wall_color = sf::Color::White;
	sf::Color cover_color(127, 127, 127);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			const Tile& tile = map.get_tile(Pos2(x, y));

			if (tile.north_wall == Wall::Blocking) {
				add_quad(Vec2(x, y) - 0.05, Vec2(1.1, 0.1), wall_color);
			} else if (tile.north_wall == Wall::Cover) {
				add_quad(Vec2(x, y) - 0.025, Vec2(1.05, 0.05), cover_color);
			}

			if (tile.west_wall == Wall::Blocking) {
				add_quad(Vec2(x, y) - 0.05, Vec2(0.1, 1.1), wall_color);
			} else if (tile.west_wall == Wall::Cover) {
				add_quad(Vec2(x, y) - 0.025, Vec2(0.05, 1.05), cover_color);
			}
		}
	}
	window.draw(vertex_arr);
}

void SFMLRenderer::render_light() {
	Pos2 size = map.get_size();
	vertex_arr.clear();
	sf::Color color(255, 255, 127, 31);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			if (map.is_lit(Pos2(x, y))) {
				add_quad(Pos2(x, y), color);
			}
		}
	}
	window.draw(vertex_arr);
}

void SFMLRenderer::render_units() {
	auto& units = map.get_units();
	for (auto& unit : units) {
		sf::Color color = (unit.get() == selected) ? sf::Color(255, 127, 127) : sf::Color::Red;
		float radius = (unit.get() == hovering) ? 0.35f : 0.3f;
		draw_circle(Vec2(unit->pos()) + 0.5, radius, color);

		if (unit.get() == hovering && ui_selected != -1) {
			const Action& action = ui.get_action(ui_selected);
			if (action.type != Action::Type::Attack) continue;
			int probability;
			auto iter = hit_probabilities.find(unit.get());
			if (iter == hit_probabilities.end()) {
				if (unit->side() == selected->side()) {
					probability = -1;
				} else {
					probability = handler.get_probability(*selected, *action.weapon, *unit);
				}
				hit_probabilities[unit.get()] = probability;
			} else {
				probability = iter->second;
			}

			if (probability != -1) {
				draw_line_rounded(Vec2(selected->pos()) + 0.5, Vec2(unit->pos()) + 0.5, 0.1, ORANGE);
			}
		}
	}
}

void SFMLRenderer::render_fov() {
	Pos2 size = map.get_size();
	vertex_arr.clear();
	sf::Color color(0, 0, 0, 190);
	size_t i;
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			bool can_see = false;
			for (auto& unit : map.get_units()) {
				if (unit->side() == Side::You && unit->can_see(Pos2(x, y))) {
					can_see = true;
					break;
				}
			}

			if (!can_see) {
				add_quad(Pos2(x, y), color);
			}
		}
	}
	window.draw(vertex_arr);
}

void SFMLRenderer::render_movement() {
	if (selected && selected->side() == Side::You) {
		float max = 0;
		float min = 99999;

		Pos2 size = map.get_size();
		sf::VertexArray arr(sf::PrimitiveType::Triangles);
		vertex_arr.clear();
		for (int y = 0; y < size.y; y++) {
			for (int x = 0; x < size.x; x++) {
				PathNode* path_node = path_map.get_node(Pos2(x, y));
				if (path_node == nullptr || path_node->state != PathNode::ACCESSABLE) continue;
				add_quad(Pos2(x, y), get_segment_color(path_node->segment, selected->move_segments()));
				if (path_node->dist > max) max = path_node->dist;
				if (path_node->dist < min) min = path_node->dist;
			}
		}
		window.draw(vertex_arr);

		for (int i = 0; i < (int)path.size() - 1; i++) {
			draw_line_rounded(Vec2(path[i]) + 0.5, Vec2(path[i + 1]) + 0.5, 0.1, sf::Color::Yellow);
		}
	}
}

void SFMLRenderer::render_ui() {
	if (ui.has_changed()) {
		ui_texture.clear(sf::Color(31, 31, 31));
		for (size_t i = 0; i < ui.num_entries(); i++) {
			sf::Text text(ui.get_text(i), font, INFO_HEIGHT);
			text.setPosition(INFO_MARGIN, (float)(i * INFO_HEIGHT + INFO_MARGIN));
			text.setFillColor(sf::Color(223, 223, 223));
			if (i == ui_hovering || i == ui_selected) {
				auto bounds = text.getGlobalBounds();
				sf::RectangleShape rect(sf::Vector2f(bounds.width + 10, INFO_HEIGHT));
				rect.setPosition(bounds.left - 5, text.getPosition().y + 3);
				rect.setFillColor(i == ui_selected ? AZURE : darken(AZURE));
				ui_texture.draw(rect);
			}
			ui_texture.draw(text);
		}
		ui_texture.display();
	}

	sf::Sprite ui_sprite(ui_texture.getTexture());
	ui_sprite.setPosition((float)(window.getSize().x - RIGHT_PANE_WIDTH), 0);
	window.draw(ui_sprite);

	auto now = std::chrono::steady_clock::now();
	double diff = std::chrono::duration_cast<std::chrono::microseconds>(now - prev_frame).count() / 1000000.;
	num_frames++;
	average_frame_diff = diff / num_frames + average_frame_diff / num_frames * (num_frames - 1);
	if (num_frames == 100) {
		num_frames = 0;
		fps_text = sf::Text(std::to_string((int)(1. / average_frame_diff)) + " fps", font, INFO_HEIGHT);
		fps_text.setPosition(10, 10);
	}
	window.draw(fps_text);
	prev_frame = now;
}

void SFMLRenderer::add_quad(Vec2 pos, sf::Color color) {
	add_quad(pos, Vec2(1, 1), color);
}

void SFMLRenderer::add_quad(Vec2 pos, Vec2 size, sf::Color color) {
	Vec2 top_left = (Vec2(pos.x, pos.y) + render_pos) * tile_size;
	Vec2 bot_rite = (Vec2(pos.x + size.x, pos.y + size.y) + render_pos) * tile_size;
	vertex_arr.append(sf::Vertex(sf::Vector2f(top_left.x, top_left.y), color));
	vertex_arr.append(sf::Vertex(sf::Vector2f(bot_rite.x, top_left.y), color));
	vertex_arr.append(sf::Vertex(sf::Vector2f(top_left.x, bot_rite.y), color));
	vertex_arr.append(sf::Vertex(sf::Vector2f(top_left.x, bot_rite.y), color));
	vertex_arr.append(sf::Vertex(sf::Vector2f(bot_rite.x, top_left.y), color));
	vertex_arr.append(sf::Vertex(sf::Vector2f(bot_rite.x, bot_rite.y), color));
}

void SFMLRenderer::reset_grid(const Grid<Tile>& g) {
	gridPos = Vec2(0, 0);
	update_render_pos();
}

void SFMLRenderer::draw_line(Vec2 start, Vec2 end, float thickness, sf::Color color) {
	start += render_pos;
	end += render_pos;
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
	start += render_pos;
	end += render_pos;
	float length = (float)(end - start).length();
	float angle  = (float)((end - start).angle() * 360 / TAU);
	sf::RectangleShape line(sf::Vector2f(length * tile_size, thickness * tile_size));
	line.setOrigin(0, thickness / 2 * tile_size);
	line.setPosition((float)start.x * tile_size, (float)start.y * tile_size);
	line.setRotation(angle);
	line.setFillColor(color);
	draw_circle(start - render_pos, thickness / 2, color);
	draw_circle(end - render_pos, thickness / 2, color);
	window.draw(line);
}
void SFMLRenderer::draw_circle(Vec2 pos, float radius, sf::Color color) {
	radius *= tile_size;
	pos += render_pos;
	pos *= tile_size;
	sf::CircleShape circle(radius);
	circle.setOrigin(radius, radius);
	circle.setPosition((float)pos.x, (float)pos.y);
	circle.setFillColor(color);
	window.draw(circle);
}
void SFMLRenderer::draw_rect(Vec2 pos, Vec2 size, sf::Color color) {
	pos += render_pos;
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
		gridPos += dist;
		prev_mouse_pos = mouse_pos;
		dragged = true;
		update_render_pos();
	}
	map_mouse_pos = Pos2(Vec2(mouse_pos) / tile_size - render_pos);

	int new_ui_hovering = -1;
	if (mouse_pos.x > window.getSize().x - RIGHT_PANE_WIDTH + INFO_MARGIN) {
		int ui_pos = (mouse_pos.y - INFO_MARGIN - 3) / INFO_HEIGHT;
		if (ui.has_action(ui_pos)) {
			new_ui_hovering = ui_pos;
		}
	}
	if (new_ui_hovering != ui_hovering) {
		ui.set_has_changed();
		ui_hovering = new_ui_hovering;
	}

	if (map.in_bounds(Vec2(map_mouse_pos.x, map_mouse_pos.y))) {
		hovering = map.get_unit(map_mouse_pos);
	}

	if (selected) {
		if (selected->side() == Side::You && hovering == nullptr && path_map.can_access(map_mouse_pos)) {
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
	} else if (mouse == Mouse::RIGHT && !dragging && selected != nullptr) {
		PathNode* path_node = path_map.get_node(map_mouse_pos);
		if (path_node == nullptr || path_node->state != PathNode::ACCESSABLE) return;
		handler.on_action(Action(*selected, map_mouse_pos, path_node->segment));
		prev_mouse_pos = mouse_pos;
		selected = nullptr;
	}
}
void SFMLRenderer::mouse_release(Pos2, Mouse mouse) {
	if (mouse == Mouse::LEFT) {
		if (!dragged) {
			ui.set_has_changed();
			ui_selected = -1;
			if (ui_hovering != -1) {
				ui_selected = ui_hovering;
				hit_probabilities.clear();
			} else if (selected == hovering || hovering == nullptr) {
				selected = nullptr;
				handler.on_select(nullptr);
			} else {
				selected = hovering;
				handler.on_select(selected);

				if (selected->side() == Side::You) {
					PathSettings settings;
					settings.diag_cost = 1.4;
					settings.step_cost = 2;
					path_map = Path::calc(map, selected->pos(), selected->move_radius(), settings, selected->move_segments());
					path.clear();
				}
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
	update_render_pos();
}

#ifndef SPENCE_SFMLRENDERER_H
#define SPENCE_SFMLRENDERER_H

#include <unordered_map>
#include <set>
#include <memory>
#include <SFML/Graphics.hpp>
#include <deque>
#include <chrono>
#include "Renderer.h"
#include "map/Path.h"
#include "IEventHandler.h"
#include "UI.h"

class SFMLRenderer: public Renderer {
public:
	SFMLRenderer(Pos2 screen_size, float tile_size, Map& map, UI& ui, IEventHandler& handler);
	void render() override;
	void reset_grid(const Grid<Tile>& grid) override;

	void mouse_move(Pos2 pos) override;
	void mouse_press(Pos2 pos, Mouse mouse) override;
	void mouse_release(Pos2 pos, Mouse mouse) override;
	void mouse_scroll(float amount) override;

	void pause() override;
	void unpause() override;

	sf::RenderWindow& get_window();

private:
	void render_movement();
	void render_cover();
	void render_light();
	void render_units();
	void render_fov();
	void render_ui();

	void add_quad(Vec2 pos, sf::Color color);
	void add_quad(Vec2 pos, Vec2 size, sf::Color color);

	void draw_line(Vec2 start, Vec2 end, float thickness, sf::Color color);
	void draw_line_rounded(Vec2 start, Vec2 end, float thickness, sf::Color color);
	void draw_circle(Vec2 pos, float radius, sf::Color color);
	void draw_rect(Vec2 pos, Vec2 size, sf::Color color);
	void draw_text(sf::Text text);
	void update_render_pos();

	sf::Font font;
	sf::RenderTexture ui_texture;

	bool paused = false;
	sf::RenderWindow window;
	float tile_size;
	Vec2 render_pos;
	sf::VertexArray vertex_arr;

	UI& ui;
	Map& map;
	IEventHandler& handler;

	Vec2 gridPos;

	const float MIN_DRAG = 0.1;
	bool dragging = false;
	bool dragged  = false;
	Pos2 prev_mouse_pos;
	Pos2 map_mouse_pos;
	Unit* hovering = nullptr;
	Unit* selected = nullptr;
	PathMap path_map;
	std::vector<Pos2> path;

	int ui_hovering = -1;
	int ui_selected = -1;
	std::unordered_map<Unit*, int> hit_probabilities;

	struct Movement {
		Vec3 start, end;
		double time_passed;
		std::unique_ptr<Movement> next;
	};
	std::set<Movement> movements;

	double average_frame_diff = 1;
	int num_frames = 0;
	std::chrono::steady_clock::time_point prev_frame;
	sf::Text fps_text;
};


#endif //SPENCE_SFMLRENDERER_H

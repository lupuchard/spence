#ifndef SPENCE_SFMLRENDERER_H
#define SPENCE_SFMLRENDERER_H

#include <unordered_map>
#include <set>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Renderer.h"
#include "map/Path.h"
#include "Squirrel.h"

class SFMLRenderer: public Renderer {
public:
	SFMLRenderer(Pos2 screen_size, float tile_size, const Map& map, Squirrel& squirrel);
	void render() override;
	void reset_grid(const Grid<Tile>& grid) override;

	void mouse_move(Pos2 pos) override;
	void mouse_press(Pos2 pos, Mouse mouse) override;
	void mouse_release(Pos2 pos, Mouse mouse) override;
	void mouse_scroll(float amount) override;

	void pause() override;
	void unpause() override;

	void add_unit(const Unit& unit) override;
	void remove_unit(const Unit& unit) override;

	sf::RenderWindow& get_window();

private:
	void draw_line(Vec2 start, Vec2 end, float thickness, sf::Color color);
	void draw_line_rounded(Vec2 start, Vec2 end, float thickness, sf::Color color);
	void draw_circle(Vec2 pos, float radius, sf::Color color);
	void draw_rect(Vec2 pos, Vec2 size, sf::Color color);
	Vec2 render_pos();

	bool paused = false;
	sf::RenderWindow window;
	float tile_size;

	const Map& map;
	Vec2 pos;
	int height = 0;
	float fheight = 0.5;

	Squirrel& squirrel;
	const float MIN_DRAG = 0.1;
	bool dragging = false;
	bool dragged  = false;
	Pos2 prev_mouse_pos;
	Pos3 map_mouse_pos;
	const Unit* hovering = nullptr;
	const Unit* selected = nullptr;
	PathMap path_map;
	std::vector<Pos3> path;

	struct UnitGraphic {
		UnitGraphic() = default;
		UnitGraphic(Vec3 pos): pos(pos) { }
		Vec3 pos;
	};
	std::unordered_map<const Unit*, UnitGraphic> units;

	struct Movement {
		Vec3 start, end;
		double time_passed;
		std::unique_ptr<Movement> next;
	};
	std::set<Movement> movements;
};


#endif //SPENCE_SFMLRENDERER_H

#ifndef SPENCE_RENDERER_H
#define SPENCE_RENDERER_H

#include "map/Tile.h"
#include "Unit.h"
#include "map/Grid.h"

enum class Key {
	UNKNOWN = -1,
	UP = 0, DOWN, LEFT, RIGHT,
	NEXT, PREV, ESC,
};
enum class Mouse {
	UNKNOWN = -1, LEFT = 0, RIGHT, MIDDLE,
};

class Renderer {
public:
	virtual void render() = 0;
	virtual void reset_grid(const Grid<Tile>& grid) = 0;

	virtual void resize(Pos2 size) { }
	virtual void mouse_move(Pos2 pos) { }
	virtual void mouse_press(Pos2 pos, Mouse mouse) { }
	virtual void mouse_release(Pos2 pos, Mouse mouse) { }
	virtual void mouse_scroll(float amount) { }
	virtual void key_press(Key key) { }
	virtual void key_release(Key key) { }
	virtual void fkey_press(int num) { }
	virtual void numkey_press(int num) { }

	virtual void pause() { }
	virtual void unpause() { }
};


#endif //SPENCE_RENDERER_H

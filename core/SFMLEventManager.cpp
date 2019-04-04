#include "SFMLEventManager.h"

SFMLEventManager::SFMLEventManager(SFMLRenderer& renderer): renderer(renderer) { }


Key sfml_to_key(sf::Event::KeyEvent key) {
	switch (key.code) {
		case sf::Keyboard::Escape: return Key::ESC;
		case sf::Keyboard::Up:     return Key::UP;
		case sf::Keyboard::Down:   return Key::DOWN;
		case sf::Keyboard::Left:   return Key::LEFT;
		case sf::Keyboard::Right:  return Key::RIGHT;
		case sf::Keyboard::Tab:    return key.shift ? Key::PREV : Key::NEXT;
		default: return Key::UNKNOWN;
	}
}
Mouse sfml_to_mouse(sf::Mouse::Button mouse) {
	switch (mouse) {
		case sf::Mouse::Right:  return Mouse::RIGHT;
		case sf::Mouse::Left:   return Mouse::LEFT;
		case sf::Mouse::Middle: return Mouse::MIDDLE;
		default: return Mouse::UNKNOWN;
	}
}
bool SFMLEventManager::update() {
	sf::Event event;
	while (renderer.get_window().pollEvent(event)) {
		switch (event.type) {
			case sf::Event::Closed:
				renderer.get_window().close();
				return true;
			case sf::Event::Resized:
				renderer.resize(Pos2(event.size.width, event.size.height));
				break;
			case sf::Event::LostFocus:
				renderer.pause();
				break;
			case sf::Event::GainedFocus:
				renderer.unpause();
				break;
			case sf::Event::KeyPressed:
				if (event.key.code >= sf::Keyboard::F1 && event.key.code <= sf::Keyboard::F15) {
					renderer.fkey_press(event.key.code - sf::Keyboard::F1 + 1);
				} else if (event.key.code >= sf::Keyboard::Num0 &&
				           event.key.code <= sf::Keyboard::Num9) {
					renderer.numkey_press(event.key.code - sf::Keyboard::Num0);
				} else if (event.key.code >= sf::Keyboard::Numpad0 &&
				           event.key.code <= sf::Keyboard::Numpad9) {
					renderer.numkey_press(event.key.code - sf::Keyboard::Numpad0);
				} else {
					Key key = sfml_to_key(event.key);
					if (key != Key::UNKNOWN) renderer.key_press(key);
				}
				break;
			case sf::Event::KeyReleased: {
				Key key = sfml_to_key(event.key);
				if (key != Key::UNKNOWN) renderer.key_release(key);
			} break;
			case sf::Event::MouseMoved:
				renderer.mouse_move(Pos2(event.mouseMove.x, event.mouseMove.y));
				break;
			case sf::Event::MouseWheelScrolled:
				renderer.mouse_scroll(event.mouseWheelScroll.delta);
				break;
			case sf::Event::MouseButtonPressed:
				renderer.mouse_press(Pos2(event.mouseButton.x, event.mouseButton.y),
				                     sfml_to_mouse(event.mouseButton.button));
				break;
			case sf::Event::MouseButtonReleased:
				renderer.mouse_release(Pos2(event.mouseButton.x, event.mouseButton.y),
				                       sfml_to_mouse(event.mouseButton.button));
				break;
			default: break;
		}
	}
	return false;
}

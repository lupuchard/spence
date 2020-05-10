#include <utility>

#ifndef SPENCE_UI_H
#define SPENCE_UI_H

#include <string>
#include <vector>
#include <functional>
#include "IEventHandler.h"

class UI {
public:
	void clear();
	void set_entry(size_t index, std::string text = "", Action action = Action());

	size_t num_entries() const;
	const std::string& get_text(size_t index) const;
	bool has_action(size_t index) const;
	Action get_action(size_t index);

	void set_has_changed();
	bool has_changed();

private:
	struct Entry {
		std::string text;
		Action action;
	};

	std::vector<Entry> entries;
	bool changed = false;
};


#endif //SPENCE_UI_H

#include "UI.h"
#include <cassert>

void UI::clear() {
	entries.clear();
	changed = true;
}

void UI::set_entry(size_t index, std::string text, Action action) {
	if (entries.size() <= index) {
		entries.resize(index + 1);
	}

	entries[index].text = std::move(text);
	entries[index].action = action;

	changed = true;
}

size_t UI::num_entries() const {
	return entries.size();
}

const std::string& UI::get_text(size_t index) const {
	assert(index < entries.size());
	return entries[index].text;
}

Action UI::get_action(size_t index) {
	assert(index < entries.size());
	return entries[index].action;
}

bool UI::has_action(size_t index) const {
	return index < entries.size() && entries[index].action.type != Action::Type::None;
}


bool UI::has_changed() {
	bool temp = changed;
	changed = false;
	return temp;
}

void UI::set_has_changed() {
	changed = true;
}

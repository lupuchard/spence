#include "UI.h"
#include <cassert>

void UI::clear() {
	entries.clear();
	changed = true;
}

void UI::set_entry(size_t index, std::string text, std::function<void()> callback) {
	if (entries.size() <= index) {
		entries.resize(index + 1);
	}

	entries[index].text = std::move(text);
	entries[index].callback = std::move(callback);

	changed = true;
}

size_t UI::num_entries() {
	return entries.size();
}

const std::string& UI::get_text(size_t index) {
	assert(index < entries.size());
	return entries[index].text;
}

const std::function<void()>& UI::get_callback(size_t index) {
	assert(index < entries.size());
	return entries[index].callback;
}


bool UI::has_changed() {
	bool temp = changed;
	changed = false;
	return temp;
}

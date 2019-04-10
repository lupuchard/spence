#include <utility>

#ifndef SPENCE_UI_H
#define SPENCE_UI_H

#include <string>
#include <vector>
#include <functional>

class UI {
public:
	void clear();
	void set_entry(size_t index, std::string text = "", std::function<void()> callback = nullptr);

	size_t num_entries();
	const std::string& get_text(size_t index);
	const std::function<void()>& get_callback(size_t index);

	bool has_changed();

private:
	struct Entry {
		std::string text;
		std::function<void()> callback;
	};

	std::vector<Entry> entries;
	bool changed = false;
};


#endif //SPENCE_UI_H

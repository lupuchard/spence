
enum Dir { North, South, East, West }
enum Wall { None, Climbable, InnerCover, OuterCover, Blocking }

function init() {
	reset_map([20, 20]);
	create_unit([10, 10, 0], "paul");

	local size = map_size();

	for (local y = 0; y < size[1]; y++) {
		for (local x = 0; x < size[0]; x++) {
			for (local dir = 0; dir < 4; ++dir) {
				local r = rand() % 15;
				if (r == 0) {
					set_wall([x, y, 0], dir, Wall.Blocking);
				} else if (r == 1) {
					set_wall([x, y, 0], dir, Wall.OuterCover);
					set_wall([x, y, 0], dir, Wall.InnerCover);
				}
			}
		}
	}
}

function on_hug(unit, action) {
	log("hugging");
}

function on_select(unit) {
	log(unit);
	set_move_radius(unit, 14);
	//add_action(unit, "hug", on_hug);
}

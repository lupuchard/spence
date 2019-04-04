#include "Squirrel.h"
#include "SQUIRREL3/include/sqstdmath.h"
#include "SQUIRREL3/include/sqstdstring.h"
#include <fstream>
#include <cstdarg>

Squirrel& self(HSQUIRRELVM v) {
	Squirrel* sq = nullptr;
	sq_getuserpointer(v, -1, (void**)&sq);
	return *sq;
}
SQInteger get_int(HSQUIRRELVM v, SQInteger idx) {
	SQInteger t;
	sq_getinteger(v, idx, &t);
	return t;
}
std::string get_str(HSQUIRRELVM v, SQInteger idx) {
	const char* str;
	sq_getstring(v, idx, &str);
	return std::string(str);
}
std::string get_func(HSQUIRRELVM v, SQInteger idx) {
	sq_getclosurename(v, idx);
	return get_str(v, -1);
}
Pos2 get_pos2(HSQUIRRELVM v, SQInteger idx) {
	SQInteger x, y;
	sq_pushinteger(v, 0);
	sq_get(v, idx);
	sq_getinteger(v, -1, &x);
	sq_pushinteger(v, 1);
	sq_get(v, idx);
	sq_getinteger(v, -1, &y);
	sq_pop(v, 2);
	return Pos2((int)x, (int)y);
}
void push_pos2(HSQUIRRELVM v, Pos2 pos) {
	sq_newarray(v, 0);
	sq_pushinteger(v, pos.x);
	sq_arrayappend(v, -2);
	sq_pushinteger(v, pos.y);
	sq_arrayappend(v, -2);
}
Pos3 get_pos3(HSQUIRRELVM v, SQInteger idx) {
	SQInteger x, y, z;
	sq_pushinteger(v, 0);
	sq_get(v, idx);
	sq_getinteger(v, -1, &x);
	sq_pushinteger(v, 1);
	sq_get(v, idx);
	sq_getinteger(v, -1, &y);
	sq_pushinteger(v, 2);
	sq_get(v, idx);
	sq_getinteger(v, -1, &z);
	sq_pop(v, 3);
	return Pos3((int)x, (int)y, (int)z);
}

std::string type_to_str(SQObjectType type) {
	switch (type) {
		case OT_NULL:        return "null";
		case OT_INTEGER:     return "integer";
		case OT_FLOAT:       return "float";
		case OT_BOOL:        return "bool";
		case OT_STRING:      return "string";
		case OT_ARRAY:       return "array";
		case OT_TABLE:       return "table";
		case OT_INSTANCE:    return "instance";
		case OT_USERPOINTER: return "pointer";
		default:             return "unknown";
	}
}

bool check_args(HSQUIRRELVM v, std::string name, ...) {
	va_list args;
	va_start(args, name);

	size_t num_args = (size_t)sq_gettop(v) - 2;
	int i = 0;
	while (true) {
		SQObjectType arg = (SQObjectType)va_arg(args, int);
		if (arg == 0) break;
		if (i >= num_args) {
			std::cout << name << ": too few args" << std::endl;
			va_end(args);return false;
		}
		SQObjectType type = sq_gettype(v, i + 2);
		if (type != arg) {
			std::cout << name << ": invalid arg " << i << " (expected " << type_to_str(arg) << ", got " << type_to_str(type) << ")" << std::endl;
			va_end(args);return false;
		}
		i++;
	}
	if (i < num_args) {
		std::cout << name << ": too many args (expected " << i << ")" << std::endl;
		va_end(args); return false;
	}

	va_end(args);
	return true;
}

std::string load(const std::string& filename) {
	std::ifstream t(filename);
	assert(t.is_open());
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

void on_compile_error(HSQUIRRELVM, const char* desc, const char* source,
                      SQInteger line, SQInteger column) {
	std::cout << "Failed to compile!" << std::endl;
	std::cout << "(" << source << ", " << line << ", " << column << "): " << desc;
}

const SQInteger NORETURN  = 0;
const SQInteger YESRETURN = 1;
const SQInteger ERROR     = SQ_ERROR;


Squirrel::Squirrel(Map& map): map(map) {
	v = sq_open(1024);
	sq_setcompilererrorhandler(v, on_compile_error);

	std::string file = load("../game/main.nut");
	assert(sq_compilebuffer(v, file.c_str(), (SQInteger)file.size(), "gameplay", SQTrue) >= 0);
	sq_pushroottable(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);
	assert(sq_call(v, 1, SQFalse, SQTrue) >= 0);

	sq_pushroottable(v);
	root_table = sq_gettop(v);
	sq_newtable(v);
	state_table = sq_gettop(v);

	init_func   = declare_func("init");
	select_func = declare_func("on_select");

	// log(str: string)
	register_func("log", [](HSQUIRRELVM v) {
		if (!check_args(v, "log", OT_STRING, 0)) return ERROR;
		std::cout << "Nutlog: " << get_str(v, 2) << std::endl;
		return NORETURN;
	});
	// reset_map(size: [int, int])
	register_func("reset_map", [](HSQUIRRELVM v) {
		if (!check_args(v, "reset_map", OT_ARRAY, 0)) return ERROR;
		self(v).map.reset(get_pos2(v, 2));
		return NORETURN;
	});
	// map_size
	register_func("map_size", [](HSQUIRRELVM v) {
		if (!check_args(v, "map_size", 0)) return ERROR;
		push_pos2(v, self(v).map.get_size());
		return YESRETURN;
	});
	// set_wall(pos: [int, int, int], dir: Dir, wall: Wall)
	register_func("set_wall", [](HSQUIRRELVM v) {
		if (!check_args(v, "set_wall", OT_ARRAY, OT_INTEGER, OT_INTEGER, 0)) return ERROR;
		self(v).map.set_wall(get_pos3(v, 2), (Dir)get_int(v, 3), (Wall)get_int(v, 4));
		return NORETURN;
	});
	// str create_unit(pos: [int, int, int] {, id: str})
	register_func("create_unit", [](HSQUIRRELVM v) {
		std::string name;
		if (sq_gettop(v) - 2 == 1) {
			if (!check_args(v, "create_unit", OT_ARRAY, 0)) return ERROR;
			name = self(v).map.create_unit(get_pos3(v, 2), "");
		} else {
			if (!check_args(v, "create_unit", OT_ARRAY, OT_STRING, 0)) return ERROR;
			name = self(v).map.create_unit(get_pos3(v, 2), get_str(v, 3));
		}
		sq_pushstring(v, name.c_str(), (SQInteger)name.size());
		return YESRETURN;
	});
	// add_action(unit: str, name: str, callback: func)
	register_func("add_action", [](HSQUIRRELVM v) {
		if (!check_args(v, "add_action", OT_STRING, OT_STRING, OT_CLOSURE, 0)) return ERROR;
		Unit* unit = self(v).map.get_unit(get_str(v, 2));
		if (unit == nullptr) return ERROR;
		unit->add_action(get_str(v, 3), get_func(v, 4));
		return NORETURN;
	});
	// set_move_radius(unit: str, radius: int)
	register_func("set_move_radius", [](HSQUIRRELVM v) {
		if (!check_args(v, "add_action", OT_STRING, OT_INTEGER, 0)) return ERROR;
		Unit* unit = self(v).map.get_unit(get_str(v, 2));
		if (unit == nullptr) return ERROR;
		unit->move_radius = (int)get_int(v, 3);
		return NORETURN;
	});
}

Squirrel::~Squirrel() {
	sq_close(v);
}

SQInteger Squirrel::declare_func(const std::string& name) {
	sq_pushstring(v, name.c_str(), (SQInteger)name.size());
	assert(sq_get(v, root_table) >= 0);
	return sq_gettop(v);
}
void Squirrel::prepare_call(SQInteger func) {
	assert(sq_gettype(v, func) == OT_CLOSURE);
	sq_push(v, func);
	sq_push(v, state_table);
}
void Squirrel::complete_call(SQInteger num_params) {
	SQRESULT res = sq_call(v, num_params, SQFalse, SQTrue);
	if (res < 0) {
		sq_getlasterror(v);
		const char* s;
		assert(sq_getstring(v, -1, &s) >= 0);
		std::cout << "Error " << res << ": " << s << std::endl;
		assert(false);
	}
	sq_pop(v, 1);
}

void Squirrel::init() {
	prepare_call(init_func);
	complete_call(1);
}
void Squirrel::on_select(const Unit& unit) {
	prepare_call(select_func);
	sq_pushstring(v, unit.name().c_str(), (SQInteger)unit.name().size());
	complete_call(2);
}

void Squirrel::register_func(const std::string& name, SQFUNCTION func) {
	sq_push(v, state_table);
	sq_pushstring(v, name.c_str(), (SQInteger)name.size());
	sq_pushuserpointer(v, this);
	sq_newclosure(v, func, 1);
	assert(sq_newslot(v, -3, SQTrue) >= 0);
	sq_pop(v, 1);
}

#include "luascript.hpp"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <fstream>
#include <iostream>
#include <utility>

#define CHECK(x)	check_error(x, #x, __FILE__, __LINE__)


namespace
{
	template <typename... Args>
	constexpr std::string concat(Args&&... args)
	{
		return (std::string() + ... + std::forward<Args>(args));
	}
}

namespace script
{

lua_script::lua_script()
	: lua_state{ luaL_newstate() }
{
	if (!lua_state)
		throw error(concat("Lua: cannot allocate new state"));

	luaL_openlibs(lua_state);

	std::cout << "script::lua_script(): -> " << this << std::endl;
}

lua_script::lua_script(const std::filesystem::path& path)
	: lua_state{ nullptr }
{
	std::ifstream is{ path };
	if (!is) throw error(concat("Cannot open file: ", path.string()));

	lua_state = luaL_newstate();
	if (!lua_state)
		throw error(concat("Lua: cannot allocate new state"));

	luaL_openlibs(lua_state);
	CHECK(luaL_loadfile(lua_state, path.string().c_str()))
		|| CHECK(lua_pcall(lua_state, 0, LUA_MULTRET, 0));
	clear_stack();

	std::cout << "script::lua_script(): " << path.string() << " -> " << this << std::endl;
}

lua_script::~lua_script()
{
	if (lua_state) lua_close(lua_state);
	std::cout << "script::~lua_script(): -> " << this << std::endl;
}

void lua_script::do_file(const std::filesystem::path& path)
{
	std::ifstream is{ path };
	if (!is) throw error(concat("Cannot open file: ", path.string()));
	CHECK(luaL_loadfile(lua_state, path.string().c_str()))
		|| CHECK(lua_pcall(lua_state, 0, LUA_MULTRET, 0));
	clear_stack();
}

void lua_script::do_string(const std::string& str)
{
	CHECK(luaL_loadstring(lua_state, str.c_str()))
		|| CHECK(lua_pcall(lua_state, 0, LUA_MULTRET, 0));
	clear_stack();
}

void lua_script::call(const char* func)
{
	lua_getglobal(lua_state, func);
	CHECK(lua_pcall(lua_state, 0, 0, 0));
	clear_stack();
}

void lua_script::call(const char* func, int* result)
{
	lua_getglobal(lua_state, func);
	CHECK(lua_pcall(lua_state, 0, 1, 0));

	if (result)
	{
		*result = lua_tointeger(lua_state, -1);
	}

	clear_stack();
}

void lua_script::matrix(const char* name, size_t rows, size_t cols, int8_t* buffer)
{
	if (!buffer) throw error("buffer is nullptr");

	lua_getglobal(lua_state, name);
	CHECK(lua_pcall(lua_state, 0, 1, 0));

	if (!lua_istable(lua_state, -1))
	{
		clear_stack();
		throw error(concat("Lua error: result is not a table '", name, "()'"));
	}

	for (size_t j = 0; j < rows; ++j)
	{
		if (LUA_TTABLE != lua_rawgeti(lua_state, -1, j + 1))
		{
			clear_stack();
			throw error(concat("Lua error: cannot find table ", name, "()[", std::to_string(j + 1), "]"));
		}

		for (size_t i = 0; i < cols; ++i)
		{
			if (LUA_TNUMBER != lua_rawgeti(lua_state, -1, i + 1))
			{
				clear_stack();
				throw error(concat("Lua error: expected number in ", name, "[", std::to_string(j + 1), "][", std::to_string(i + 1), "]"));
			}

			auto r = lua_tointeger(lua_state, -1);
			lua_pop(lua_state, 1);
			buffer[j * cols + i] = static_cast<int8_t>(r);
		}

		lua_pop(lua_state, 1);
	}

	clear_stack();
}

int lua_script::check_error(int rc, const char* statement, const char* file, int line)
{
	if (rc != LUA_OK)
	{
		auto msg{ lua_tostring(lua_state, -1) };
		clear_stack();
		throw error(concat("Lua error: "
			, msg
			, "\nstatement: ", statement
			, " at ", file
			, ":", std::to_string(line)));
	}
	return rc;
}

int lua_script::clear_stack()
{
	auto top{ lua_gettop(lua_state) };
	if (top) lua_pop(lua_state, top);
	return top;
}

} // namespace script

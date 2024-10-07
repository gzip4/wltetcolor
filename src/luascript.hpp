#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

extern "C" struct lua_State;

namespace script {

class error : public std::runtime_error
{
public:
	explicit error(const std::string& msg)
		: std::runtime_error(msg)
	{
	}
};

class lua_script final
{
public:
	lua_script();
	lua_script(const std::filesystem::path& path);
	~lua_script();

	lua_script(lua_script&&) = delete;
	lua_script(const lua_script&) = delete;
	lua_script& operator=(lua_script&&) = delete;
	lua_script& operator=(const lua_script&) = delete;

	void do_file(const std::filesystem::path& path);
	void do_string(const std::string& str);

	void call(const char* func);
	void call(const char* func, int* result);
	void matrix(const char* name, size_t rows, size_t cols, int8_t* buffer);

private:
	int check_error(int, const char*, const char*, int);
	int clear_stack();

private:
	lua_State* lua_state;
};

} // namespace script

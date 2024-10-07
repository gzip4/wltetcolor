cmake_minimum_required (VERSION 3.8)

add_executable (wltetcolor
	"main.cpp"
	"luascript.hpp"
	"luascript.cpp"
	"game.hpp"
	"game.cpp"
	"platform_w32.cpp"
	"platform.hpp")
target_link_libraries (wltetcolor PUBLIC Lua::Lua)

#include "game.hpp"
#include "luascript.hpp"

namespace tetcolor
{

game::game(unsigned int _width, unsigned int _height)
	: width{ _width }
	, height{ _height }
	, field_buffer(_width * _height)
	, script{ std::make_unique<script::lua_script>("./scripts/game2.lua") }
	, turn_no{}
	, level{}
{
}

void game::start()
{
	if (state == game_state::stop)
	{
		state = game_state::run;
		script->call("game_start");
		level = 0;
		update_field_buffer();
	}
}

void game::stop()
{
	auto old_state = state;
	state = game_state::stop;
	if (old_state != game_state::stop)
	{
		script->call("game_stop");
	}
}

void game::toggle_pause()
{
	if (state == game_state::pause) state = game_state::run;
	if (state == game_state::run) 	state = game_state::pause;
}

void game::turn()
{
	if (state != game_state::run) return;

	int rc{};
	script->call("game_turn", &rc);	// rc != 0 -> game over

	update_field_buffer();
	++turn_no;

	if (rc)
	{
		stop();
	}
}

void game::move_left()
{
	if (state != game_state::run) return;
	script->call("game_move_left");
	update_field_buffer();
}

void game::move_right()
{
	if (state != game_state::run) return;
	script->call("game_move_right");
	update_field_buffer();
}

void game::move_up()
{
	if (state != game_state::run) return;
	script->call("game_move_up");
	update_field_buffer();
}

void game::move_down()
{
	if (state != game_state::run) return;
	script->call("game_move_down");
	update_field_buffer();
}

void game::move_drop()
{
	if (state != game_state::run) return;
	script->call("game_move_drop");
	update_field_buffer();
}

void game::update_field_buffer()
{
	script->matrix("get_game_field", height, width, field_buffer.data());
}

} // namespace tetcolor

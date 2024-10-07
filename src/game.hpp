#pragma once

#include <memory>
#include <vector>

namespace script
{
class lua_script;
}

namespace tetcolor
{

enum class game_state
{
	run,
	pause,
	stop
};

constexpr unsigned int default_width = 7;
constexpr unsigned int default_height = 18;

class game final
{
public:
	game(unsigned int width = default_width, unsigned int height = default_height);
	~game() = default;

	game(game&&) = delete;
	game(const game&) = delete;
	game& operator=(game&&) = delete;
	game& operator=(const game&) = delete;

	void turn();

	// state change
	void start();
	void stop();
	void toggle_pause();

	// state check
	bool is_paused() const { return state == game_state::pause; }
	bool is_running() const { return state == game_state::run; }
	bool is_stopped() const { return state == game_state::stop; }
	unsigned int get_level() const { return level; }
	const int8_t* field() const { return field_buffer.data(); }

	// control
	void move_left();
	void move_right();
	void move_up();
	void move_down();
	void move_drop();

public:
	const unsigned int width;
	const unsigned int height;

private:
	void update_field_buffer();

private:
	game_state state{ game_state::stop };
	std::vector<int8_t> field_buffer;
	std::unique_ptr<script::lua_script> script;
	unsigned int turn_no;
	unsigned int level;
};

} // namespace tetcolor

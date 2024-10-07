print("Lua tetcolor v0.1")
math.randomseed(os.time())

local _width = 7
local _height = 18
local _level = 0

local _cell = {
	empty = -1,
	red = 0,
	green = 1,
	blue = 2,
	cyan = 3,
	yellow = 4,
	magenta = 5
}

local function create_field(width, height)
	local result = {}
	for j = 1, height do
		local row = {}
		for i = 1, width do
			--row[i] = math.random(-1, 5)
			row[i] = _cell.empty
		end
		result[j] = row
	end
	return result
end

_game_field = create_field(7, 18)
_figure = nil
_figure_pos = nil


local function gen_color()
	return math.random(0, 5)
end


local function generate_figure()
	local ftype = math.random(4)
	if ftype == 1 then
		return {
			{_cell.empty, gen_color(), _cell.empty},
			{_cell.empty, _cell.empty, _cell.empty},
			{_cell.empty, _cell.empty, _cell.empty}
		}
	end
	if ftype == 2 then
		return {
			{_cell.empty, gen_color(), _cell.empty},
			{_cell.empty, gen_color(), _cell.empty},
			{_cell.empty, _cell.empty, _cell.empty}
		}
	end
	if ftype == 3 then
		return {
			{_cell.empty, gen_color(), _cell.empty},
			{_cell.empty, gen_color(), _cell.empty},
			{_cell.empty, gen_color(), _cell.empty},
		}
	end
	if ftype == 4 then
		return {
			{_cell.empty, gen_color(), _cell.empty},
			{_cell.empty, gen_color(), gen_color()},
			{_cell.empty, _cell.empty, _cell.empty}
		}
	end
end


-- http://lua-users.org/wiki/CopyTable
local function deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end


local function copy_figure_to(dst)
	if not _figure then
		return;
	end

	local x, y = _figure_pos[1], _figure_pos[2]

	for j = 0, 2 do
		for i = 0, 2 do
			local value = _figure[j + 1][i + 1]
			if value ~= _cell.empty then
				if (y + j) >= 1 and (y + j) <= _height and (x + i) >= 1 and (x + i) <= _width then
					dst[y + j][x + i] = value
				end
			end
		end
	end
end


local function check_collision(x, y, figure)
	if figure == nil then
		figure = _figure
	end

	if not figure then
		return false
	end

	if y > _height then
		return true
	end

	for j = 0, 2 do
		for i = 0, 2 do
			local value = figure[j + 1][i + 1]
			if value == _cell.empty then
				goto skip_to_next
			end

			if y + j > _height then
				return true
			end

			local cell = _game_field[y + j][x + i]	-- can be nil
			if cell ~= _cell.empty then
				return true
			end

			::skip_to_next::
		end
	end

	return false
end



-- INTERFACE


function get_game_field()
	local game_field_copy = deepcopy(_game_field);
	copy_figure_to(game_field_copy)
	return game_field_copy;
end


function game_start()
	print("start")
	_level = 0
	_game_field = create_field(_width, _height)
end


function game_stop()
	print("stop level: " .. _level)
	copy_figure_to(_game_field)
	_figure = nil
	_figure_pos = nil
end


function game_turn()
	print("turn")

	if not _figure then
		_figure = generate_figure()
		_figure_pos = {3, 1}

		if check_collision(3, 1) then
			-- game over
			copy_figure_to(_game_field)
			return 1
		end

		return 0
	end

	local x, y = _figure_pos[1], _figure_pos[2]
	--local y = _figure_pos[2]

	if check_collision(x, y + 1) then
		copy_figure_to(_game_field)
		_figure = nil
		_figure_pos = nil
		return 0
	end

	_figure_pos[2] = _figure_pos[2] + 1

	return 0
end


function game_move_drop()
	if not _figure then
		return
	end

	local x, y = _figure_pos[1], _figure_pos[2]
	--local y = _figure_pos[2]

	for yy = y + 1, _height + 3 do
		if check_collision(x, yy) then
			_figure_pos[2] = yy - 1
			return
		end
	end
end


function game_move_left()
	if not _figure then
		return
	end

	local x, y = _figure_pos[1], _figure_pos[2]
	--local y = _figure_pos[2]

	if not check_collision(x - 1, y) then
		_figure_pos[1] = x - 1
	end
end


function game_move_right()
	if not _figure then
		return
	end

	local x, y = _figure_pos[1], _figure_pos[2]
	--local y = _figure_pos[2]

	if not check_collision(x + 1, y) then
		_figure_pos[1] = x + 1
	end
end


function game_move_up()	-- rotate counter clockwise
	if not _figure then
		return
	end

	if _figure[2][2] == _cell.empty then
		return
	end

	local f = _figure
	local figure = {
		{ f[1][3], f[2][3], f[3][3] },
		{ f[1][2], f[2][2], f[3][2] },
		{ f[1][1], f[2][1], f[3][1] }
	}

	local x, y = _figure_pos[1], _figure_pos[2]
	--local y = _figure_pos[2]

	if not check_collision(x, y, figure) then
		_figure = figure
	end
end


function game_move_down()	-- rotate clockwise
	if not _figure then
		return
	end

	if _figure[2][2] == _cell.empty then
		return
	end

	local f = _figure
	local figure = {
		{ f[3][1], f[2][1], f[1][1] },
		{ f[3][2], f[2][2], f[1][2] },
		{ f[3][3], f[2][3], f[1][3] }
	}

	local x, y = _figure_pos[1], _figure_pos[2]
	--local y = _figure_pos[2]

	if not check_collision(x, y, figure) then
		_figure = figure
	end
end

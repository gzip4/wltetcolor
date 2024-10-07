print("Lua tetcolor v0.1")
math.randomseed(os.time())

local _cell = {
	empty = -1,
	red = 0,
	green = 1,
	blue = 2,
	cyan = 3,
	yellow = 4,
	magenta = 5
}

local _game = nil
local _width = 7
local _height = 18

local function create_empty_field(width, height)
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

local function copy_figure_to(dst, figure, x, y)
	if not figure then
		return;
	end

	for j = 0, 2 do
		for i = 0, 2 do
			local value = figure[j + 1][i + 1]
			if value ~= _cell.empty then
				if (y + j) >= 1 and (y + j) <= _height and (x + i) >= 1 and (x + i) <= _width then
					dst[y + j][x + i] = value
				end
			end
		end
	end
end


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


local function check_collision(field, figure, x, y)
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

			local cell = field[y + j][x + i]	-- can be nil
			if cell ~= _cell.empty then
				return true
			end

			::skip_to_next::
		end
	end

	return false
end


-- GAME CLASS

Game = {
    -- field = nil,
    -- level = 0,
    -- fx = nil,
    -- fy = nil,
    -- figure = nil
}
    
function Game:new(o)
    o = o or {}
    setmetatable(o, self)
    self.__index = self

    self.field = create_empty_field(_width, _height)
    self.level = 0
    self.fx = nil
    self.fy = nil
    self.figure = nil

    return o
end


function Game:get_field()
	local field_copy = deepcopy(self.field);
	copy_figure_to(field_copy, self.figure, self.fx, self.fy)   -- noop if self.figure is nil
	return field_copy;
end


function Game:start()
end


function Game:land_figure()
    copy_figure_to(self.field, self.figure, self.fx, self.fy)
    self.figure = nil
    self.fx = nil
    self.fy = nil
end


function Game:stop()
    self:land_figure()
end


-- returns: 0 - good, 1 - game over
function Game:turn()
	if not self.figure then
		self.figure = generate_figure()
        self.fx = 3
        self.fy = 1

		if check_collision(self.field, self.figure, self.fx, self.fy) then
			-- game over
			-- copy_figure_to(self.field, self.figure, self.fx, self.fy)
            self:land_figure()
			return 1
		end

		return 0
	end

    -- if next turn collides - land the figure now
	if check_collision(self.field, self.figure, self.fx, self.fy + 1) then
        self:land_figure()
		return 0
	end

	self.fy = self.fy + 1

	return 0
end


function Game:move_drop()
	if not self.figure then
		return
	end

	for y = self.fy + 1, _height + 3 do
		if check_collision(self.field, self.figure, self.fx, y) then
			self.fy = y - 1
			return
		end
	end
end


function Game:move_left()
	if not self.figure then
		return
	end

	if not check_collision(self.field, self.figure, self.fx - 1, self.fy) then
		self.fx = self.fx - 1
	end
end


function Game:move_right()
	if not self.figure then
		return
	end

	if not check_collision(self.field, self.figure, self.fx + 1, self.fy) then
		self.fx = self.fx + 1
	end
end


function Game:move_up()
	if not self.figure then
		return
	end

	if self.figure[2][2] == _cell.empty then
		return
	end

	local f = self.figure
	local figure = {      -- rotated figure
        { f[3][1], f[2][1], f[1][1] },
        { f[3][2], f[2][2], f[1][2] },
        { f[3][3], f[2][3], f[1][3] }
    }

	if not check_collision(self.field, figure, self.fx, self.fy) then
		self.figure = figure
	end
end


function Game:move_down()
	if not self.figure then
		return
	end

	if self.figure[2][2] == _cell.empty then
		return
	end

	local f = self.figure
	local figure = {      -- rotated figure
		{ f[1][3], f[2][3], f[3][3] },
		{ f[1][2], f[2][2], f[3][2] },
		{ f[1][1], f[2][1], f[3][1] }
	}

	if not check_collision(self.field, figure, self.fx, self.fy) then
		self.figure = figure
	end
end


-- INTERFACE

function get_game_field()
    return _game:get_field();
	--return create_empty_field(7, 18)
end

function game_start()
	print("start")
    _game = Game:new()
end

function game_stop()
    _game:stop()
    -- _game = nil    -- ????????
end

function game_turn()
	print("turn")
	return _game:turn()
end

function game_move_drop()
    _game:move_drop()
end

function game_move_left()
    _game:move_left()
end

function game_move_right()
    _game:move_right()
end

function game_move_up()	-- rotate counter clockwise
    _game:move_up()
end

function game_move_down()	-- rotate clockwise
    _game:move_down()
end

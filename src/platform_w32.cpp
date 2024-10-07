#include "platform.hpp"
#include "luascript.hpp"
#include "game.hpp"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <array>
#include <cassert>
#include <iostream>
#include <functional>
#include <memory>



namespace {
using milliseconds = UINT;

constexpr char EMPTY_CELL = -1;
constexpr unsigned int CELL_SIZE_PX = 36;

struct
{
	bool quit;
	HWND hwnd;
	UINT_PTR nIDEvent;
	milliseconds elapsed;
	HDC hdc;
	PAINTSTRUCT ps;
	HBRUSH border_color;
	HBRUSH bg_color;
	std::array<HBRUSH, 6> colors;
	std::unique_ptr<tetcolor::game> game;
} platform_state{};

void on_paint();
void on_timer(HWND, UINT, UINT_PTR, DWORD);
void on_key_down(WPARAM, LPARAM);
void reset_timer(milliseconds elapsed = 1000);
void remove_timer();
void launch_game();
void need_repaint();
void draw_field();
void draw_paused();

template<typename... Args>
void print(Args&&... args)
{
	(std::cout << ... << std::forward<Args>(args)) << std::endl;
}
template<typename... Args>
void eprint(Args&&... args)
{
	(std::cerr << ... << std::forward<Args>(args)) << std::endl;
}


void no_exceptions(std::function<void()> f, const char* msg = nullptr) noexcept
{
	try
	{
		f();
	}
	catch (const std::exception& ex)
	{
		eprint(ex.what());
		platform_state.quit = true;
	}
	catch (...)
	{
		if (msg)
			eprint(msg);
		platform_state.quit = true;
	}
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_PAINT:
		print("paint");
		{
			platform_state.hdc = ::BeginPaint(hwnd, &platform_state.ps);
			no_exceptions(&on_paint, "ON PAINT ... EXCEPTION");
			::EndPaint(hwnd, &platform_state.ps);
		}
		return 0;

	case WM_KEYDOWN:
		no_exceptions([wParam, lParam]() {
			on_key_down(wParam, lParam);
			});
		return 0;
	} // switch (uMsg)

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void create_window()
{
	const TCHAR CLASS_NAME[]{ TEXT("Tetcolor") };
	WNDCLASS wc{};

	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);
	::RegisterClass(&wc);

	HWND hwnd = ::CreateWindowEx(
		0,                   // Optional window styles.
		CLASS_NAME,          // Window class
		TEXT("Tetcolor"),    // Window text
		WS_OVERLAPPEDWINDOW, // Window style

		// Position and size
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,

		NULL,      // Parent window
		NULL,      // Menu
		NULL,      // Instance handle
		NULL       // Additional application data
	);

	if (hwnd == NULL)
		throw std::runtime_error("Cannot create main window");

	platform_state.hwnd = hwnd;
	::ShowWindow(hwnd, SW_SHOW);

	platform_state.colors[0] = ::CreateSolidBrush(0x003030FF);	// RGB
	platform_state.colors[1] = ::CreateSolidBrush(0x0030FF30);
	platform_state.colors[2] = ::CreateSolidBrush(0x00FF3030);
	platform_state.colors[3] = ::CreateSolidBrush(0x00FFFF00);	// cyan, yellow, magenta
	platform_state.colors[4] = ::CreateSolidBrush(0x0000FFFF);
	platform_state.colors[5] = ::CreateSolidBrush(0x00FF00FF);
	platform_state.border_color = ::CreateSolidBrush(0x00);
	platform_state.bg_color = ::CreateSolidBrush(0x00808080);
}


void reset_timer(milliseconds elapsed)
{
	remove_timer();

	UINT_PTR nIDEvent = ::SetTimer(NULL, 0, elapsed, on_timer);
	if (nIDEvent == 0)
		throw std::runtime_error("Cannot create timer");
	platform_state.nIDEvent = nIDEvent;
	platform_state.elapsed = elapsed;
	print("reset_timer: ", nIDEvent, ", ", elapsed);
}


void remove_timer()
{
	if (platform_state.nIDEvent != 0)
	{
		::KillTimer(NULL, platform_state.nIDEvent);
		platform_state.nIDEvent = 0;
	}
}


void need_repaint()
{
	::InvalidateRect(platform_state.hwnd, NULL, FALSE);
}


void draw_paused()
{
	print("draw paused");
	const LONG width = platform_state.game->width;
	const LONG height = platform_state.game->height;
	const LONG cw = CELL_SIZE_PX;

	RECT client{};
	::GetClientRect(platform_state.hwnd, &client);
	const LONG horizont_offset{ client.right / 2 - width * cw / 2 };

	RECT border_rect{
		horizont_offset,
		0,
		width * cw + horizont_offset + 1,
		height * cw + 1 };
	::FrameRect(platform_state.hdc, &border_rect, platform_state.border_color);
}


void draw_field()
{
	print("draw field");
	const auto* f = platform_state.game->field();
	const LONG width = platform_state.game->width;
	const LONG height = platform_state.game->height;
	const LONG cw = CELL_SIZE_PX;

	RECT client{};
	::GetClientRect(platform_state.hwnd, &client);
	const LONG horizont_offset = client.right / 2 - width * cw / 2;

	RECT border_rect{
		horizont_offset,
		0,
		width * cw + horizont_offset + 1,
		height * cw + 1
	};
	::FrameRect(platform_state.hdc, &border_rect, platform_state.border_color);

	for (LONG row = 0; row < height; ++row)
	{
		for (LONG col = 0; col < width; ++col)
		{
			auto cell{ f[row * width + col] };
			if (cell == EMPTY_CELL) continue;
			assert(cell >= 0 && cell <= 5);

			LONG left = col * cw + horizont_offset;
			LONG top = row * cw;
			LONG right = col * cw + cw + horizont_offset;
			LONG bottom = row * cw + cw;
			RECT r{ left, top, right, bottom };

			auto color{ platform_state.colors[cell] };
			::FillRect(platform_state.hdc, &r, color);

			++r.right;
			++r.bottom;
			::FrameRect(platform_state.hdc, &r, platform_state.border_color);
		}
	}
}


void event_loop()
{
	MSG msg{};
	while (!platform_state.quit && ::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}


void launch_game()
{
	if (!platform_state.game)
		platform_state.game = std::make_unique<tetcolor::game>();
	print("launch game");
	platform_state.game->start();
	platform_state.game->turn();
	reset_timer();
	need_repaint();
}


void on_paint()
{
	::FillRect(platform_state.hdc, &platform_state.ps.rcPaint, platform_state.bg_color);

	if (!platform_state.game)
		return;

	if (!platform_state.game->is_paused())
		draw_field();
}


CALLBACK void on_timer(HWND, UINT, UINT_PTR, DWORD)
{
	try
	{
		print("timer");
		if (platform_state.game)
		{
			platform_state.game->turn();
			need_repaint();

			if (platform_state.game->is_stopped())
			{
				remove_timer();
			}
		}
	}
	catch (const std::exception& ex)
	{
		eprint(ex.what());
		platform_state.quit = true;
	}
}

void on_key_down(WPARAM key, LPARAM)
{
	print("on_key_down: ", key);

	if (platform_state.game)
	{
		switch (key)
		{
		case VK_ESCAPE:
			platform_state.quit = true;
			break;

		case VK_RETURN:
		{
			if (platform_state.game->is_running())
			{
				remove_timer();
				platform_state.game->stop();
				print("stop game");
			}
			else if (platform_state.game->is_stopped())
			{
				platform_state.game.reset();
				launch_game();
			}
		}
		break;

		case VK_NUMPAD4:
		case VK_LEFT:
			if (platform_state.game->is_running())
			{
				platform_state.game->move_left();
				need_repaint();
			}
			break;

		case VK_NUMPAD6:
		case VK_RIGHT:
			if (platform_state.game->is_running())
			{
				platform_state.game->move_right();
				need_repaint();
			}
			break;

		case VK_NUMPAD5:
		case VK_NUMPAD8:
		case VK_UP:
			if (platform_state.game->is_running())
			{
				platform_state.game->move_up();
				need_repaint();
			}
			break;

		case VK_NUMPAD2:
		case VK_DOWN:
			if (platform_state.game->is_running())
			{
				platform_state.game->move_down();
				need_repaint();
			}
			break;

		case VK_SPACE:
			if (platform_state.game->is_running())
			{
				platform_state.game->move_drop();
				need_repaint();
				reset_timer(platform_state.elapsed);
			}
			break;

		case 80: // P
			if (platform_state.game->is_running())
			{
				platform_state.game->toggle_pause();
			}
			break;
		} // switch (key)
	}
	else  // if (!platform_state.game)
	{
		if (key == VK_RETURN) launch_game();
	}

} // on_key_down

} // namespace


int platform::run(int /* argc */, char** /* argv */)
{
	try
	{
		print(std::filesystem::current_path().string());
		create_window();
		event_loop();
	}
	catch (const std::exception& ex)
	{
		eprint(ex.what());
		return 1;
	}
	catch (...)
	{
		eprint("... exception!");
		return 2;
	}

	return 0;
}

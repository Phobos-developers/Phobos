#include <Utilities/Macro.h>

#include <Unsorted.h>
#include <Drawing.h>
#include <SessionClass.h>

#ifdef CALL
#undef CALL
#endif

static BOOL WINAPI ClientToScreenHook(HWND hWnd, LPPOINT lpPoint)
{
	return TRUE;
}
DEFINE_PATCH_TYPED(void*, 0x7E14B8, ClientToScreenHook);

static void __fastcall CenterWindowIn(HWND window, HWND parent)
{
	RECT parentRect;
	::GetClientRect(parent, &parentRect);

	if (parent == Game::hWnd)
	{
		parentRect.right = Drawing::RenderWidth;
		parentRect.bottom = Drawing::RenderHeight;
	}

	::ClientToScreen(parent, reinterpret_cast<LPPOINT>(&parentRect));
	::ClientToScreen(parent, reinterpret_cast<LPPOINT>(&parentRect.right));
	parentRect.right -= parentRect.left;
	parentRect.bottom -= parentRect.top;

	RECT rect;
	::GetClientRect(window, &rect);
	::ClientToScreen(window, reinterpret_cast<LPPOINT>(&rect));
	::ClientToScreen(window, reinterpret_cast<LPPOINT>(&rect.right));
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	int x = (parentRect.right - rect.right + 1) / 2;
	int y = (parentRect.bottom - rect.bottom + 1) / 2;

	x = std::max(x, 0);
	y = std::max(y, 0);

	::SetWindowPos(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x777080, CenterWindowIn);

static BOOL __fastcall MoveDialog(HWND window, int x, int y)
{
	int xPos;
	int yPos;

	RECT screenRect;
	screenRect.left = 0;
	screenRect.top = 0;
	screenRect.right = Drawing::RenderWidth;
	screenRect.bottom = Drawing::RenderHeight;

	::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&screenRect));
	::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&screenRect.right));

	RECT windowRect;
	::GetWindowRect(window, &windowRect);

	windowRect.right -= windowRect.left;
	windowRect.bottom -= windowRect.top;

	if (x == -1)
		xPos = windowRect.left - screenRect.left;
	else
		xPos = x;
	windowRect.left = xPos;

	if (y == -1)
		yPos = windowRect.top - screenRect.top;
	else
		yPos = y;
	windowRect.top = yPos;

	return ::MoveWindow(window, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom, FALSE);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x623170, MoveDialog);

static BOOL __fastcall WinDialogGetRectangle(HWND hWnd, LPRECT rect)
{
	BOOL result = ::GetWindowRect(hWnd, rect);
	if (result)
	{
		RECT client;
		::GetClientRect(Game::hWnd, &client);
		::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&client));
		rect->left -= client.left;
		rect->right -= client.left;
		rect->top -= client.top;
		rect->bottom -= client.top;
	}
	return result;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x775690, WinDialogGetRectangle);

static BOOL __fastcall GetWindowRectHook(HWND hWnd, LPRECT rect)
{
	return ::GetWindowRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL, 0x610E77, GetWindowRectHook);

static BOOL __fastcall MoveIngameWindowControls(HWND hWnd)
{
	if (!SessionClass::Instance.CurrentlyInGame)
		return FALSE;

	auto parent = ::GetParent(hWnd);

	RECT rect;
	RECT parentRect;
	if (!parent || !::GetWindowRect(hWnd, &rect) || !::GetWindowRect(parent, &parentRect))
		return FALSE;

	int x = rect.left - parentRect.left + (parentRect.right - parentRect.left - 800) / 2;
	int y = rect.top - parentRect.top + (parentRect.bottom - parentRect.top - 600) / 2;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	return ::MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, FALSE);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x60B7A0, MoveIngameWindowControls);

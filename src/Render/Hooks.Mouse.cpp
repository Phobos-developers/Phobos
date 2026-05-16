#include "Mouse.h"

#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

#include <Unsorted.h>
#include <Surface.h>
#include <CCToolTip.h>

#include <semaphore>

DEFINE_HOOK(0x6BDEF9, WinMain_CreateWWMouse, 0x5) {
	DXMouse::Instance = GameCreate<DXMouse>(DSurface::Primary, Game::hWnd);
	R->EAX(DXMouse::Instance);
	return 0x6BDF25;
}

static HANDLE MouseThread;
static std::binary_semaphore MouseThreadSemaphore { 0 };

static DWORD WINAPI _MouseThread(LPVOID) {
	while (!MouseThreadSemaphore.try_acquire_for(std::chrono::milliseconds(10))) {
		if (DXMouse::Instance)
			DXMouse::Instance->Process_Mouse();
	}
	return 0;
}

static void __fastcall _DXMouse_StartMouseThread() {
	MouseThread = ::CreateThread(nullptr, 0, _MouseThread, nullptr, 0, nullptr);
	if (!MouseThread) {
		MouseThreadSemaphore.release();
		return;
	}
	::SetThreadPriority(MouseThread, THREAD_PRIORITY_TIME_CRITICAL);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x7B84F0, _DXMouse_StartMouseThread);

static void __fastcall _DXMouse_EndMouseThread() {
	MouseThreadSemaphore.release();
	if (MouseThread) {
		::WaitForSingleObject(MouseThread, INFINITE);
		::CloseHandle(MouseThread);
		MouseThread = nullptr;
	}
}
DEFINE_FUNCTION_JUMP(LJMP, 0x7B86B0, _DXMouse_EndMouseThread);

static void __fastcall _DXMouse_ProcessMouse(DXMouse* This) {
	This->Process_Mouse();
}
DEFINE_FUNCTION_JUMP(LJMP, 0x7BA090, _DXMouse_ProcessMouse);

DEFINE_HOOK_AGAIN(0x72429E, DXMouse_TooltipManager_GetMousePosition, 0xA);
DEFINE_HOOK(0x724359, DXMouse_TooltipManager_GetMousePosition, 0xA) {
	GET(ToolTipManager*, pThis, ESI);
	pThis->CurrentMousePosition = DXMouse::Instance->Get_Mouse_Point();
	R->EBX(&pThis->CurrentMousePosition);
	return R->Origin() + 0x15;
}

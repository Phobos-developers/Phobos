#include "Mouse.h"

#include <Utilities/Macro.h>

#include <Unsorted.h>
#include <Surface.h>
#include <CCToolTip.h>

#include <semaphore>

DEFINE_HOOK(0x6BDEF9, WinMainCreateWWMouse, 0x5) {
	DXMouse::Instance = GameCreate<DXMouse>(DSurface::Primary, Game::hWnd);
	R->EAX(DXMouse::Instance);
	return 0x6BDF25;
}

static HANDLE MouseThread;
static std::binary_semaphore MouseThreadSemaphore { 0 };

static DWORD WINAPI MouseThreadProc(LPVOID) {
	while (!MouseThreadSemaphore.try_acquire_for(std::chrono::milliseconds(10))) {
		if (DXMouse::Instance)
			DXMouse::Instance->ProcessMouse();
	}
	return 0;
}

static void __fastcall DXMouseStartMouseThread() {
	MouseThread = ::CreateThread(nullptr, 0, MouseThreadProc, nullptr, 0, nullptr);
	if (!MouseThread) {
		MouseThreadSemaphore.release();
		return;
	}
	::SetThreadPriority(MouseThread, THREAD_PRIORITY_TIME_CRITICAL);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x7B84F0, DXMouseStartMouseThread);

static void __fastcall DXMouseEndMouseThread() {
	MouseThreadSemaphore.release();
	if (MouseThread) {
		::WaitForSingleObject(MouseThread, INFINITE);
		::CloseHandle(MouseThread);
		MouseThread = nullptr;
	}
}
DEFINE_FUNCTION_JUMP(LJMP, 0x7B86B0, DXMouseEndMouseThread);

static void __fastcall DXMouseProcessMouse(DXMouse* pThis) {
	pThis->ProcessMouse();
}
DEFINE_FUNCTION_JUMP(LJMP, 0x7BA090, DXMouseProcessMouse);

DEFINE_HOOK_AGAIN(0x72429E, DXMouseTooltipManagerGetMousePosition, 0xA);
DEFINE_HOOK(0x724359, DXMouseTooltipManagerGetMousePosition, 0xA) {
	GET(ToolTipManager*, pThis, ESI);
	pThis->CurrentMousePosition = DXMouse::Instance->GetMousePoint();
	R->EBX(&pThis->CurrentMousePosition);
	return R->Origin() + 0x15;
}

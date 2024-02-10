#include <Helpers/Macro.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Phobos.h>

#include <GScreenClass.h>
#include <WWMouseClass.h>
#include <TacticalClass.h>
#include <GadgetClass.h>
#include <MessageListClass.h>
#include <CCToolTip.h>
#include <MouseClass.h>
#include <SessionClass.h>

#include <mutex>
#include <thread>

namespace Multithreading
{
	static constexpr reference<bool, 0xB0B519u> const BlitMouse {};
	static constexpr reference<bool, 0xA9FAB0u> const IonStormClass_ChronoScreenEffect_Status {};
	static constexpr reference<GadgetClass*, 0xA8EF54u> const Buttons {};
	static constexpr reference<bool, 0xA8ED6Bu> const DebugDebugDebug {};
	static constexpr reference<bool, 0xA8B8B4u> const EnableMultiplayerDebug {};

	void MultiplayerDebugPrint()
		{ JMP_STD(0x55F1E0); }

	bool MainLoop()
		{ JMP_STD(0x55D360); }

	// Wait this long in LockOrDemandMutex before getting impatient. Bigger values = less frequent lock demands.
	static const std::chrono::duration MainPatienceDuration = std::chrono::milliseconds(5);
	// Wait this long in LockOrDemandMutex before getting impatient. Bigger values = less frequent lock demands.
	static const std::chrono::duration DrawingPatienceDuration = std::chrono::milliseconds(5);
	// Check this often if the demanded mutex lock has been released by the other thread.
	static const std::chrono::duration ChillingDuration = std::chrono::milliseconds(1);

	std::unique_ptr<std::thread> DrawingThread = nullptr;
	std::timed_mutex DrawingMutex;
	std::mutex PauseMutex;
	bool MainThreadDemandsDrawingMutex = false;
	bool DrawingThreadDemandsDrawingMutex = false;
	bool MainThreadDemandsPauseMutex = false;
	bool IsInMultithreadMode = false;

	// Our new drawing thread loop.
	void DrawingLoop();
	// Faithful reproduction of GScreenClass::Render().
	void Render(GScreenClass* pThis);
	// Spawn the drawing thread.
	void EnterMultithreadMode();
	// Kill the drawing thread.
	void ExitMultithreadMode();
	// Wait for mutex lock respectfuly or, if too much time has passed, demand it.
	void LockOrDemandMutex(std::timed_mutex& mutex, bool& demands, std::chrono::duration<long long, std::milli> patienceDuration);

	DEFINE_DYNAMIC_PATCH(Disable_MainGame_MainLoop, 0x48CE8A,
		0xE8, 0xD1, 0x04, 0x0D, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_GScreenClass_Render_Disable, 0x4F4480,
		0x83, 0xEC, 0x08, 0xA1, 0x1C, 0x73, 0x88, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StartLock, 0x55D878,
		0x8B, 0x0D, 0xF8, 0xD5, 0xA8, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StartLock_2, 0x55DBC3,
		0xB9, 0x90, 0x03, 0x8A, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StopLock, 0x55DDA0,
		0xB9, 0x60, 0xBC, 0xA8, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StopLock_2, 0x55D903,
		0xC6, 0x05, 0x9D, 0xED, 0xA8, 0x00, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_PauseGame_SetPause, 0x683EB6,
		0x8B, 0x0D, 0x58, 0xE7, 0x87, 0x00);
	DEFINE_DYNAMIC_PATCH(Disable_PauseGame_ResetPause, 0x683FB2,
		0xB9, 0xE8, 0xF7, 0x87, 0x00);
}

void Multithreading::EnterMultithreadMode()
{
	if (IsInMultithreadMode)
		return;
	Debug::Log("Entering the multithread mode - just before MainLoop gameplay loop.\n");
	IsInMultithreadMode = true;
	DrawingThread = std::make_unique<std::thread>(std::thread(DrawingLoop));
}

void Multithreading::ExitMultithreadMode()
{
	if (!IsInMultithreadMode)
		return;
	Debug::Log("Exiting the multithread mode - MainLoop reported end of gameplay.\n");
	IsInMultithreadMode = false;
	DrawingThread.get()->detach();
	DrawingThread.release();
}

void Multithreading::LockOrDemandMutex(std::timed_mutex& mutex, bool& demands, std::chrono::duration<long long, std::milli> patienceDuration)
{
	if (mutex.try_lock_for(patienceDuration))
		return;
	demands = true;
	mutex.lock();
}

void Multithreading::Render(GScreenClass* pThis)
{
	auto pTempSurface = DSurface::Temp.get();
	DSurface::Temp = DSurface::Composite;
	WWMouseClass::Instance->func_40(DSurface::Composite, false);

	bool shouldDraw = pThis->Bitfield != 0;
	bool complete = pThis->Bitfield == 2;
	pThis->Bitfield = 0;

	if (!IonStormClass_ChronoScreenEffect_Status)
	{
		TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 0);
		TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 1);
		pThis->Draw(complete);
		TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 2);
	}

	if (BlitMouse.get() && !DebugDebugDebug.get())
	{
		WWMouseClass::Instance->func_40(DSurface::Sidebar, true);
		BlitMouse = false;
	}

	if (Buttons.get())
		Buttons->DrawAll(false);

	MessageListClass::Instance->Draw();

	if (CCToolTip::Instance.get())
		CCToolTip::Instance->Draw(false);

	if (EnableMultiplayerDebug.get())
		MultiplayerDebugPrint();

	WWMouseClass::Instance->func_3C(DSurface::Composite, false);
	pThis->vt_entry_44();

	DSurface::Temp = pTempSurface;
}

void Multithreading::DrawingLoop()
{
	while (IsInMultithreadMode)
	{
		// Main thread wants to pause the game. Let it lock the mutex by not doing anything yourself.
		while (MainThreadDemandsPauseMutex)
			std::this_thread::sleep_for(ChillingDuration);

		// If the game is paused, wait until it's unpaused (aka until the mutex is free to be locked).
		PauseMutex.lock();

		// We must avoid starving out the main thread...
		while (MainThreadDemandsDrawingMutex)
			std::this_thread::sleep_for(ChillingDuration);

		// ...but we also don't want to run 1200 TPS with 10 FPS.
		// Let's try waiting for the lock until we get impatient and *demand* priority.
		LockOrDemandMutex(DrawingMutex, DrawingThreadDemandsDrawingMutex, DrawingPatienceDuration);

		// Do the thing.
		Render(MouseClass::Instance);

		// We're done. Unlock all mutexes.
		DrawingThreadDemandsDrawingMutex = false;
		DrawingMutex.unlock();
		PauseMutex.unlock();
	}
	Debug::Log("Exiting the drawing thread.\n");
}

// Disable the hooks if we're in multiplayer modes or if multithreading was disabled in rules.
DEFINE_HOOK(0x48CE7E, MainGame_BeforeMainLoop, 7)
{
	if (Phobos::Config::MultiThreadSinglePlayer && SessionClass::Instance->IsSingleplayer())
		return 0;

	Multithreading::Disable_MainGame_MainLoop->Apply();
	Multithreading::Disable_GScreenClass_Render_Disable->Apply();
	Multithreading::Disable_MainLoop_StartLock->Apply();
	Multithreading::Disable_MainLoop_StartLock_2->Apply();
	Multithreading::Disable_MainLoop_StopLock->Apply();
	Multithreading::Disable_MainLoop_StopLock_2->Apply();
	Multithreading::Disable_PauseGame_SetPause->Apply();
	Multithreading::Disable_PauseGame_ResetPause->Apply();

	return 0;
}

// Run the MainLoop (sadly not enough space to hook after it),
// then decide if we should exit multithread mode.
DEFINE_HOOK(0x48CE8A, MainGame_MainLoop, 0)
{
	Multithreading::EnterMultithreadMode();

	bool gameEnded = Multithreading::MainLoop();
	R->EAX(gameEnded);
	if (gameEnded)
		Multithreading::ExitMultithreadMode();

	return 0x48CE8F;
}

// Completely skip vanilla GScreenClass::Render code in the main thread
// if we run in multithread mode.
DEFINE_HOOK(0x4F4480, GScreenClass_Render_Disable, 8)
{
	return Multithreading::IsInMultithreadMode ? 0x4F45A8 : 0;
}

// We want to lock access to game resources when we're doing game logic potientially related to graphics.
// The main thread should let the drawing thread run if it complains that it's too hungry and vice versa.
DEFINE_HOOK_AGAIN(0x55DBC3, MainLoop_StartLock_2, 5)
DEFINE_HOOK(0x55D878, MainLoop_StartLock, 6)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	while (Multithreading::DrawingThreadDemandsDrawingMutex)
		std::this_thread::sleep_for(Multithreading::ChillingDuration);

	Multithreading::LockOrDemandMutex(
		Multithreading::DrawingMutex,
		Multithreading::MainThreadDemandsDrawingMutex,
		Multithreading::MainPatienceDuration);

	Multithreading::MainThreadDemandsDrawingMutex = false;

	return 0;
}

// See above.
DEFINE_HOOK_AGAIN(0x55D903, MainLoop_StopLock_2, 7)
DEFINE_HOOK(0x55DDA0, MainLoop_StopLock, 5)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	Multithreading::DrawingMutex.unlock(); // TODO: shut up the warning
	return 0;
}

// We don't want to draw the tactical view when the player has paused the game.
// Technically it can be done with a busy wait loop, but mutexes are better for performance.
// The main thread should always have priority for the mutex access.
DEFINE_HOOK(0x683EB6, PauseGame_SetPause, 6)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	Multithreading::MainThreadDemandsPauseMutex = true;
	Multithreading::PauseMutex.lock();
	Multithreading::MainThreadDemandsPauseMutex = false;
	return 0;
}

// Resume operation after pausing the game.
DEFINE_HOOK(0x683FB2, ResumeGame_ResetPause, 5)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	Multithreading::PauseMutex.unlock();
	return 0;
}

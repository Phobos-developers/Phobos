#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <GScreenClass.h>
#include <WWMouseClass.h>
#include <TacticalClass.h>
#include <GadgetClass.h>
#include <MessageListClass.h>
#include <CCToolTip.h>
#include <MouseClass.h>

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

	// Wait this long in LockOrDemandMutex before getting impatient.
	static const std::chrono::duration PatienceDuration = std::chrono::milliseconds(5);
	// Check this often if the demanded mutex lock has been released by the other thread.
	static const std::chrono::duration ChillingDuration = std::chrono::milliseconds(1);

	std::thread DrawingThread;
	std::timed_mutex DrawingMutex;
	std::mutex PauseMutex;
	bool MainDemandsDrawingMutex = false;
	bool DrawingDemandsDrawingMutex = false;
	bool MainDemandsPauseMutex = false;
	bool InGameLoop = false;

	// Our new drawing thread loop.
	void DrawingLoop();
	// Faithful reproduction of GScreenClass::Render().
	void Render(GScreenClass* pThis);
	// Spawn the drawing thread.
	void EnterMultithreadMode();
	// Kill the drawing thread.
	void ExitMultithreadMode();
	// Wait for mutex lock respectfuly or, if too much time has passed, demand it.
	void LockOrDemandMutex(std::timed_mutex& mutex, bool& demands);
}

void Multithreading::EnterMultithreadMode()
{
	Debug::Log("Entering the multithread mode - just before MainLoop gameplay loop.");
	InGameLoop = true;
	DrawingThread = std::thread(DrawingLoop);
}

void Multithreading::ExitMultithreadMode()
{
	Debug::Log("Exiting the multithread mode - MainLoop reported end of gameplay.");
	InGameLoop = false;
	DrawingThread.detach();
	DrawingThread.~thread();
}

void Multithreading::LockOrDemandMutex(std::timed_mutex& mutex, bool& demands)
{
	if (mutex.try_lock_for(PatienceDuration))
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
	while (InGameLoop)
	{
		// Main thread wants to pause the game. Let it lock the mutex by not doing anything yourself.
		while (MainDemandsPauseMutex)
			std::this_thread::sleep_for(ChillingDuration);

		// If the game is paused, wait until it's unpaused (aka until the mutex is free to be locked).
		PauseMutex.lock();

		// We must avoid starving out the main thread...
		while (MainDemandsDrawingMutex)
			std::this_thread::sleep_for(ChillingDuration);

		// ...but we also don't want to run 1200 TPS with 10 FPS.
		// Let's try waiting for the lock until we get impatient and *demand* priority.
		LockOrDemandMutex(DrawingMutex, DrawingDemandsDrawingMutex);

		// Do the thing.
		Render(MouseClass::Instance);

		// We're done. Unlock all mutexes.
		DrawingDemandsDrawingMutex = false;
		DrawingMutex.unlock();
		PauseMutex.unlock();
	}
	Debug::Log("Exiting the drawing thread.");
}

// Before the MainLoop/AuxLoop begins in MainGame.
DEFINE_HOOK(0x48CE7E, MainGame_BeforeGameLoop, 7)
{
	Multithreading::EnterMultithreadMode();
	return 0;
}

// Run the MainLoop (sadly not enough space to hook after it),
// then decide if we should exit multithread mode.
DEFINE_HOOK(0x48CE8A, MainGame_MainLoop, 0)
{
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
	return Multithreading::InGameLoop ? 0x4F45A8 : 0;
}

// We want to lock access to game resources when we're doing game logic potientially related to graphics.
// The main thread should let the drawing thread run if it complains that it's too hungry and vice versa.
// TODO: Try to hook later for shorter lock period.
DEFINE_HOOK(0x55D878, MainLoop_StartLock, 6)
{
	while (Multithreading::DrawingDemandsDrawingMutex)
		std::this_thread::sleep_for(Multithreading::ChillingDuration);
	Multithreading::LockOrDemandMutex(Multithreading::DrawingMutex, Multithreading::MainDemandsDrawingMutex);
	Multithreading::MainDemandsDrawingMutex = false;

	return 0;
}

// See above.
// TODO: Try to hook sooner for shorter lock period.
DEFINE_HOOK(0x55DDA0, MainLoop_StopLock, 5)
{
	Multithreading::DrawingMutex.unlock(); // TODO: shut up the warning
	return 0;
}

// We don't want to draw the tactical view when the player has paused the game.
// Technically it can be done with a busy wait loop, but mutexes are better for performance.
// The main thread should always have priority for the mutex access.
DEFINE_HOOK(0x683EB6, PauseGame_SetPause, 6)
{
	Multithreading::MainDemandsPauseMutex = true;
	Multithreading::PauseMutex.lock();
	Multithreading::MainDemandsPauseMutex = false;
	return 0;
}

// Resume operation after pausing the game.
DEFINE_HOOK(0x683FB2, ResumeGame_ResetPause, 5)
{
	Multithreading::PauseMutex.unlock();
	return 0;
}

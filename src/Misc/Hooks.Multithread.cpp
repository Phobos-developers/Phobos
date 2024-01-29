#include <Helpers/Macro.h>

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

	std::thread DrawingThread;

	std::timed_mutex DrawingMutex;
	std::mutex PauseMutex;
	std::unique_lock DrawingLock { DrawingMutex, std::defer_lock };
	std::unique_lock PauseLock { PauseMutex, std::defer_lock };
	bool DrawingDemandsDrawingMutex = false;
	bool MainDemandsPauseMutex = false;

	bool InGameLoop = false;
	bool IsPaused = false;

	void MultiplayerDebugPrint()
		{ JMP_STD(0x55F1E0); }

	// Faithful reproduction of GScreenClass::Render().
	void Render(GScreenClass* pThis)
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

	// Our new drawing thread loop.
	void DrawingLoop()
	{
		while (InGameLoop)
		{
			// Main thread wants to pause the game. Let it lock the mutex by not doing anything yourself.
			while (MainDemandsPauseMutex)
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

			// If the game is paused, wait until it's unpaused (aka until the mutex is free to be locked).
			PauseMutex.lock();

			// We must avoid starving out the main thread, but we also don't want to run 1200 TPS with 10 FPS.
			// Let's try waiting for the lock until we get impatient and *demand* priority.
			bool gotLock = DrawingMutex.try_lock_for(std::chrono::milliseconds(5));
			if (!gotLock)
			{
				DrawingDemandsDrawingMutex = true;
				DrawingMutex.lock();
			}

			// Do the thing.
			Render(MouseClass::Instance);

			// We're done. Unlock all mutexes.
			DrawingDemandsDrawingMutex = false;
			DrawingMutex.unlock();
			PauseMutex.unlock();
		}
	}
}

// Before the MainLoop/AuxLoop begins in MainGame.
DEFINE_HOOK(0x48CE7E, MainGame_BeforeGameLoop, 7)
{
	Multithreading::InGameLoop = true;
	Multithreading::DrawingThread = std::thread(Multithreading::DrawingLoop);
	return 0;
}

// After we exited the MainLoop/AuxLoop.
DEFINE_HOOK(0x48CEAF, MainGame_AfterGameLoop, 5)
{
	Multithreading::InGameLoop = false;
	Multithreading::DrawingThread.detach();
	Multithreading::DrawingThread.~thread();
	return 0;
}

// Completely skip vanilla GScreenClass::Render code in the main thread.
DEFINE_HOOK(0x4F4480, GScreenClass_Render_Disable, 0)
{
	return 0x4F45A8;
}

// We want to lock access to game resources when we're doing game logic potientially realted to graphics.
// The main thread should let the drawing thread run if it complains that it's too hungry.
// TODO: Try to hook later for shorter lock period.
DEFINE_HOOK(0x55D878, MainLoop_StartLock, 6)
{
	while (Multithreading::DrawingDemandsDrawingMutex)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	Multithreading::DrawingMutex.lock();
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
DEFINE_HOOK(0x683FB2, ResmueGame_ResetPause, 5)
{
	Multithreading::PauseMutex.unlock();
	return 0;
}

// Old things below.

/*
// Main stuff
DEFINE_HOOK(0x55D84F, MainLoop_SkipRender_1, 0)
{
	return 0x55D854;
}

DEFINE_HOOK(0x55D8F2, MainLoop_SkipRender_2, 0)
{
	return 0x55D8F7;
}

DEFINE_HOOK(0x55DBBE, MainLoop_SkipRender_3, 0)
{
	return 0x55DBC3;
}

DEFINE_HOOK(0x53B66B, MainLoop_SkipRender_3, 0)
{
	return 0x53B670;
}

DEFINE_HOOK(0x683F95, PauseGame_SkipRender, 0)
{
	return 0x683F9F;
}

// Mouse stuff
DEFINE_HOOK(0x7BA204, ProcessMouse_SkipBlit_1, 0)
{
	return 0x7BA20B;
}

DEFINE_HOOK(0x7BA26C, ProcessMouse_SkipBlit_2, 0)
{
	return 0x7BA282;
}
*/

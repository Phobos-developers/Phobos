#include <Helpers/Macro.h>

#include <GScreenClass.h>
#include <WWMouseClass.h>
#include <TacticalClass.h>
#include <GadgetClass.h>
#include <MessageListClass.h>
#include <CCToolTip.h>
#include <MouseClass.h>

#include <thread>

namespace Multithreading
{
	static constexpr reference<bool, 0xB0B519u> const BlitMouse {};
	static constexpr reference<bool, 0xA9FAB0u> const IonStormClass_ChronoScreenEffect_Status {};
	static constexpr reference<GadgetClass*, 0xA8EF54u> const Buttons {};
	static constexpr reference<bool, 0xA8ED6Bu> const DebugDebugDebug {};
	static constexpr reference<bool, 0xA8B8B4u> const EnableMultiplayerDebug {};

	std::thread DrawingThread;
	bool SomethingWentWrong = false;
	bool InGameLoop = false;
	bool IsPaused = false;

	void MultiplayerDebugPrint()
		{ JMP_STD(0x55F1E0); }

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

	void DrawingLoop()
	{
		while (InGameLoop)
		{
			// A failed attempt at smooth mouse handling outside of the mouse thread
			/*
			if ((WWMouseClass::Instance->XYOld.X != WWMouseClass::Instance->XY1.X)
				|| (WWMouseClass::Instance->XYOld.Y != WWMouseClass::Instance->XY1.Y)
				|| WWMouseClass::DoneSomething.get())
			{
				WWMouseClass::Instance->SomeBlit();
				WWMouseClass::Instance->SomeDraw(WWMouseClass::Instance->Surface, 0, 0);
			}
			*/

			// TODO: Exit when the main thread raises an exception (how can we know that?)
			if (SomethingWentWrong)
				return;

			// NOTE: Busy wait is the worst, but let's leave performance for later :)
			if (!IsPaused)
				Render(MouseClass::Instance);
		}
	}
}

DEFINE_HOOK(0x48CE7E, MainGame_BeforeGameLoop, 7)
{
	Multithreading::InGameLoop = true;
	Multithreading::DrawingThread = std::thread(Multithreading::DrawingLoop);
	return 0;
}

DEFINE_HOOK(0x48CEAF, MainGame_AfterGameLoop, 5)
{
	Multithreading::InGameLoop = false;
	Multithreading::DrawingThread.detach();
	Multithreading::DrawingThread.~thread();
	return 0;
}

DEFINE_HOOK(0x4F4480, GScreenClass_Render_Disable, 0)
{
	return 0x4F45A8;
}

DEFINE_HOOK(0x683EB6, PauseGame_SetPause, 6)
{
	Multithreading::IsPaused = true;
	return 0;
}

DEFINE_HOOK(0x683FB2, ResmueGame_ResetPause, 5)
{
	Multithreading::IsPaused = false;
	return 0;
}

// Main stuff
/*
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
*/

// Mouse stuff
/*
DEFINE_HOOK(0x7BA204, ProcessMouse_SkipBlit_1, 0)
{
	return 0x7BA20B;
}

DEFINE_HOOK(0x7BA26C, ProcessMouse_SkipBlit_2, 0)
{
	return 0x7BA282;
}
*/

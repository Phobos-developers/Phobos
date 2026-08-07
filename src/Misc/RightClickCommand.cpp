#include <Phobos.h>

#include <MapClass.h>
#include <ObjectClass.h>
#include <DisplayClass.h>
#include <GeneralDefinitions.h>
#include <Ext/Techno/Body.h> // pulls in the complete FootClass/TechnoClass definitions that
                             // MapClass/DisplayClass inline abstract_casts require

// Right-click to command: the right mouse button issues orders, the left mouse button only
// selects / deploys (and deselects when clicking empty ground), matching the control style
// of modern RTS games. Off by default; enable with [Phobos] -> RightClickCommand.
//
// The tactical mouse message handler is MouseClass method 0x6930A0, which dispatches Windows
// mouse messages through a jump table (0x693410). Relevant cases (reverse-engineered):
//   LBUTTONDOWN 0x201 -> 0x693126  (begin select/action; sets the drag flag [this+0x555A]=1)
//   LBUTTONUP   0x202 -> 0x6931F9  (command dispatch: ProcessClickCoords -> DecideAction
//                                   -> apply 0x4AB9B0, from 0x69323E onward)
//   RBUTTONUP   0x205 -> 0x693366  (vanilla: cancel current mode, then deselect)
//
// RBUTTONUP and LBUTTONUP are two cases of the SAME function, sharing one stack frame and
// the same `this`; both prologues turn [esp+0x34] from a message-point pointer into an inline
// Point2D. So the RMB-up handler can jump straight into the LMB-up command dispatch at
// 0x69323E, reusing 100% of the game's command (and network) logic - no argument
// reconstruction is needed.

namespace RightClickCommand
{
	// A special LEFT-click mode is active - RMB must keep its vanilla "cancel" behaviour
	// (cancel building placement, leave repair/sell/power/beacon/superweapon targeting) and
	// LMB must keep its vanilla behaviour (repair/sell/place/target).
	static bool InSpecialLeftClickMode()
	{
		auto& d = DisplayClass::Instance;
		return d.RepairMode
			|| d.SellMode
			|| d.PowerToggleMode
			|| d.PlaceBeaconMode
			|| d.PlanningMode
			|| d.CurrentSWTypeIndex >= 0    // superweapon targeting
			|| d.CurrentBuilding != nullptr; // building placement
	}

	// Set by the RBUTTONUP command hook right before it jumps into the shared LMB-up command
	// dispatch (0x69323E), which flows through the LBUTTONUP neutralise hook at 0x693276.
	// Without this flag that hook would downgrade the RMB-issued order to None. Not static:
	// MultiClickTypeSelect.cpp reads and clears it at 0x693290, the last point the redirect
	// passes through.
	bool RmbCommandInProgress = false;

	// Tick of the click that last selected something, used by HoldsOffDeploy below.
	static DWORD LastSelectTick = 0;

	// Actions the LEFT button may still perform: selection and self-deploy only. Everything
	// else (Move/Attack/Enter/Harvest/Capture/Guard/...) is a command and belongs to the
	// RIGHT button now.
	static bool IsLeftClickAllowed(Action action)
	{
		switch (action)
		{
		case Action::None:
		case Action::Select:
		case Action::ToggleSelect:
		case Action::Self_Deploy:
			return true;
		default:
			return false;
		}
	}

	// If right-click-to-command is on and no special left-click mode is active, downgrade the
	// command action (in EAX, freshly returned by DecideAction) to None so the left button
	// only selects/deploys. Returns true if a command was actually neutralised (the click
	// landed on empty ground / an enemy - a command target, not a selectable own unit).
	static bool NeutraliseLeftCommand(REGISTERS* R)
	{
		if (Phobos::Config::RightClickCommand && !InSpecialLeftClickMode())
		{
			if (!IsLeftClickAllowed(static_cast<Action>(R->EAX())))
			{
				R->EAX(static_cast<DWORD>(Action::None));
				return true;
			}
		}
		return false;
	}

	// A deployable unit is deployed by clicking it again once it is selected, which collides
	// with the double-click type select: the second click would unpack the MCV instead. So
	// for a short while after a click selected something, the left button does not deploy.
	// Same trick Emperor: Battle for Dune uses. Only active with TypeSelectByMultiClick on.
	static bool HoldsOffDeploy(Action action)
	{
		const int delay = Phobos::Config::TypeSelectByMultiClick_DeployDelay;

		if (action != Action::Self_Deploy || !Phobos::Config::TypeSelectByMultiClick || delay <= 0)
			return false;

		return GetTickCount() - LastSelectTick <= static_cast<DWORD>(delay);
	}

	// Mouse flags RadarClass::GetMouseAction (0x6539D0) is called with, in its first stack
	// argument. The "up" bits mean the button is not down, i.e. the cursor is just hovering.
	enum RadarInput : BYTE
	{
		LeftPress = 0x01, LeftHeld = 0x02, LeftRelease = 0x04, LeftUp = 0x08,
		RightPress = 0x10, RightHeld = 0x20, RightRelease = 0x40, RightUp = 0x80,
	};

	// Swap the two buttons for the minimap, keeping the hover bits as they are so the cursor
	// still updates while moving over the radar.
	static BYTE SwapRadarButtons(BYTE flags)
	{
		const BYTE left = flags & (LeftPress | LeftHeld | LeftRelease);
		const BYTE right = flags & (RightPress | RightHeld | RightRelease);

		return static_cast<BYTE>((flags & (LeftUp | RightUp)) | (left << 4) | (right >> 4));
	}

	// Runs after DecideAction on both left button hooks. Returns true if the action was
	// downgraded to a command-less None because of the deploy hold-off, which unlike a
	// neutralised command must NOT deselect - the unit stays selected and waits.
	static bool ApplyDeployHoldOff(REGISTERS* R)
	{
		const auto action = static_cast<Action>(R->EAX());

		if (action == Action::Select || action == Action::ToggleSelect)
			LastSelectTick = GetTickCount();

		if (!HoldsOffDeploy(action))
			return false;

		R->EAX(static_cast<DWORD>(Action::None));
		return true;
	}
}

// RBUTTONUP handler, just past its "press/drag in progress" gate (cmp [this+0x555A],bl /
// je 0x693408 at 0x69338F), so it inherits the same gate the vanilla deselect uses. Stolen
// bytes: cmp byte ptr [0x884D40], bl (absolute operand, safe to relocate).
//
// The LMB-up dispatch we jump into (0x69323E) reads the click point as &[esp+0x10], i.e. the
// view-relative coords (raw window xy minus the tactical view origin at 0x886FA0/0x886FA4).
// The LMB prologue stores those at [esp+0x10]/[esp+0x14]; the RMB prologue computes the same
// values but only passes them to 0x63AB00 without storing them, so we populate the slots
// ourselves before jumping (otherwise ProcessClickCoords reads stale stack and the order
// lands on the wrong cell).
DEFINE_HOOK(0x693397, TacticalMsgHandler_RButtonUp_RightClickCommand, 0x6)
{
	if (Phobos::Config::RightClickCommand
		&& ObjectClass::CurrentObjects.Count > 0
		&& !RightClickCommand::InSpecialLeftClickMode())
	{
		// Raw packed window xy stashed by the RMB prologue at [esp+0x34].
		const int packed = R->Stack<int>(0x34);
		const int rawX = static_cast<short>(packed & 0xFFFF);
		const int rawY = static_cast<short>((packed >> 16) & 0xFFFF);

		const int originX = *reinterpret_cast<int*>(0x886FA0);
		const int originY = *reinterpret_cast<int*>(0x886FA4);

		// Feed the LMB-up command dispatch the view-relative click point.
		R->Stack(0x10, rawX - originX);
		R->Stack(0x14, rawY - originY);

		// Tell the LBUTTONUP neutralise hook to leave this (RMB-issued) command alone.
		RightClickCommand::RmbCommandInProgress = true;

		// Issue the order to the current selection instead of deselecting.
		return 0x69323E;
	}

	// Vanilla: run the stolen compare and fall through to the cancel/deselect path.
	return 0;
}

// The minimap needs the same treatment. RadarClass::GetMouseAction (0x6539D0) decides what a
// click on the radar does, and vanilla already commands from there: the LEFT button runs the
// same applier the tactical view uses (0x4AB9B0, called at 0x653D58) and only moves the view
// when there is nothing to command, while the RIGHT button just moves the view. The whole
// function reads the buttons out of its flags argument, so swapping the button bits there
// once turns it around: right commands, left moves the view.
//
// Hooked right at the top, before the first read of the argument. Stolen bytes: mov dl,
// byte ptr [esp+0x48] (the flags we just rewrote) + push ebx.
DEFINE_HOOK(0x6539D3, RadarClass_GetMouseAction_RightClickCommand, 0x5)
{
	if (Phobos::Config::RightClickCommand && !RightClickCommand::InSpecialLeftClickMode())
		R->Stack8(0x48, RightClickCommand::SwapRadarButtons(R->Stack8(0x48)));

	return 0;
}

// LBUTTONDOWN: neutralise a command action right after DecideAction so button-down does not
// preview/issue a command. Stolen bytes: mov reg,[esp+..] + push eax (the possibly-modified
// EAX is what the following push forwards to the applier).
DEFINE_HOOK(0x6931B4, TacticalMsgHandler_LButtonDown_RightClickSelectOnly, 0x5)
{
	if (!RightClickCommand::ApplyDeployHoldOff(R))
		RightClickCommand::NeutraliseLeftCommand(R);

	return 0;
}

// LBUTTONUP: same neutralise, and turn a would-be command on empty ground / an enemy into a
// deselect (MapClass::UnselectAll). Clicking own units (Select/ToggleSelect) or deploying
// (Self_Deploy) is not neutralised, so it selects/deploys as normal - no deselect there.
// Completed band-selects never reach here (0x63A8E0 consumes them and early-exits at
// 0x693408), so deselecting on a neutralised command is always correct.
DEFINE_HOOK(0x693276, TacticalMsgHandler_LButtonUp_RightClickSelectOnly, 0x5)
{
	// If we arrived here via the RMB command redirect (0x69323E), let the order stand. The
	// flag is cleared further along the same path, at 0x693290 in MultiClickTypeSelect.cpp.
	if (RightClickCommand::RmbCommandInProgress)
		return 0;

	// A held-off deploy leaves the unit selected, so no deselect here.
	if (RightClickCommand::ApplyDeployHoldOff(R))
		return 0;

	if (RightClickCommand::NeutraliseLeftCommand(R))
		MapClass::UnselectAll();

	return 0;
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include <ColorScheme.h>
#include <GameOptionsClass.h>
#include <FPSCounter.h>
#include <SessionClass.h>
#include <Phobos.h>

#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>

namespace TimerValueTemp
{
	static int oldValue;
};

DEFINE_HOOK(0x6D4B50, PrintTimerOnTactical_Start, 0x6)
{
	if (!Phobos::Config::RealTimeTimers)
		return 0;

	REF_STACK(int, value, STACK_OFFSET(0, 0x4));
	TimerValueTemp::oldValue = value;

	if (Phobos::Config::RealTimeTimers_Adaptive
		|| GameOptionsClass::Instance.GameSpeed == 0
		|| (Phobos::Misc::CustomGS && !SessionClass::IsMultiplayer()))
	{
		value = (int)((double)value / (std::max((double)FPSCounter::CurrentFrameRate, 1.0) / 15.0));
		return 0;
	}

	switch (GameOptionsClass::Instance.GameSpeed)
	{
	case 1:	// 60 FPS
		value = value / 4;
		break;
	case 2:	// 30 FPS
		value = value / 2;
		break;
	case 3:	// 20 FPS
		value = (value * 3) / 4;
		break;
	case 4:	// 15 FPS
		break;
	case 5:	// 12 FPS
		value = (value * 5) / 4;
		break;
	case 6:	// 10 FPS
		value = (value * 3) / 2;
		break;
	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6D4C68, PrintTimerOnTactical_End, 0x8)
{
	if (!Phobos::Config::RealTimeTimers)
		return 0;

	REF_STACK(int, value, STACK_OFFSET(0x654, 0x4));
	value = TimerValueTemp::oldValue;
	return 0;
}

DEFINE_HOOK(0x6D4CD9, PrintTimerOnTactical_BlinkColor, 0x6)
{
	enum { SkipGameCode = 0x6D4CE2 };

	R->EDI(ColorScheme::Array.GetItem(RulesExt::Global()->TimerBlinkColorScheme));

	return SkipGameCode;
}

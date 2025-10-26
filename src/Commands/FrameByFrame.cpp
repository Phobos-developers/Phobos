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
#include "FrameByFrame.h"

#include <SessionClass.h>
#include <TacticalClass.h>
#include <Utilities/GeneralUtils.h>

#include <Unsorted.h>

size_t FrameByFrameCommandClass::FrameStepCount = 0;
bool FrameByFrameCommandClass::FrameStep = false;

const char* FrameByFrameCommandClass::GetName() const
{
	return "FrameByFrame";
}

const wchar_t* FrameByFrameCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_FRAME_BY_FRAME", L"Toggle Frame By Frame");
}

const wchar_t* FrameByFrameCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* FrameByFrameCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_FRAME_BY_FRAME_DESC", L"Enter or exit frame by frame mode.");
}

void FrameByFrameCommandClass::Execute(WWKey eInput) const
{
	if (!SessionClass::IsSingleplayer())
		return;

	if (!FrameStep)
		Debug::LogAndMessage("Entering Stepping Mode...\n");
	else
		Debug::LogAndMessage("Exiting Stepping Mode...\n");

	FrameStep = !FrameStep;
	FrameStepCount = 0;
}

bool FrameByFrameCommandClass::FrameStepMainLoop()
{
	if (Game::IsActive)
	{
		Game::CallBack();

		if (Game::IsFocused && Game::SpecialDialog == 0)
		{
			MapClass::Instance.MarkNeedsRedraw(2);

			DWORD input;
			int x, y;
			MapClass::Instance.GetInputAndUpdate(input, x, y);
			if (input != NULL)
			{
				Game::KeyboardProcess(input);

				// Allow to open options
				if (input == VK_ESCAPE || input == VK_SPACE)
					Game::SpecialDialog = 1;
			}

			MapClass::Instance.Render();
			TacticalClass::Instance->Update();
		}
	}

	Sleep(1);
	return Game::IsActive == false;
}

DEFINE_HOOK(0x55D360, MainLoop_FrameStep_Begin, 0x5)
{
	// FrameStep mode enabled but no frames to process
	if (FrameByFrameCommandClass::FrameStep && FrameByFrameCommandClass::FrameStepCount == 0)
	{
		R->EAX(FrameByFrameCommandClass::FrameStepMainLoop());
		return 0x55DEDB;
	}

	// This frame need to be processed
	return 0;
}

DEFINE_HOOK_AGAIN(0x55D871, MainLoop_FrameStep_End, 0x6)
DEFINE_HOOK_AGAIN(0x55DEC1, MainLoop_FrameStep_End, 0x6)
DEFINE_HOOK(0x55DED5, MainLoop_FrameStep_End, 0x6)
{
	// This frame is processed, decrease the counter
	if (FrameByFrameCommandClass::FrameStep && FrameByFrameCommandClass::FrameStepCount > 0)
		--FrameByFrameCommandClass::FrameStepCount;

	return 0;
}

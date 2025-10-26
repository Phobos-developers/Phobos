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
#pragma once

#include "Commands.h"

#include "FrameByFrame.h"

template<size_t Frame>
class FrameStepCommandClass : public CommandClass
{
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t Frame>
inline const char* FrameStepCommandClass<Frame>::GetName() const
{
	_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Step%dFrames", Frame);
	return Phobos::readBuffer;
}

template<size_t Frame>
inline const wchar_t* FrameStepCommandClass<Frame>::GetUIName() const
{
	const wchar_t* csfString = StringTable::TryFetchString("TXT_STEP_XX_FORWARD", L"Step Forward %d Frames");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Frame);
	return Phobos::wideBuffer;
}

template<size_t Frame>
inline const wchar_t* FrameStepCommandClass<Frame>::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

template<size_t Frame>
inline const wchar_t* FrameStepCommandClass<Frame>::GetUIDescription() const
{
	const wchar_t* csfString = StringTable::TryFetchString("TXT_STEP_XX_FORWARD_DESC", L"Frame Step Only: Step forward %d frames.");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Frame);
	return Phobos::wideBuffer;
}

template<size_t Frame>
inline void FrameStepCommandClass<Frame>::Execute(WWKey eInput) const
{
	if (!FrameByFrameCommandClass::FrameStep)
		return;

	Debug::LogAndMessage("Stepping %d frames...\n", Frame);

	FrameByFrameCommandClass::FrameStepCount = Frame;
}

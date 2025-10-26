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
#include <TriggerClass.h>

#include <Helpers/Macro.h>

#include <Ext/TEvent/Body.h>

DEFINE_HOOK(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return nIndex >= PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= PhobosTriggerEvent::LocalVariableAndIsTrue
		|| nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable
		|| nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable
		|| nIndex == static_cast<int>(TriggerEvent::LocalSet)
		? 0x72706E
		: 0x727069;
}

DEFINE_HOOK(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= PhobosTriggerEvent::GlobalVariableAndIsTrue
		|| nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable
		|| nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable
		|| nIndex == static_cast<int>(TriggerEvent::GlobalSet)
		? 0x72702E
		: 0x727029;
}

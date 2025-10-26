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


#include <FactoryClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x4FB63A, HouseClass_UnitFromFactory_DisablingEVAUnitReady, 0xF)
{
	if (!RulesExt::Global()->IsVoiceCreatedGlobal.Get())
		VoxClass::Play(GameStrings::EVA_UnitReady);

	return 0x4FB649;
}

DEFINE_HOOK(0x4FB64B, HouseClass_UnitFromFactory_VoiceCreated, 0x5)
{
	GET(TechnoClass* const, pThisTechno, ESI);
	GET(FactoryClass* const, pThisFactory, EBX);

	auto const pThisTechnoType = TechnoExt::ExtMap.Find(pThisTechno)->TypeExtData;
	if (pThisTechno->Owner->IsControlledByCurrentPlayer() && pThisTechnoType->VoiceCreated.isset())
	{
		if (RulesExt::Global()->IsVoiceCreatedGlobal.Get())
			pThisTechno->QueueVoice(pThisTechnoType->VoiceCreated);
		else
			VocClass::PlayAt(pThisTechnoType->VoiceCreated, pThisTechno->Location);
	}

	pThisFactory->CompletedProduction();
	return 0x4FB650;
}

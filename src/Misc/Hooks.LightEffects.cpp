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
#include <GameOptionsClass.h>

#include <Ext/WarheadType/Body.h>
#include <Helpers/Macro.h>

namespace LightEffectsTemp
{
	bool AlphaIsLightFlash = false;
}

DEFINE_HOOK(0x48A444, AreaDamage_Particle_LightFlashSet, 0x5)
{
	GET(WarheadTypeClass*, pWH, EDI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt->Particle_AlphaImageIsLightFlash.Get(RulesExt::Global()->WarheadParticleAlphaImageIsLightFlash))
		LightEffectsTemp::AlphaIsLightFlash = true;

	return 0;
}

DEFINE_HOOK(0x48A47E, AreaDamage_Particle_LightFlashUnset, 0x6)
{
	LightEffectsTemp::AlphaIsLightFlash = false;

	return 0;
}

DEFINE_HOOK(0x5F5053, ObjectClass_Unlimbo_AlphaImage, 0x6)
{
	enum { SkipAlphaImage = 0x5F514B };

	int detailLevel = 0;

	if (LightEffectsTemp::AlphaIsLightFlash)
	{
		if (Phobos::Config::HideLightFlashEffects)
			return SkipAlphaImage;

		detailLevel = RulesExt::Global()->LightFlashAlphaImageDetailLevel;
	}

	if (detailLevel > GameOptionsClass::Instance.DetailLevel)
		return SkipAlphaImage;

	return 0;
}

DEFINE_HOOK(0x48A62E, DoFlash_CombatLightOptions, 0x6)
{
	enum { Continue = 0x48A64A, SkipFlash = 0x48A6FA };

	if (Phobos::Config::HideLightFlashEffects)
		return SkipFlash;

	GET(WarheadTypeClass*, pWH, EDI);
	GET(const int, currentDetailLevel, EAX);
	GET(const int, damage, ECX);

	R->ESI(damage); // Restore overridden instructions.
	int detailLevel = RulesExt::Global()->CombatLightDetailLevel;

	if (pWH)
	{
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
		detailLevel = pWHExt->CombatLightDetailLevel.Get(detailLevel);

		if (pWHExt->CombatLightChance < Randomizer::Global.RandomDouble())
			return SkipFlash;

		if (pWHExt->CLIsBlack)
			R->EBX(SpotlightFlags::NoColor);
	}

	if (detailLevel <= currentDetailLevel)
		return Continue;

	return SkipFlash;
}

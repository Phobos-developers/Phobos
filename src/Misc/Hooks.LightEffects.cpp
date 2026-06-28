#include <GameOptionsClass.h>
#include <FPSCounter.h>

#include <Ext/WarheadType/Body.h>
#include <Utilities/AresFunctions.h>

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
	enum { Continue = 0x48A668, SkipFlash = 0x48A6FA };

	if (Phobos::Config::HideLightFlashEffects)
		return SkipFlash;

	GET(WarheadTypeClass*, pWH, EDI);
	GET(const int, currentDetailLevel, EAX);
	GET(const int, damage, ECX);
	GET(const int, bitmask, EBX);
	GET_STACK(const bool, forced, STACK_OFFSET(0xC, 0x10));

	R->ESI(damage); // Restore overridden instructions.

	int detailLevel = RulesExt::Global()->CombatLightDetailLevel;
	bool checkColored = RulesExt::Global()->CombatLightDetailLevel_CheckColored;

	if (pWH)
	{
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

		if (pWHExt->CombatLightChance < Randomizer::Global.RandomDouble())
			return SkipFlash;

		detailLevel = pWHExt->CombatLightDetailLevel.Get(detailLevel);
		checkColored = pWHExt->CombatLightDetailLevel_CheckColored.Get(checkColored);

		if (pWHExt->CLIsBlack)
			R->EBX(SpotlightFlags::NoColor);
	}

	bool fpsOK = false;

	// Ares has additional checks for detail level frame rate thing concerning game speed settings.
	// For consistency's sake use the Ares check if available, original game logic otherwise.
	if (AresFunctions::DetailsCurrentlyEnabled)
		fpsOK = AresFunctions::DetailsCurrentlyEnabled();
	else
		fpsOK = !Detail::ReduceEffects();

	// (bitmask & 0xF) != 0) is true if any color channel is disabled.
	if (((detailLevel <= currentDetailLevel && fpsOK) || (!checkColored && ((bitmask & 0xF) != 0))) && (forced || (pWH && pWH->Bright)))
		return Continue;

	return SkipFlash;
}

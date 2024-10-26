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

	if (detailLevel > GameOptionsClass::Instance->DetailLevel)
		return SkipAlphaImage;

	return 0;
}

DEFINE_HOOK(0x48A62E, DoFlash_CombatLightOptions, 0x6)
{
	enum { Continue = 0x48A64A, SkipFlash = 0x48A6FA };

	if (Phobos::Config::HideLightFlashEffects)
		return SkipFlash;

	GET(WarheadTypeClass*, pWH, EDI);
	GET(int, currentDetailLevel, EAX);
	GET(int, damage, ECX);

	R->ESI(damage); // Restore overridden instructions.
	int detailLevel = RulesExt::Global()->CombatLightDetailLevel;

	if (pWH)
	{
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
		detailLevel = pWHExt->CombatLightDetailLevel.Get(detailLevel);

		if (pWHExt->CombatLightChance < Randomizer::Global->RandomDouble())
			return SkipFlash;

		if (pWHExt->CLIsBlack)
			R->EBX(SpotlightFlags::NoColor);
	}

	if (detailLevel <= currentDetailLevel)
		return Continue;

	return SkipFlash;
}

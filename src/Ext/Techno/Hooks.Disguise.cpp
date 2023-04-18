#include "Body.h"

#include <Utilities/EnumFunctions.h>

DEFINE_HOOK_AGAIN(0x522790, TechnoClass_DefaultDisguise, 0x6) // InfantryClass_SetDisguise_DefaultDisguise
DEFINE_HOOK(0x6F421C, TechnoClass_DefaultDisguise, 0x6) // TechnoClass_DefaultDisguise
{
	GET(TechnoClass*, pThis, ESI);

	enum { SetDisguise = 0x5227BF, DefaultDisguise = 0x6F4277 };

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->DefaultDisguise.isset())
		{
			pThis->Disguise = pExt->DefaultDisguise;
			pThis->Disguised = true;
			return R->Origin() == 0x522790 ? SetDisguise : DefaultDisguise;
		}
	}

	pThis->Disguised = false;

	return 0;
}

__forceinline bool CanBlinkDisguise(TechnoClass* pTechno)
{
	return HouseClass::IsCurrentPlayerObserver()
		|| EnumFunctions::CanTargetHouse(RulesExt::Global()->DisguiseBlinkingVisibility, HouseClass::CurrentPlayer, pTechno->Owner);
}

DEFINE_HOOK(0x4DEDCB, FootClass_GetImage_DisguiseBlinking, 0x7)
{
	enum { ContinueChecks = 0x4DEDDB, AllowBlinking = 0x4DEE15 };

	GET(TechnoClass*, pThis, ESI);

	if (CanBlinkDisguise(pThis))
		return AllowBlinking;

	return ContinueChecks;
}

DEFINE_HOOK(0x70EE70, TechnoClass_IsClearlyVisibleTo_DisguiseBlinking, 0x5)
{
	enum { ContinueChecks = 0x70EE79, DisallowBlinking = 0x70EEB6 };

	GET(TechnoClass*, pThis, ESI);

	if (CanBlinkDisguise(pThis))
		return ContinueChecks;

	return DisallowBlinking;
}

DEFINE_HOOK(0x7062F5, TechnoClass_TechnoClass_DrawObject_DisguiseBlinking, 0x6)
{
	enum { ContinueChecks = 0x706304, DisallowBlinking = 0x70631F };

	GET(TechnoClass*, pThis, ESI);

	if (CanBlinkDisguise(pThis))
		return ContinueChecks;

	return DisallowBlinking;
}

DEFINE_HOOK(0x7060A9, TechnoClass_TechnoClass_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	LightConvertClass* convert = nullptr;

	auto const pType = pThis->IsDisguised() ? TechnoTypeExt::GetTechnoType(pThis->Disguise) : nullptr;
	int colorIndex = pThis->GetDisguiseHouse(true)->ColorSchemeIndex;

	if (pType && pType->Palette && pType->Palette->Count > 0)
		convert = pType->Palette->GetItem(colorIndex)->LightConvert;
	else
		convert = ColorScheme::Array->GetItem(colorIndex)->LightConvert;

	R->EBX(convert);

	return SkipGameCode;
}

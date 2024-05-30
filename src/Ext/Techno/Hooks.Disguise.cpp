#include "Body.h"

#include <Utilities/EnumFunctions.h>

DEFINE_HOOK_AGAIN(0x522790, TechnoClass_DefaultDisguise, 0x6) // InfantryClass_SetDisguise_DefaultDisguise
DEFINE_HOOK(0x6F421C, TechnoClass_DefaultDisguise, 0x6) // TechnoClass_DefaultDisguise
{
	GET(TechnoClass*, pThis, ESI);

	enum { SetDisguise = 0x5227BF, DefaultDisguise = 0x6F4277 };

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->DefaultDisguise)
		{
			pThis->Disguise = pExt->DefaultDisguise;
			pThis->Disguised = true;
			return R->Origin() == 0x522790 ? SetDisguise : DefaultDisguise;
		}
	}

	pThis->Disguised = false;

	return 0;
}

__forceinline bool CanBlinkDisguise(HouseClass* pCurrent, HouseClass* pTarget)
{
	if (!pCurrent || !pTarget)
		return false;

	if (SessionClass::IsCampaign() && (pTarget->IsHumanPlayer || pTarget->IsInPlayerControl))
	{
		if ((RulesExt::Global()->DisguiseBlinkingVisibility & AffectedHouse::Allies) != AffectedHouse::None && pCurrent->IsAlliedWith(pTarget))
			return true;

		return (RulesExt::Global()->DisguiseBlinkingVisibility & AffectedHouse::Owner) != AffectedHouse::None;
	}

	return pCurrent->IsObserver() || EnumFunctions::CanTargetHouse(RulesExt::Global()->DisguiseBlinkingVisibility, pCurrent, pTarget);
}

bool __fastcall IsAlly_Wrapper(HouseClass* pThis, void* _, HouseClass* pOther)
{
	return pThis->IsObserver() || pOther->IsAlliedWith(pOther) || (RulesExt::Global()->DisguiseBlinkingVisibility & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool __fastcall IsControlledByCurrentPlayer_Wrapper(HouseClass* pThis)
{
	return CanBlinkDisguise(HouseClass::CurrentPlayer, pThis);
}

DEFINE_JUMP(CALL, 0x4DEDD2, GET_OFFSET(IsAlly_Wrapper));                      // FootClass_GetImage
DEFINE_JUMP(CALL, 0x70EE5D, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_ClearlyVisibleTo
DEFINE_JUMP(CALL, 0x70EE70, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_ClearlyVisibleTo
DEFINE_JUMP(CALL, 0x7062FB, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_DrawObject

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

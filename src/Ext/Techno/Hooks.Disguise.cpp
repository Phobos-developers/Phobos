#include "Body.h"

#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x522790, InfantryClass_ClearDisguise_DefaultDisguise, 0x6)
{
	GET(InfantryClass*, pThis, ECX);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->DefaultDisguise)
	{
		pThis->Disguise = pExt->DefaultDisguise;
		pThis->Disguised = true;
		return 0x5227BF;
	}

	pThis->Disguised = false;

	return 0;
}

DEFINE_HOOK(0x6F421C, TechnoClass_Init_DefaultDisguise, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	// mirage is not here yet
	if (pThis->WhatAmI() == AbstractType::Infantry && pExt->DefaultDisguise)
	{
		pThis->Disguise = pExt->DefaultDisguise;
		pThis->Disguised = true;
		return 0x6F4277;
	}

	pThis->Disguised = false;

	return 0;
}

#pragma region DisguiseBlinkingVisibility

bool __fastcall IsAlly_Wrapper(HouseClass* pThis, void* _, HouseClass* pOther)
{
	return pThis->IsObserver() || pOther->IsAlliedWith(pOther) || (RulesExt::Global()->DisguiseBlinkingVisibility & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool __fastcall IsControlledByCurrentPlayer_Wrapper(HouseClass* pThis)
{
	HouseClass* pCurrent = HouseClass::CurrentPlayer;
	AffectedHouse visibilityFlags = RulesExt::Global()->DisguiseBlinkingVisibility;

	if (SessionClass::IsCampaign() && (pThis->IsHumanPlayer || pThis->IsInPlayerControl))
	{
		if ((visibilityFlags & AffectedHouse::Allies) != AffectedHouse::None && pCurrent->IsAlliedWith(pThis))
			return true;

		return (visibilityFlags & AffectedHouse::Owner) != AffectedHouse::None;
	}

	return pCurrent->IsObserver() || EnumFunctions::CanTargetHouse(visibilityFlags, pCurrent, pThis);
}

DEFINE_JUMP(CALL, 0x4DEDD2, GET_OFFSET(IsAlly_Wrapper));                      // FootClass_GetImage
DEFINE_JUMP(CALL, 0x70EE5D, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_ClearlyVisibleTo
DEFINE_JUMP(CALL, 0x70EE70, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_ClearlyVisibleTo
DEFINE_JUMP(CALL, 0x7062FB, GET_OFFSET(IsControlledByCurrentPlayer_Wrapper)); // TechnoClass_DrawObject

#pragma endregion

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

DEFINE_HOOK(0x74691D, UnitClass_UpdateDisguise_EMP, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	// Remove mirage disguise if under emp or being flipped, approximately 15 deg
	if (pThis->IsUnderEMP() || std::abs(pThis->AngleRotatedForwards) > 0.25 || std::abs(pThis->AngleRotatedSideways) > 0.25)
	{
		pThis->ClearDisguise();
		R->EAX(pThis->MindControlRingAnim);
		return 0x746AA5;
	}

	return 0x746931;
}

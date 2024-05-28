#include <BuildingClass.h>
#include <Drawing.h>
#include <EditClass.h>
#include <FootClass.h>
#include <GeneralDefinitions.h>
#include <HouseClass.h>
#include <Surface.h>
#include <StringTable.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Rules/Body.h>

// In vanilla YR, game destroys building animations directly by calling constructor.
// Ares changed this to call UnInit() which has a consequence of doing pointer invalidation on the AnimClass pointer.
// This notably causes an issue with Grinder that restores ActiveAnim if the building is sold/destroyed while SpecialAnim is playing even if the building is gone or in limbo.
// Now it does not do this if the building is in limbo, which covers all cases from being destroyed, sold, to erased by Temporal weapons.
// There is another potential case for this with ProductionAnim & IdleAnim which is also patched here just in case.
DEFINE_HOOK_AGAIN(0x44E997, BuildingClass_Detach_RestoreAnims, 0x6)
DEFINE_HOOK(0x44E9FA, BuildingClass_Detach_RestoreAnims, 0x6)
{
	enum { SkipAnimOne = 0x44E9A4, SkipAnimTwo = 0x44EA07 };

	GET(BuildingClass*, pThis, ESI);

	if (pThis->InLimbo)
		return R->Origin() == 0x44E997 ? SkipAnimOne : SkipAnimTwo;

	return 0;
}

void __fastcall TechnoClass_DrawExtraInfo_Wrapper(TechnoClass* pThis, Point2D const& location, Point2D const& originalLocation, RectangleStruct const& bounds)
{
//Ares version of rewritten and extended TechnoClass_PrintSelectedTip
//DEFINE_HOOK(0x70AA60, TechnoClass_DrawExtraInfo, 0x6)

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pType = pBuilding->Type;
		auto const pOwner = pBuilding->Owner;
		if (!pType || !pOwner)
			JMP_THIS(0x70AD4C);
		
		auto DrawTheStuff = [&](const wchar_t* pFormat)
		{
			Point2D pLocation = const_cast<Point2D&>(originalLocation);
			RectangleStruct pRect = const_cast<RectangleStruct&>(bounds);
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, originalLocation, 256|64|9, 4, 2);
			auto nIntersect = Drawing::Intersect(nTextDimension, bounds);
			auto nColorInt = pOwner->ColorSchemeIndex;//0x63DAD0

			DSurface::Temp->FillRect(&nIntersect, (COLORREF)0);
			DSurface::Temp->DrawRect(&nIntersect, (COLORREF)nColorInt);
			
			DSurface::Temp->DrawText(pFormat, &pRect, &pLocation, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
			VocClass::PlayAt(23, pBuilding->GetCoords());
		};

		const bool IsAlly = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();
		const bool isFake = BuildingTypeExt::ExtMap.Find(pType)->Fake;
		const bool bReveal = pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

		if (IsAlly || IsObserver || bReveal)
		{
			if (isFake)
				DrawTheStuff(StringTable::LoadString("TXT_FAKE"));

			if (pType->PowerBonus > 0)
			{
				auto pDrainFormat = StringTable::LoadString(GameStrings::TXT_POWER_DRAIN2());
				wchar_t pOutDrainFormat[0x80];
				auto pDrain = (int)pOwner->Power_Drain();
				auto pOutput = (int)pOwner->Power_Output();
				swprintf_s(pOutDrainFormat, pDrainFormat, pOutput, pDrain);

				DrawTheStuff(pOutDrainFormat);
			}

			if (pType->Storage > 0)
			{
				auto pMoneyFormat = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_1());
				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, pMoneyFormat, nMoney);
				DrawTheStuff(pOutMoneyFormat);
			}

			if (pThis->IsPrimaryFactory)
			{
				//if(RulesExt::Global()->PrimaryFactoryIndicator)
				//	BuildingTypeExt::DrawPrimaryIcon(pThis, originalLocation, bounds);
				//else
					DrawTheStuff(StringTable::LoadString((pType->GetFoundationWidth() != 1) ? GameStrings::TXT_PRIMARY() : GameStrings::TXT_PRI()));
			}
		}
	}
	
	JMP_THIS(0x70AD4C);
}

DEFINE_JUMP(VTABLE, 0x7E26FC, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); //AircraftClass
DEFINE_JUMP(VTABLE, 0x7E4314, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); //BuildingClass
DEFINE_JUMP(VTABLE, 0x7E90DC, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); //FootClass
DEFINE_JUMP(VTABLE, 0x7EB4B0, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); //InfantryClass
DEFINE_JUMP(VTABLE, 0x7F4DB8, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); //TechnoClass
DEFINE_JUMP(VTABLE, 0x7F60C8, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); // UnitClass
//DEFINE_JUMP(CALL, 0x70AA60, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper)); 
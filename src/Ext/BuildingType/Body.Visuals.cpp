#include "Body.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Ext/Rules/Body.h>

void BuildingTypeExt::DrawPrimaryIcon(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
//PrimaryFactoryIndicator_Palette

	SHPStruct* pImage = RulesExt::Global()->PrimaryFactoryIndicator;
	Point2D position = { pLocation->X - pImage->Width - 20, pLocation->Y + pImage->Height + 20 };
	DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pImage, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	return;
}

void __fastcall TechnoClass_DrawExtraInfo_Wrapper(TechnoClass* pThis)
{
	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pType = pBuilding->Type;
		auto const pOwner = pThis->Owner;
		if (!pType || !pOwner)
			JMP_THIS(0x70AD4C);
		
		Point2D pLocation = TacticalClass::Instance->CoordsToClient(pBuilding->GetRenderCoords()).first;
		RectangleStruct pRect = DSurface::Temp->GetRect();
		pRect.Height -= 32;
		int const textHeight = 22;

		auto DrawTheStuff = [&](const wchar_t* pFormat)
		{
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, pLocation, 256|64|9, 4, 2);
			auto nIntersect = Drawing::Intersect(nTextDimension, pRect);
			int nColorInt = Drawing::RGB_To_Int(pOwner->Color); //0x63DAD0
			DSurface::Temp->FillRect(&nIntersect, 0);
			DSurface::Temp->DrawRectEx(&pRect, &nIntersect, nColorInt);
			nIntersect.Width -= 2;
			nIntersect.Height -= 2;
			nIntersect.X += 1;
			nIntersect.Y += 1;
			DSurface::Temp->DrawRectEx(&pRect, &nIntersect, nColorInt);
			
			//Point2D pLocation2 = {pLocation.X - 1, pLocation.Y + 1};
			//DSurface::Temp->DrawText(pFormat, &pRect, &pLocation2, (COLORREF)0, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
			DSurface::Temp->DrawTextA(pFormat, &pRect, &pLocation, nColorInt, 0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
		};

		const bool IsAlly = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();
		const bool isFake = BuildingTypeExt::ExtMap.Find(pType)->Fake;
		const bool isPrimary = pThis->IsPrimaryFactory;
		const bool hasPowerBonus = pType->PowerBonus > 0;
		const bool hasStorage = pType->Storage > 0;
		const bool bReveal = pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

		if (IsAlly || IsObserver || bReveal)
		{

			if (isPrimary)
			{
				if(RulesExt::Global()->PrimaryFactoryIndicator)
					BuildingTypeExt::DrawPrimaryIcon(pThis, &pLocation, &pRect);
				else
				{
					pLocation.Y -= textHeight/2;
					DrawTheStuff(StringTable::LoadString((pType->GetFoundationWidth() != 1) ? GameStrings::TXT_PRIMARY() : GameStrings::TXT_PRI()));
					pLocation.Y += textHeight;
				}
			}

			if (isFake)
			{
				pLocation.Y -= textHeight/2;
				DrawTheStuff(StringTable::LoadString("TXT_FAKE"));
				pLocation.Y += textHeight;
			}
			
			if (hasPowerBonus)
			{
				auto pDrainFormat = StringTable::LoadString(GameStrings::TXT_POWER_DRAIN2());
				wchar_t pOutDrainFormat[0x80];
				auto pDrain = (int)pOwner->Power_Drain();
				auto pOutput = (int)pOwner->Power_Output();
				swprintf_s(pOutDrainFormat, pDrainFormat, pOutput, pDrain);
				DrawTheStuff(pOutDrainFormat);
				pLocation.Y += textHeight;
			}

			if (hasStorage)
			{
				auto pMoneyFormat = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_1());
				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, pMoneyFormat, nMoney);
				DrawTheStuff(pOutMoneyFormat);
			}
		}
	}
	
	JMP_THIS(0x70AD4C);
}

DEFINE_JUMP(VTABLE, 0x7E26FC, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper));
DEFINE_JUMP(VTABLE, 0x7E4314, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper));
DEFINE_JUMP(VTABLE, 0x7E90DC, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper));
DEFINE_JUMP(VTABLE, 0x7EB4B0, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper));
DEFINE_JUMP(VTABLE, 0x7F4DB8, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper));
DEFINE_JUMP(VTABLE, 0x7F60C8, GET_OFFSET(TechnoClass_DrawExtraInfo_Wrapper));
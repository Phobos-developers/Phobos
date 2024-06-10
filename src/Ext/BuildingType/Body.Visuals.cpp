#include "Body.h"

#include <TacticalClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Ext/Rules/Body.h>

void BuildingTypeExt::DrawPrimaryIcon(BuildingClass* pThisBuilding, RectangleStruct* pBounds)
{
	SHPStruct* pImage = RulesExt::Global()->PrimaryFactoryIndicator;
	ConvertClass* pPalette = RulesExt::Global()->PrimaryFactoryIndicator_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	int const cellsToAdjust = pThisBuilding->Type->GetFoundationHeight(false) - 1;
	Point2D pPosition = TacticalClass::Instance->CoordsToClient(pThisBuilding->GetCell()->GetCoords()).first;
	pPosition.X -= Unsorted::CellWidthInPixels/2 * cellsToAdjust;
	pPosition.Y += Unsorted::CellHeightInPixels/2 * cellsToAdjust - 4;
	DSurface::Temp->DrawSHP(pPalette, pImage, 0, &pPosition, pBounds, BlitterFlags(0x600), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

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
		int const textHeight = 22; //temporarily hardcoded

		auto DrawTheStuff = [&](const wchar_t* pFormat)
		{
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, pLocation, 256|64|9, 4, 2);
			auto nIntersect = Drawing::Intersect(nTextDimension, pRect);
			int nColorInt = Drawing::RGB_To_Int(pOwner->Color); //0x63DAD0
			DSurface::Temp->FillRect(&nIntersect, 0);
			DSurface::Temp->DrawRectEx(&pRect, &nIntersect, nColorInt);
			/*nIntersect.Width += 2;
			nIntersect.Height -= 2;
			nIntersect.X -= 1;
			nIntersect.Y += 1;
			DSurface::Temp->DrawRectEx(&pRect, &nIntersect, nColorInt); //rounded effect for text frame*/
			DSurface::Temp->DrawTextA(pFormat, &pRect, &pLocation, nColorInt, 0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
		};

		const bool isAlly = pOwner->IsAlliedWith(HouseClass::CurrentPlayer);
		const bool isObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();
		const bool isFake = BuildingTypeExt::ExtMap.Find(pType)->Fake;
		const bool isPrimary = pThis->IsPrimaryFactory;
		const bool hasPowerBonus = pType->PowerBonus > 0 && BuildingTypeExt::ExtMap.Find(pType)->ShowPower;
		const bool hasStorage = pType->Storage > 0;
		const bool isUsingStorage = BuildingTypeExt::ExtMap.Find(pType)->Refinery_UseStorage;
		const bool bReveal = pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

		if (isAlly || isObserver || bReveal)
		{

			if (isPrimary)
			{
				if(RulesExt::Global()->PrimaryFactoryIndicator)
					BuildingTypeExt::DrawPrimaryIcon(pBuilding, &pRect);
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
				if(pType->GetFoundationWidth() > 2 && pType->GetFoundationHeight(false) > 2)
					swprintf_s(pOutDrainFormat, pDrainFormat, pOutput, pDrain);
				else
				{
					swprintf_s(pOutDrainFormat, L"Power=%d", pOutput); //needs csf
					DrawTheStuff(pOutDrainFormat);
					pLocation.Y += textHeight - 2;
					swprintf_s(pOutDrainFormat, L"Drain=%d", pDrain); //needs csf
				}
				DrawTheStuff(pOutDrainFormat);
				pLocation.Y += textHeight;
			}

			if (hasStorage && !isUsingStorage)
			{
				auto pMoneyFormat = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_1());
				wchar_t pOutMoneyFormat[0x80];
				auto nMoney = pOwner->Available_Money();
				swprintf_s(pOutMoneyFormat, pMoneyFormat, nMoney);
				DrawTheStuff(pOutMoneyFormat);
				pLocation.Y += textHeight;
			}

			if (hasStorage && isUsingStorage)
			{
				auto pStorageFormat = L"Storage: %d"; //needs csf
				wchar_t pOutStorageFormat[0x80];
				auto nStorage = pBuilding->GetStoragePercentage();
				swprintf_s(pOutStorageFormat, pStorageFormat, nStorage);
				DrawTheStuff(pOutStorageFormat);
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
#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>

static SHPStruct* LoadSHPCameo(TechnoTypeClass* pType)
{
	auto pTypeExt = TechnoTypeExt::Fetch(pType);

	if(pTypeExt->SHPCameo_IsLoad)
		return pTypeExt->SpareCameo;
	
	char pFileName[0x20];

	strcpy_s(pFileName, pTypeExt->SpareCameoFile.data());
	_strlwr_s(pFileName);

	if(!_stricmp(pFileName, pTypeExt->SpareCameoFile.data()) 
	   && !strstr(pFileName, ".pcx"))
	{
		std::string Filename(pFileName);
		Filename += ".shp";
		SHPStruct* pSHP = FileSystem::LoadSHPFile(Filename.c_str());
		pTypeExt->SHPCameo_IsLoad = true;
		pTypeExt->SpareCameo = pSHP;
		return pSHP;
	}
	return nullptr;
}

static bool DrawPCXCameo(TechnoTypeClass* pType, const int destX, const int destY)
{
	auto pTypeExt = TechnoTypeExt::Fetch(pType);
	char pFileName[0x20];

	strcpy_s(pFileName, pTypeExt->SpareCameoPCX.GetFilename());
	_strlwr_s(pFileName);

	if(!_stricmp(pFileName, pTypeExt->SpareCameoPCX.GetFilename())
       && strstr(pFileName, ".pcx"))
	{
		PCX::Instance.LoadFile(pFileName);
		if (const auto pCameoPCX = PCX::Instance.GetSurface(pFileName))
		{
			RectangleStruct bounds = { destX, destY, 60, 48 };
			return PCX::Instance.BlitToSurface(&bounds, DSurface::Sidebar, pCameoPCX);
		}
	}
	return false;
}

static void SetElite(TechnoClass* pTechno)
{
	const auto pHouseTypeExt = HouseTypeExt::Fetch(pTechno->Owner->Type);

	switch(pTechno->WhatAmI())
	{
		case AbstractType::Infantry:
		{
			if(pHouseTypeExt->EliteInfantry.Contains(abstract_cast<InfantryTypeClass*>(pTechno->GetTechnoType())))
				pTechno->Veterancy.SetElite();
			break;
		}
		case AbstractType::Unit:
		{
			if(pHouseTypeExt->EliteUnits.Contains(abstract_cast<UnitTypeClass*>(pTechno->GetTechnoType())))
				pTechno->Veterancy.SetElite();
			break;
		}
		case AbstractType::Aircraft:
		{
			if(pHouseTypeExt->EliteAircraft.Contains(abstract_cast<AircraftTypeClass*>(pTechno->GetTechnoType())))
				pTechno->Veterancy.SetElite();
			break;
		}
		default:
			break;
	}
}

DEFINE_HOOK(0x6A980A, StripClass_Draw_DrawSHPCameo, 0x8)
{
	GET(TechnoTypeClass*, pType, EBX);

	auto pHouseExt = HouseExt::Fetch(HouseClass::CurrentPlayer);
	auto pHouseTypeExt = HouseTypeExt::Fetch(HouseClass::CurrentPlayer->Type);

	SHPStruct* pSHP = nullptr;

	switch(pType->WhatAmI())
	{
		case AbstractType::InfantryType:
		{
			if(pHouseTypeExt->EliteInfantry.Contains(abstract_cast<InfantryTypeClass*>(pType)))
			{
				pHouseExt->TechnoPCX_IsLoad.push_back(pType);
				pSHP = LoadSHPCameo(pType);
			}
			break;
		}
		case AbstractType::UnitType:
		{
			if(pHouseTypeExt->EliteUnits.Contains(abstract_cast<UnitTypeClass*>(pType)))
			{
				pHouseExt->TechnoPCX_IsLoad.push_back(pType);
				pSHP = LoadSHPCameo(pType);
			}
			break;
		}
		case AbstractType::AircraftType:
		{
			if(pHouseTypeExt->EliteAircraft.Contains(abstract_cast<AircraftTypeClass*>(pType)))
			{
				pHouseExt->TechnoPCX_IsLoad.push_back(pType);
				pSHP = LoadSHPCameo(pType);
			}
			break;
		}
		default:
			break;
	}

	if(pSHP)
		R->EAX(pSHP);
	
	return 0;
}

DEFINE_HOOK(0x6A99E7, StripClass_Draw_DrawPCXCameo, 0x6)
{
	enum { SkipDrawSHP = 0x6A9A43 };

	GET(const int, destX, ESI);
	GET(const int, destY, EBP);
	GET_STACK(SHPStruct*, pSHP, STACK_OFFSET(0x48C, -0x444));

	auto pHouseExt = HouseExt::Fetch(HouseClass::CurrentPlayer);

	for(auto pType : pHouseExt->TechnoPCX_IsLoad)
	{		
		if(pSHP == pType->Cameo || pSHP == pType->AltCameo)
		{
			if(DrawPCXCameo(pType, destX, destY))
				return SkipDrawSHP;
		}
	}
	return 0;
}
// Only affects production
DEFINE_HOOK(0x443C71, BuildingClass_ExitObject_CountryInitialElite, 0x6)
{
	GET(TechnoClass*, pTechno, EDI);
	SetElite(pTechno);
	return 0;
}

DEFINE_HOOK(0x446EE8, BuildingClass_Place_OccupantsInitialElite, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	for(auto pOccupant : pThis->Occupants)
		SetElite(pOccupant);

	return 0;
}

DEFINE_HOOK(0x4D71A0, FootClass_Put_PassengersInitialElite, 0x6)
{
	GET(FootClass*, pFoot, ESI);

	for(auto pPassenger = pFoot->Passengers.FirstPassenger; pPassenger ; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject))
		SetElite(pPassenger);
	
	return 0;
}
// Affects all
DEFINE_HOOK(0x517CB4, InfantryClass_InitialElite, 0x5)
{
	GET(InfantryClass*, pInfantry, EAX);

	const auto pHouseTypeExt = HouseTypeExt::Fetch(pInfantry->Owner->Type);

	if(pHouseTypeExt->Elite_AffectsAll)
		SetElite(pInfantry);
	return 0;
}

DEFINE_HOOK(0x73577A, UnitClass_InitialElite, 0x6)
{
	GET(UnitClass*, pUnit, EAX);

	const auto pHouseTypeExt = HouseTypeExt::Fetch(pUnit->Owner->Type);

	if(pHouseTypeExt->Elite_AffectsAll)
		SetElite(pUnit);
	return 0;
}

DEFINE_HOOK(0x413F6C, Aircraft_InitialElite, 0x5)
{
	GET(AircraftClass*, pAircraft, EAX);

	const auto pHouseTypeExt = HouseTypeExt::Fetch(pAircraft->Owner->Type);

	if(pHouseTypeExt->Elite_AffectsAll)
		SetElite(pAircraft);
	return 0;
}

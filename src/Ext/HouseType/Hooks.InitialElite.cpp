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
			if(pHouseTypeExt->EliteInfantry.Contains(pType))
			{
				pHouseExt->TechnoPCX_IsLoad.push_back(pType);
				pSHP = LoadSHPCameo(pType);
			}
			break;
		}
		case AbstractType::UnitType:
		{
			if(pHouseTypeExt->EliteUnits.Contains(pType))
			{
				pHouseExt->TechnoPCX_IsLoad.push_back(pType);
				pSHP = LoadSHPCameo(pType);
			}
			break;
		}
		case AbstractType::AircraftType:
		{
			if(pHouseTypeExt->EliteAircraft.Contains(pType))
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

DEFINE_HOOK(0x443C71, BuildingClass_ExitObject_CountryInitialElite, 0x6)
{
	GET(TechnoClass*, pTechno, EDI);

	const auto pHouseTypeExt = HouseTypeExt::Fetch(pTechno->Owner->Type);

	switch(pTechno->WhatAmI())
	{
		case AbstractType::Infantry:
		{
			if(pHouseTypeExt->EliteInfantry.Contains(pTechno->GetTechnoType()))
				pTechno->Veterancy.SetElite();
			break;
		}
		case AbstractType::Unit:
		{
			if(pHouseTypeExt->EliteUnits.Contains(pTechno->GetTechnoType()))
				pTechno->Veterancy.SetElite();
			break;
		}
		case AbstractType::Aircraft:
		{
			if(pHouseTypeExt->EliteAircraft.Contains(pTechno->GetTechnoType()))
				pTechno->Veterancy.SetElite();
			break;
		}
		default:
			break;
	}

	return 0;
}

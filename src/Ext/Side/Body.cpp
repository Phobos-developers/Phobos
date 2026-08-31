#include "Body.h"

#include <ThemeClass.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/HouseType/Body.h>

SideExt::ExtContainer SideExt::ExtMap;

void SideExt::Initialize()
{
	const char* pID = this->OwnerObject()->ID;

	this->ArrayIndex = SideClass::FindIndex(pID);
	this->Sidebar_GDIPositions = this->ArrayIndex == 0; // true = Allied

	// Init MessageTextColor like Ares
	if (!_strcmpi(pID, "Nod")) //Soviets
		this->MessageTextColor = 11;
	else if (!_strcmpi(pID, "ThirdSide")) //Yuri
		this->MessageTextColor = 25;
	else //Allies or any other country
		this->MessageTextColor = 21;
};

void SideExt::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	this->Sidebar_GDIPositions.Read(exINI, pSection, "Sidebar.GDIPositions");
	this->IngameScore_WinTheme = pINI->ReadTheme(pSection, "IngameScore.WinTheme", this->IngameScore_WinTheme);
	this->IngameScore_LoseTheme = pINI->ReadTheme(pSection, "IngameScore.LoseTheme", this->IngameScore_LoseTheme);
	this->Sidebar_HarvesterCounter_Offset.Read(exINI, pSection, "Sidebar.HarvesterCounter.Offset");
	this->Sidebar_HarvesterCounter_HideMaxValue.Read(exINI, pSection, "Sidebar.HarvesterCounter.HideMaxValue");
	this->Sidebar_HarvesterCounter_OnlyMaxValue.Read(exINI, pSection, "Sidebar.HarvesterCounter.OnlyMaxValue");
	this->Sidebar_HarvesterCounter_ColorGreen.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorGreen");
	this->Sidebar_HarvesterCounter_ColorYellow.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorYellow");
	this->Sidebar_HarvesterCounter_ColorRed.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorRed");
	this->Sidebar_WeedsCounter_Offset.Read(exINI, pSection, "Sidebar.WeedsCounter.Offset");
	this->Sidebar_WeedsCounter_Color.Read(exINI, pSection, "Sidebar.WeedsCounter.Color");
	this->Sidebar_ProducingProgress_Offset.Read(exINI, pSection, "Sidebar.ProducingProgress.Offset");
	this->Sidebar_PowerDelta_Offset.Read(exINI, pSection, "Sidebar.PowerDelta.Offset");
	this->Sidebar_PowerDelta_ColorGreen.Read(exINI, pSection, "Sidebar.PowerDelta.ColorGreen");
	this->Sidebar_PowerDelta_ColorYellow.Read(exINI, pSection, "Sidebar.PowerDelta.ColorYellow");
	this->Sidebar_PowerDelta_ColorRed.Read(exINI, pSection, "Sidebar.PowerDelta.ColorRed");
	this->Sidebar_PowerDelta_ColorGrey.Read(exINI, pSection, "Sidebar.PowerDelta.ColorGrey");
	this->Sidebar_PowerDelta_Align.Read(exINI, pSection, "Sidebar.PowerDelta.Align");
	this->ToolTip_Background_Color.Read(exINI, pSection, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, pSection, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, pSection, "ToolTip.Background.BlurSize");
	this->BriefingTheme = pINI->ReadTheme(pSection, "BriefingTheme", this->BriefingTheme);
	this->MessageTextColor.Read(exINI, pSection, "MessageTextColor");
	this->SuperWeaponSidebar_OnPCX.Read(pINI, pSection, "SuperWeaponSidebar.OnPCX");
	this->SuperWeaponSidebar_OffPCX.Read(pINI, pSection, "SuperWeaponSidebar.OffPCX");
	this->SuperWeaponSidebar_TopPCX.Read(pINI, pSection, "SuperWeaponSidebar.TopPCX");
	this->SuperWeaponSidebar_CenterPCX.Read(pINI, pSection, "SuperWeaponSidebar.CenterPCX");
	this->SuperWeaponSidebar_BottomPCX.Read(pINI, pSection, "SuperWeaponSidebar.BottomPCX");

	this->EVATag.Read(pINI, pSection, "EVA.Tag");
}

void SideExt::UpdateMainEvaVoice(BuildingClass* pThis)
{
	if (!pThis || !pThis->Type)
		return;

	const auto pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->NewEvaVoice_Tag < 0)
		return;

	const auto pHouse = pThis->Owner;

	if (!pHouse || !pHouse->IsControlledByCurrentPlayer())
		return;

	int newPriority = -1;
	int newEvaIndex = VoxClass::EVAIndex;
	BuildingTypeExt* pWinningTypeExt = nullptr;

	// If pThis is active (alive, not in limbo, not selling), consider it as candidate
	const bool pThisIsActive = pThis->IsAlive && pThis->Health > 0 && !pThis->InLimbo && pThis->CurrentMission != Mission::Selling;
	if (pThisIsActive && pTypeExt->NewEvaVoice_Tag >= 0)
	{
		newPriority = pTypeExt->NewEvaVoice_Priority;
		newEvaIndex = pTypeExt->NewEvaVoice_Tag;
		pWinningTypeExt = pTypeExt;
	}

	for (const auto pBuilding : pHouse->Buildings)
	{
		if (!pBuilding || !pBuilding->Type || pBuilding == pThis)
			continue;

		if (!pBuilding->IsAlive || pBuilding->Health <= 0 || pBuilding->InLimbo || pBuilding->CurrentMission == Mission::Selling)
			continue;

		const auto pBuildingTypeExt = BuildingTypeExt::Fetch(pBuilding->Type);

		if (pBuildingTypeExt->NewEvaVoice_Tag < 0)
			continue;

		// The highest priority takes precedence over lower ones
		if (pBuildingTypeExt->NewEvaVoice_Priority > newPriority)
		{
			newPriority = pBuildingTypeExt->NewEvaVoice_Priority;
			newEvaIndex = pBuildingTypeExt->NewEvaVoice_Tag;
			pWinningTypeExt = pBuildingTypeExt;
		}
	}

	if (newPriority >= 0)
	{
		if (VoxClass::EVAIndex != newEvaIndex)
		{
			VoxClass::EVAIndex = newEvaIndex;

			// Greeting of the new EVA voice
			if (pWinningTypeExt)
			{
				int idxPlay = pWinningTypeExt->NewEvaVoice_InitialMessage.Get(-1);

				if (idxPlay != -1)
					VoxClass::PlayIndex(idxPlay);
			}
		}
	}
	else
	{
		// Hierarchical Fallback:
		// 1. HouseType (Country) EVA.Tag
		// 2. Side EVA.Tag
		// 3. Vanilla SideIndex (0: Allied, 1: Russian, 2: Yuri)
		int fallbackIndex = -1;

		if (const auto pHouseTypeExt = HouseTypeExt::Fetch(pHouse->Type))
		{
			if (pHouseTypeExt->EVATag >= 0)
				fallbackIndex = pHouseTypeExt->EVATag;
		}

		if (fallbackIndex < 0)
		{
			if (const auto pSide = SideClass::Array.GetItemOrDefault(pHouse->SideIndex))
			{
				if (const auto pSideExt = SideExt::Fetch(pSide))
				{
					if (pSideExt->EVATag >= 0)
						fallbackIndex = pSideExt->EVATag;
				}
			}
		}

		if (fallbackIndex < 0)
		{
			if (pHouse->SideIndex == 1)
				fallbackIndex = 1; // Russian
			else if (pHouse->SideIndex == 2)
				fallbackIndex = 2; // Yuri
			else
				fallbackIndex = 0; // Allied / Default
		}

		VoxClass::EVAIndex = fallbackIndex;
	}
}

// =============================
// load / save

template <typename T>
void SideExt::Serialize(T& Stm)
{
	Stm
		.Process(this->ArrayIndex)
		.Process(this->Sidebar_GDIPositions)
		.Process(this->Sidebar_HarvesterCounter_Offset)
		.Process(this->Sidebar_HarvesterCounter_HideMaxValue)
		.Process(this->Sidebar_HarvesterCounter_OnlyMaxValue)
		.Process(this->Sidebar_HarvesterCounter_ColorGreen)
		.Process(this->Sidebar_HarvesterCounter_ColorYellow)
		.Process(this->Sidebar_HarvesterCounter_ColorRed)
		.Process(this->Sidebar_WeedsCounter_Offset)
		.Process(this->Sidebar_WeedsCounter_Color)
		.Process(this->Sidebar_ProducingProgress_Offset)
		.Process(this->Sidebar_PowerDelta_Offset)
		.Process(this->Sidebar_PowerDelta_ColorGreen)
		.Process(this->Sidebar_PowerDelta_ColorYellow)
		.Process(this->Sidebar_PowerDelta_ColorRed)
		.Process(this->Sidebar_PowerDelta_ColorGrey)
		.Process(this->Sidebar_PowerDelta_Align)
		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->IngameScore_WinTheme)
		.Process(this->IngameScore_LoseTheme)
		.Process(this->BriefingTheme)
		.Process(this->MessageTextColor)
		.Process(this->SuperWeaponSidebar_OnPCX)
		.Process(this->SuperWeaponSidebar_OffPCX)
		.Process(this->SuperWeaponSidebar_TopPCX)
		.Process(this->SuperWeaponSidebar_CenterPCX)
		.Process(this->SuperWeaponSidebar_BottomPCX)
		.Process(this->EVATag)
		;
}

void SideExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SideExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool SideExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool SideExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

SideExt::ExtContainer::ExtContainer() : Container("SideClass") { }
SideExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6A4609, SideClass_CTOR, 0x7)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6A499F, SideClass_SDDTOR, 0x6)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK(0x679A10, SideClass_LoadAllFromINI, 0x5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	for (auto const pSide : SideClass::Array)
		SideExt::Fetch(pSide)->LoadFromINI(pINI);

	return 0;
}

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
*/

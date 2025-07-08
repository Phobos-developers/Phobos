#include "Body.h"

#include <ThemeClass.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

SideExt::ExtContainer SideExt::ExtMap;

void SideExt::ExtData::Initialize()
{
	const char* pID = this->OwnerObject()->ID;

	this->ArrayIndex = SideClass::FindIndex(pID);
	this->Sidebar_GDIPositions = this->ArrayIndex == 0; // true = Allied
};

void SideExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
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
	this->Sidebar_HarvesterCounter_Yellow.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorYellow");
	this->Sidebar_HarvesterCounter_Red.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorRed");
	this->Sidebar_WeedsCounter_Offset.Read(exINI, pSection, "Sidebar.WeedsCounter.Offset");
	this->Sidebar_WeedsCounter_Color.Read(exINI, pSection, "Sidebar.WeedsCounter.Color");
	this->Sidebar_ProducingProgress_Offset.Read(exINI, pSection, "Sidebar.ProducingProgress.Offset");
	this->Sidebar_PowerDelta_Offset.Read(exINI, pSection, "Sidebar.PowerDelta.Offset");
	this->Sidebar_PowerDelta_Green.Read(exINI, pSection, "Sidebar.PowerDelta.ColorGreen");
	this->Sidebar_PowerDelta_Yellow.Read(exINI, pSection, "Sidebar.PowerDelta.ColorYellow");
	this->Sidebar_PowerDelta_Red.Read(exINI, pSection, "Sidebar.PowerDelta.ColorRed");
	this->Sidebar_PowerDelta_Grey.Read(exINI, pSection, "Sidebar.PowerDelta.ColorGrey");
	this->Sidebar_PowerDelta_Align.Read(exINI, pSection, "Sidebar.PowerDelta.Align");
	this->ToolTip_Background_Color.Read(exINI, pSection, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, pSection, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, pSection, "ToolTip.Background.BlurSize");
	this->BriefingTheme = pINI->ReadTheme(pSection, "BriefingTheme", this->BriefingTheme);
	this->SuperWeaponSidebar_OnPCX.Read(pINI, pSection, "SuperWeaponSidebar.OnPCX");
	this->SuperWeaponSidebar_OffPCX.Read(pINI, pSection, "SuperWeaponSidebar.OffPCX");
	this->SuperWeaponSidebar_TopPCX.Read(pINI, pSection, "SuperWeaponSidebar.TopPCX");
	this->SuperWeaponSidebar_CenterPCX.Read(pINI, pSection, "SuperWeaponSidebar.CenterPCX");
	this->SuperWeaponSidebar_BottomPCX.Read(pINI, pSection, "SuperWeaponSidebar.BottomPCX");

	pINI->ReadString(pSection, "EVA.Tag", "", Phobos::readBuffer);

	if (std::strlen(Phobos::readBuffer) > 0)
		this->EVA_Tag = _strdup(Phobos::readBuffer);
}

void SideExt::UpdateMainEvaVoice(BuildingClass* pThis)
{
	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->NewEvaVoice_Index.isset())
		return;

	const auto pHouse = pThis->Owner;

	if (!pHouse->IsControlledByCurrentPlayer())
		return;

	int newPriority = -1;
	int newEvaIndex = VoxClass::EVAIndex;

	for (const auto pBuilding : pHouse->Buildings)
	{
		const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		// Special case that must be avoided here because can be the current EVA changer
		if (!pBuildingTypeExt->NewEvaVoice_Index.isset() || pBuilding->CurrentMission == Mission::Selling)
			continue;

		// The first highest priority takes precedence over lower ones
		if (pBuildingTypeExt->NewEvaVoice_Priority > newPriority)
		{
			newPriority = pBuildingTypeExt->NewEvaVoice_Priority;
			newEvaIndex = pBuildingTypeExt->NewEvaVoice_Index;
		}
	}

	if (pThis->CurrentMission != Mission::Selling && pTypeExt->NewEvaVoice_Priority > newPriority)
	{
		newPriority = pTypeExt->NewEvaVoice_Priority;
		newEvaIndex = pTypeExt->NewEvaVoice_Index;
	}

	if (newPriority > 0 && VoxClass::EVAIndex != newEvaIndex)
	{
		VoxClass::EVAIndex = newEvaIndex;

		// Greeting of the new EVA voice
		int idxPlay = pTypeExt->NewEvaVoice_InitialMessage.Get(-1);

		if (idxPlay != -1)
			VoxClass::PlayIndex(idxPlay);
	}
	else if (newPriority < 0)
	{
		// Restore the original EVA voice of the owner's side
		const auto pSide = SideClass::Array.GetItem(pHouse->SideIndex);
		const auto pSideExt = SideExt::ExtMap.Find(pSide);
		newEvaIndex = 0; // By default is set the "Allies" EVA voice

		if (pSideExt->EVA_Tag.isset())
		{
			const auto evaTag = pSideExt->EVA_Tag.Get();

			for (size_t i = 0; i < RulesExt::Global()->EVAIndexList.size(); i++)
			{
				const auto item = RulesExt::Global()->EVAIndexList[i].c_str();

				if (_strcmpi(item, evaTag) == 0)
				{
					newEvaIndex = i;
					break;
				}
			}
		}
		else
		{
			if (pHouse->SideIndex == 0)
				newEvaIndex = 0; // Allied
			if (pHouse->SideIndex == 1)
				newEvaIndex = 1; // Russian
			else if (pHouse->SideIndex == 2)
				newEvaIndex = 2; // Yuri
		}

		VoxClass::EVAIndex = newEvaIndex;
	}
}

// =============================
// load / save

template <typename T>
void SideExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->ArrayIndex)
		.Process(this->Sidebar_GDIPositions)
		.Process(this->Sidebar_HarvesterCounter_Offset)
		.Process(this->Sidebar_HarvesterCounter_Yellow)
		.Process(this->Sidebar_HarvesterCounter_Red)
		.Process(this->Sidebar_WeedsCounter_Offset)
		.Process(this->Sidebar_WeedsCounter_Color)
		.Process(this->Sidebar_ProducingProgress_Offset)
		.Process(this->Sidebar_PowerDelta_Offset)
		.Process(this->Sidebar_PowerDelta_Green)
		.Process(this->Sidebar_PowerDelta_Yellow)
		.Process(this->Sidebar_PowerDelta_Red)
		.Process(this->Sidebar_PowerDelta_Grey)
		.Process(this->Sidebar_PowerDelta_Align)
		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->IngameScore_WinTheme)
		.Process(this->IngameScore_LoseTheme)
		.Process(this->BriefingTheme)
		.Process(this->SuperWeaponSidebar_OnPCX)
		.Process(this->SuperWeaponSidebar_OffPCX)
		.Process(this->SuperWeaponSidebar_TopPCX)
		.Process(this->SuperWeaponSidebar_CenterPCX)
		.Process(this->SuperWeaponSidebar_BottomPCX)
		.Process(this->EVA_Tag)
		;
}

void SideExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<SideClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SideExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<SideClass>::SaveToStream(Stm);
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

DEFINE_HOOK_AGAIN(0x6A48A0, SideClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6A4780, SideClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(SideClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SideExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6A488B, SideClass_Load_Suffix, 0x6)
{
	SideExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6A48FC, SideClass_Save_Suffix, 0x5)
{
	SideExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x679A10, SideClass_LoadAllFromINI, 0x5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	for (auto const pSide : SideClass::Array)
		SideExt::ExtMap.Find(pSide)->LoadFromINI(pINI);

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

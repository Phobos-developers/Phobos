#include "Body.h"

#include <SessionClass.h>
#include <VeinholeMonsterClass.h>

std::unique_ptr<ScenarioExt::ExtData> ScenarioExt::Data = nullptr;

bool ScenarioExt::CellParsed = false;

void ScenarioExt::ExtData::SetVariableToByID(bool bIsGlobal, int nIndex, char bState)
{
	auto& dict = Global()->Variables[bIsGlobal];

	auto itr = dict.find(nIndex);

	if (itr != dict.end() && itr->second.Value != bState)
	{
		itr->second.Value = bState;
		ScenarioClass::Instance->VariablesChanged = true;
		if (!bIsGlobal)
			TagClass::NotifyLocalChanged(nIndex);
		else
			TagClass::NotifyGlobalChanged(nIndex);
	}
}

void ScenarioExt::ExtData::GetVariableStateByID(bool bIsGlobal, int nIndex, char* pOut)
{
	auto& dict = Global()->Variables[bIsGlobal];

	auto itr = dict.find(nIndex);
	if (itr != dict.end())
		*pOut = static_cast<char>(itr->second.Value);
}

void ScenarioExt::ExtData::ReadVariables(bool bIsGlobal, CCINIClass* pINI)
{
	if (!bIsGlobal) // Local variables need to be read again
		Global()->Variables[false].clear();
	else if (Global()->Variables[true].size() != 0) // Global variables had been loaded, DO NOT CHANGE THEM
		return;

	const int nCount = pINI->GetKeyCount("VariableNames");
	for (int i = 0; i < nCount; ++i)
	{
		const auto pKey = pINI->GetKeyName("VariableNames", i);
		int nIndex;
		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = Global()->Variables[bIsGlobal][nIndex];
			pINI->ReadString("VariableNames", pKey, pKey, Phobos::readBuffer);
			char* buffer;
			strcpy_s(var.Name, strtok_s(Phobos::readBuffer, ",", &buffer));
			if (auto pState = strtok_s(nullptr, ",", &buffer))
				var.Value = atoi(pState);
			else
				var.Value = 0;
		}
	}
}

// you've inspired something controversial
void ScenarioExt::ExtData::SaveVariablesToFile(bool isGlobal)
{
	CCINIClass fINI {};
	CCFileClass file { isGlobal ? "globals.ini" : "locals.ini" };

	if (file.Exists())
		fINI.ReadCCFile(&file);
	else
		file.CreateFileA();

	for (const auto& [_,varext] : Global()->Variables[isGlobal])
		fINI.WriteInteger(ScenarioClass::Instance->FileName, varext.Name, varext.Value, false);

	fINI.WriteCCFile(&file);
	file.Close();
}

void ScenarioExt::Allocate(ScenarioClass* pThis)
{
	Data = std::make_unique<ScenarioExt::ExtData>(pThis);
}

void ScenarioExt::Remove(ScenarioClass* pThis)
{
	Data = nullptr;
}

void ScenarioExt::LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINI(pINI);
}

void ScenarioExt::ExtData::UpdateAutoDeathObjectsInLimbo()
{
	for (auto const pExt : this->AutoDeathObjects)
	{
		auto const pTechno = pExt->OwnerObject();

		if (!pTechno->IsInLogic && pTechno->IsAlive)
			pExt->CheckDeathConditions(true);
	}
}

void ScenarioExt::ExtData::UpdateTransportReloaders()
{
	for (auto const pExt : this->TransportReloaders)
	{
		auto const pTechno = pExt->OwnerObject();

		if (pTechno->IsAlive && pTechno->Transporter && pTechno->Transporter->IsInLogic)
			pTechno->Reload();
	}
}

// =============================
// load / save

void ScenarioExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();

	INI_EX maINI(pINI);
	INI_EX ruINI(CCINIClass::INI_Rules);

	if (SessionClass::IsCampaign())
	{
		Nullable<bool> SP_MCVRedeploy;
		SP_MCVRedeploy.Read(maINI, GameStrings::Basic, GameStrings::MCVRedeploys);
		if (!SP_MCVRedeploy.isset())
			SP_MCVRedeploy.Read(ruINI, GameStrings::Basic, GameStrings::MCVRedeploys);
		GameModeOptionsClass::Instance.MCVRedeploy = SP_MCVRedeploy.Get(false);

		CCINIClass ini_missionmd {};
		ini_missionmd.LoadFromFile(GameStrings::MISSIONMD_INI);
		auto const scenarioName = pThis->FileName;

		// Override rankings
		pThis->ParTimeEasy = ini_missionmd.ReadTime(scenarioName, "Ranking.ParTimeEasy", pThis->ParTimeEasy);
		pThis->ParTimeMedium = ini_missionmd.ReadTime(scenarioName, "Ranking.ParTimeMedium", pThis->ParTimeMedium);
		pThis->ParTimeDifficult = ini_missionmd.ReadTime(scenarioName, "Ranking.ParTimeHard", pThis->ParTimeDifficult);
		ini_missionmd.ReadString(scenarioName, "Ranking.UnderParTitle", pThis->UnderParTitle, pThis->UnderParTitle);
		ini_missionmd.ReadString(scenarioName, "Ranking.UnderParMessage", pThis->UnderParMessage, pThis->UnderParMessage);
		ini_missionmd.ReadString(scenarioName, "Ranking.OverParTitle", pThis->OverParTitle, pThis->OverParTitle);
		ini_missionmd.ReadString(scenarioName, "Ranking.OverParMessage", pThis->OverParMessage, pThis->OverParMessage);

		this->ShowBriefing = pINI->ReadBool(GameStrings::Basic, "ShowBriefing", this->ShowBriefing);
		this->BriefingTheme = pINI->ReadTheme(GameStrings::Basic, "BriefingTheme", this->BriefingTheme);
	}

	// Dropship loadout stuff
	this->DropshipLoadout_Theme = pINI->ReadTheme(GameStrings::Basic, "DropshipLoadout.Theme", this->DropshipLoadout_Theme);
	this->DropshipLoadout_Money = pINI->ReadInteger(GameStrings::Basic, "DropshipLoadout.Money", this->DropshipLoadout_Money);
	this->DropshipLoadout_StartEVA = pINI->ReadVoxName(GameStrings::Basic, "DropshipLoadout.StartEVA", this->DropshipLoadout_StartEVA);
	this->DropshipLoadout_AddUnusedMoneyToPlayer = pINI->ReadBool(GameStrings::Basic, "DropshipLoadout.AddUnusedMoneyToPlayer", this->DropshipLoadout_AddUnusedMoneyToPlayer);

	// Custom Dropship loadout images, in SHP format
	char* context = nullptr;

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.Palette", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_Palette = FileSystem::LoadPALFile(Phobos::readBuffer, DSurface::Hidden);

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.Background", "", Phobos::readBuffer) != 0)
	{
		char filename[260];
		_snprintf_s(filename, sizeof(filename), Phobos::readBuffer, ScenarioClass::Instance->StartingDropships);
		this->DropshipLoadout_Background = FileSystem::LoadSHPFile(_strdup(filename));
	}

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.UpArrow", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_UpArrow = FileSystem::LoadSHPFile(Phobos::readBuffer);

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.Down", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_DownArrow = FileSystem::LoadSHPFile(Phobos::readBuffer);

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.Loadout", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_Loadout = FileSystem::LoadSHPFile(Phobos::readBuffer);

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.PilotLit", "", Phobos::readBuffer) != 0)
		this->DropshipLoadout_PilotLit = FileSystem::LoadSHPFile(Phobos::readBuffer);

	// Sidebar click animations list (the animation that appears in the sidebar when a cameo is clicked)
	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.DGreenList", "", Phobos::readBuffer);
	
	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		this->DropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile(cur));
	}

	// Custom Dropship loadout images, in PCX format
	context = nullptr;

	if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.BackgroundPCX", "", Phobos::readBuffer) != 0)
	{
		char filename[260];
		_snprintf_s(filename, sizeof(filename), Phobos::readBuffer, ScenarioClass::Instance->StartingDropships);
		this->DropshipLoadout_BackgroundPCX = PhobosPCXFile(_strdup(filename));
	}

	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.UpArrowPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_UpArrowPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.DownArrowPCX", "", Phobos::readBuffer);
	this->DropshipLoadout_DownArrowPCX = PhobosPCXFile(Phobos::readBuffer);

	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.LoadoutPCX", "", Phobos::readBuffer);
	auto pAnimationLoadoutVector = GeneralUtils::GetAnimationPCX(Phobos::readBuffer);

	if (pAnimationLoadoutVector)
		this->DropshipLoadout_LoadoutPCX = std::move(*pAnimationLoadoutVector);

	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.PilotLitPCX", "", Phobos::readBuffer);
	auto pAnimationPilotLitVector = GeneralUtils::GetAnimationPCX(Phobos::readBuffer);

	if (pAnimationPilotLitVector)
		this->DropshipLoadout_PilotLitPCX = std::move(*pAnimationPilotLitVector);

	// Sidebar click animations list (the animation that appears in the sidebar when a cameo is clicked)
	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.DGreenListPCX", "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		this->DropshipLoadout_DGreenListPCX.emplace_back(GeneralUtils::GetAnimationPCX(cur));
	}

	this->DropshipLoadout_DGreenAnimationsCount = pINI->ReadInteger(GameStrings::Basic, "DropshipLoadout.DGreenAnimationsCount", 0);

	for (int i = 0; i < this->DropshipLoadout_DGreenAnimationsCount; i++)
	{
		char tempBuffer[256];
		Point2D location = Point2D::Empty;

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.DGreenLocation%d", i);
		pINI->ReadPoint2D(location, GameStrings::Basic, tempBuffer, location);
		this->DropshipLoadout_DGreenLocations.push_back(location);
	}

	// List of Dropship transports used in the map action
	pINI->ReadString(GameStrings::Basic, "DropshipLoadout.Carriers", "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		TechnoTypeClass* buffer;

		if (Parser<TechnoTypeClass*>::TryParse(cur, &buffer))
			this->DropshipLoadout_Carriers.emplace_back(buffer);
		else
			Debug::Log("[Developer warning] DropshipLoadout.Carriers (Elements: %d): Error parsing [%s] -> Skipped\n", this->DropshipLoadout_Carriers.size(), cur);
	}

	// Custom Dropship Loadout coordinates
	Point2D defaultEmptyLocation = { Point2D::Empty };
	pINI->ReadPoint2D(DropshipLoadout_LoadoutLocation, GameStrings::Basic, "DropshipLoadout.LoadoutLocation", defaultEmptyLocation);
	pINI->ReadPoint2D(DropshipLoadout_PilotLitLocation, GameStrings::Basic, "DropshipLoadout.PilotLitLocation", defaultEmptyLocation);
	pINI->ReadPoint2D(DropshipLoadout_UpArrowLocation, GameStrings::Basic, "DropshipLoadout.UpArrowLocation", defaultEmptyLocation);
	pINI->ReadPoint2D(DropshipLoadout_DownArrowLocation, GameStrings::Basic, "DropshipLoadout.DownArrowLocation", defaultEmptyLocation);

	this->DropshipLoadout_SidebarCameosCount = pINI->ReadInteger(GameStrings::Basic, "DropshipLoadout.SidebarCameosCount", 0);

	for (int i = 0; i < this->DropshipLoadout_SidebarCameosCount; i++)
	{
		char tempBuffer[256];
		Point2D location = Point2D::Empty;

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.SidebarCameoLocation%d", i);
		pINI->ReadPoint2D(location, GameStrings::Basic, tempBuffer, location);
		this->DropshipLoadout_SidebarCameoLocations.push_back(location);
	}

	this->DropshipLoadout_DropshipCameosCount = pINI->ReadInteger(GameStrings::Basic, "DropshipLoadout.DropshipCameosCount", 0);

	for (int i = 0; i < ScenarioClass::Instance->StartingDropships; i++)
	{
		std::vector<Point2D> locations;

		for (int j = 0; j < this->DropshipLoadout_DropshipCameosCount; j++)
		{
			char tempBuffer[256];
			Point2D location = Point2D::Empty;

			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DropshipLoadout.Dropship%d.CameoLocation%d", i, j);
			pINI->ReadPoint2D(location, GameStrings::Basic, tempBuffer, location);
			locations.push_back(location);
		}

		this->DropshipLoadout_DropshipCameoLocations.push_back(locations);

		pINI->ReadString(GameStrings::Basic, "DropshipLoadout.Carriers", "", Phobos::readBuffer);

		if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.BuyClickSound", "", Phobos::readBuffer) != 0)
			this->DropshipLoadout_BuyClickSound = VocClass::FindIndex(Phobos::readBuffer);
		//else
			//this->DropshipLoadout_SellClickSound = RulesClass::Instance->GUITabSound;

		//this->DropshipLoadout_SellClickSound.
		if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.SellClickSound", "", Phobos::readBuffer) != 0)
			this->DropshipLoadout_SellClickSound = VocClass::FindIndex(Phobos::readBuffer);
		//else
			//this->DropshipLoadout_SellClickSound = RulesClass::Instance->GUITabSound;

		if (pINI->ReadString(GameStrings::Basic, "DropshipLoadout.ArrowsClickSound", "", Phobos::readBuffer) != 0)
			this->DropshipLoadout_ArrowsClickSound = VocClass::FindIndex(Phobos::readBuffer);
	}
}

template <typename T>
void ScenarioExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Waypoints)
		.Process(this->Variables[0])
		.Process(this->Variables[1])
		.Process(this->ShowBriefing)
		.Process(this->BriefingTheme)
		.Process(this->AutoDeathObjects)
		.Process(this->TransportReloaders)
		.Process(this->SWSidebar_Enable)
		.Process(this->SWSidebar_Indices)
		.Process(this->DefaultLS640BkgdName)
		.Process(this->DefaultLS800BkgdName)
		.Process(this->DefaultLS800BkgdPal)
		.Process(this->MasterDetonationBullet)
		.Process(this->DropshipLoadout_Theme)
		.Process(this->DropshipLoadout_Money)
		.Process(this->DropshipLoadout_StartEVA)
		.Process(this->DropshipLoadout_Carriers)
		.Process(this->DropshipLoadout_AddUnusedMoneyToPlayer)
		.Process(this->DropshipLoadout_Palette)
		.Process(this->DropshipLoadout_Background)
		.Process(this->DropshipLoadout_UpArrow)
		.Process(this->DropshipLoadout_DownArrow)
		.Process(this->DropshipLoadout_Loadout)
		.Process(this->DropshipLoadout_LoadoutLocation)
		.Process(this->DropshipLoadout_PilotLit)
		.Process(this->DropshipLoadout_PilotLitLocation)
		.Process(this->DropshipLoadout_DGreenList)
		.Process(this->DropshipLoadout_BackgroundPCX)
		.Process(this->DropshipLoadout_UpArrowPCX)
		.Process(this->DropshipLoadout_DownArrowPCX)
		.Process(this->DropshipLoadout_LoadoutPCX)
		.Process(this->DropshipLoadout_PilotLitPCX)
		.Process(this->DropshipLoadout_DGreenListPCX)
		.Process(this->DropshipLoadout_DGreenAnimationsCount)
		.Process(this->DropshipLoadout_DGreenLocations)
		.Process(this->DropshipLoadout_UpArrowLocation)
		.Process(this->DropshipLoadout_DownArrowLocation)
		.Process(this->DropshipLoadout_SidebarCameosCount)
		.Process(this->DropshipLoadout_SidebarCameoLocations)
		.Process(this->DropshipLoadout_DropshipCameosCount)
		.Process(this->DropshipLoadout_DropshipCameoLocations)
		.Process(this->DropshipLoadout_BuyClickSound)
		.Process(this->DropshipLoadout_SellClickSound)
		.Process(this->DropshipLoadout_ArrowsClickSound)
//		.Process(this->NewMessageList); // Should not S/L
		;
}

void ScenarioExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ScenarioClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ScenarioExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ScenarioClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container hooks

DEFINE_HOOK(0x683549, ScenarioClass_CTOR, 0x9)
{
	GET(ScenarioClass*, pItem, EAX);

	ScenarioExt::Allocate(pItem);

	ScenarioExt::Global()->Waypoints.clear();
	ScenarioExt::Global()->Variables[0].clear();
	ScenarioExt::Global()->Variables[1].clear();

	return 0;
}

DEFINE_HOOK(0x6BEB7D, ScenarioClass_DTOR, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);

	ScenarioExt::Remove(pItem);
	return 0;
}

IStream* ScenarioExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(0x689470, ScenarioClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x689310, ScenarioClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	ScenarioExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix, 0x6)
{
	auto buffer = ScenarioExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(ScenarioExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(ScenarioExt::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x68945B, ScenarioClass_Save_Suffix, 0x8)
{
	auto buffer = ScenarioExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(ScenarioExt::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(ScenarioExt::g_pStm);

	return 0;
}

DEFINE_HOOK(0x68AD2F, ScenarioClass_LoadFromINI, 0x5)
{
	GET(ScenarioClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EDI);

	ScenarioExt::LoadFromINIFile(pItem, pINI);
	return 0;
}

DEFINE_HOOK(0x55B4E1, LogicClass_Update_BeforeAll, 0x5)
{
	VeinholeMonsterClass::UpdateAllVeinholes();

	ScenarioExt::Global()->UpdateAutoDeathObjectsInLimbo();
	ScenarioExt::Global()->UpdateTransportReloaders();

	return 0;
}

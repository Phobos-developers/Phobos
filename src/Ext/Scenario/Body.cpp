#include "Body.h"

#include <SessionClass.h>
#include <GameStrings.h>

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

	int nCount = pINI->GetKeyCount("VariableNames");
	for (int i = 0; i < nCount; ++i)
	{
		auto pKey = pINI->GetKeyName("VariableNames", i);
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

void ScenarioExt::ExtData::SaveVariablesToFile(bool isGlobal)
{
	const auto fileName = isGlobal ? "globals.ini" : "locals.ini";
	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();

	const auto& variables = Global()->Variables[isGlobal];
	for (const auto& variable : variables)
		pINI->WriteInteger(ScenarioClass::Instance()->FileName, variable.second.Name, variable.second.Value, false);

	pINI->WriteCCFile(pFile);
	pFile->Close();
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

// =============================
// load / save

void ScenarioExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();

	INI_EX exINI(pINI);

	if (SessionClass::IsCampaign())
	{
		Nullable<bool> SP_MCVRedeploy;
		SP_MCVRedeploy.Read(exINI, GameStrings::Basic, GameStrings::MCVRedeploys);
		GameModeOptionsClass::Instance->MCVRedeploy = SP_MCVRedeploy.Get(false);

		CCINIClass* pINI_MISSIONMD = CCINIClass::LoadINIFile(GameStrings::MISSIONMD_INI);
		auto const scenarioName = pThis->FileName;

	    // Override rankings
		pThis->ParTimeEasy = pINI_MISSIONMD->ReadTime(scenarioName, "Ranking.ParTimeEasy", pThis->ParTimeEasy);
		pThis->ParTimeMedium = pINI_MISSIONMD->ReadTime(scenarioName, "Ranking.ParTimeMedium", pThis->ParTimeMedium);
		pThis->ParTimeDifficult = pINI_MISSIONMD->ReadTime(scenarioName, "Ranking.ParTimeHard", pThis->ParTimeDifficult);
		pINI_MISSIONMD->ReadString(scenarioName, "Ranking.UnderParTitle", pThis->UnderParTitle, pThis->UnderParTitle);
		pINI_MISSIONMD->ReadString(scenarioName, "Ranking.UnderParMessage", pThis->UnderParMessage, pThis->UnderParMessage);
		pINI_MISSIONMD->ReadString(scenarioName, "Ranking.OverParTitle", pThis->OverParTitle, pThis->OverParTitle);
		pINI_MISSIONMD->ReadString(scenarioName, "Ranking.OverParMessage", pThis->OverParMessage, pThis->OverParMessage);

		this->ShowBriefing = pINI_MISSIONMD->ReadBool(scenarioName, "ShowBriefing", pINI->ReadBool(GameStrings::Basic,"ShowBriefing", this->ShowBriefing));

		CCINIClass::UnloadINIFile(pINI_MISSIONMD);
	}
}

template <typename T>
void ScenarioExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Waypoints)
		.Process(this->Variables[0])
		.Process(this->Variables[1])
		.Process(SessionClass::Instance->Config)
		.Process(ShowBriefing)
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

bool ScenarioExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool ScenarioExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
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

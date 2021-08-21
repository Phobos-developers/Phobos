#include "Body.h"

template<> const DWORD Extension<ScenarioClass>::Canary = 0xABCD1595;
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
		*pOut = itr->second.Value;
}

void ScenarioExt::ExtData::ReadVariables(bool bIsGlobal, CCINIClass* pINI)
{
	int nCount = pINI->GetKeyCount("VariableNames");
	for (int i = 0; i < nCount; ++i)
	{
		auto pKey = pINI->GetKeyName("VariableNames", i);
		int nIndex;
		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = Global()->Variables[bIsGlobal][nIndex];
			pINI->ReadString("VariableNames", pKey, pKey, Phobos::readBuffer);
			strcpy(var.Name, strtok(Phobos::readBuffer, ","));
			if (auto pState = strtok(nullptr, ","))
				var.Value = atoi(pState) != 0;
			else
				var.Value = 0;
		}
	}
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
	// auto pThis = this->OwnerObject();

	// INI_EX exINI(pINI);

	

}

template <typename T>
void ScenarioExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Waypoints)
		.Process(this->Variables[0])
		.Process(this->Variables[1])
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

		if (Reader.Expect(ScenarioExt::ExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x68945B, ScenarioClass_Save_Suffix, 0x8)
{
	auto buffer = ScenarioExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(ScenarioExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(ScenarioExt::g_pStm);

	return 0;
}

DEFINE_HOOK(0x68AD62, ScenarioClass_LoadFromINI, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x38, -0x8));

	ScenarioExt::LoadFromINIFile(pItem, pINI);
	return 0;
}

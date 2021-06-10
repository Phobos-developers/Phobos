#include "Body.h"

template<> const DWORD Extension<ScenarioClass>::Canary = 0xABCD1595;
std::unique_ptr<ScenarioExt::ExtData> ScenarioExt::Data = nullptr;

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

	for (int i = 0; i < pINI->GetKeyCount("Waypoints"); ++i)
	{
		const auto pName = pINI->GetKeyName("Waypoints", i);
		int nCoord = pINI->ReadInteger("Waypoints", pName, -1);
		if (nCoord == -1)
			Debug::FatalErrorAndExit("[Fatal Error] Invalid waypoint read! Key name = %s", pName);
		Global()->Waypoints.push_back(nCoord);
	}

}

template <typename T>
void ScenarioExt::ExtData::Serialize(T& Stm)
{
	Stm
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

DEFINE_HOOK(683549, ScenarioClass_CTOR, 9)
{
	GET(ScenarioClass*, pItem, EAX);

	ScenarioExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(6BEB7D, ScenarioClass_DTOR, 6)
{
	GET(ScenarioClass*, pItem, ESI);

	ScenarioExt::Remove(pItem);
	return 0;
}

IStream* ScenarioExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(689470, ScenarioClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(689310, ScenarioClass_SaveLoad_Prefix, 5)
{
	GET_STACK(IStream*, pStm, 0x4);

	ScenarioExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(689669, ScenarioClass_Load_Suffix, 6)
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

DEFINE_HOOK(68945B, ScenarioClass_Save_Suffix, 8)
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

DEFINE_HOOK(68AD62, ScenarioClass_LoadFromINI, 6)
{
	GET(ScenarioClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x38, -0x8));

	ScenarioExt::LoadFromINIFile(pItem, pINI);
	return 0;
}

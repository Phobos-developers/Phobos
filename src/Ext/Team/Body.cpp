#include "Body.h"

template<> const DWORD Extension<TeamClass>::Canary = 0x414B4B41;
TeamExt::ExtContainer TeamExt::ExtMap;

// =============================
// load / save

template <typename T>
void TeamExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->WaitNoTargetAttempts)
		.Process(this->NextSuccessWeightAward)
		;
}

void TeamExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TeamClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TeamClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TeamExt::ExtContainer::ExtContainer() : Container("TeamClass") { }
TeamExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//Everything InitEd beside the Vector below this address
DEFINE_HOOK(6E8B46, TeamClass_CTOR, 7)
{
	GET(TeamClass*, pThis, ESI);
	
	TeamExt::ExtMap.FindOrAllocate(pThis);

	return 0;
}

//before `test` i hope not crash the game ,..
DEFINE_HOOK(6E8EC6, TeamClass_DTOR, 9)
{
	GET(TeamClass*, pThis, ESI);
	
	TeamExt::ExtMap.Remove(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(6EC450, TeamClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(6EC540, TeamClass_SaveLoad_Prefix, 8)
{
	GET_STACK(TeamClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	
	TeamExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(6EC52C, TeamClass_Load_Suffix, 4)
{
	TeamExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(6EC55A, TeamClass_Save_Suffix, 5)
{
	TeamExt::ExtMap.SaveStatic();
	return 0;
}


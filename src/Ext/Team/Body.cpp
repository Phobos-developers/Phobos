#include "Body.h"

TeamExt::ExtContainer TeamExt::ExtMap;

// =============================
// load / save

template <typename T>
void TeamExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->WaitNoTargetAttempts)
		.Process(this->NextSuccessWeightAward)
		.Process(this->IdxSelectedObjectFromAIList)
		.Process(this->CloseEnough)
		.Process(this->Countdown_RegroupAtLeader)
		.Process(this->MoveMissionEndMode)
		.Process(this->WaitNoTargetCounter)
		.Process(this->WaitNoTargetTimer)
		.Process(this->ForceJump_Countdown)
		.Process(this->ForceJump_InitialCountdown)
		.Process(this->ForceJump_RepeatMode)
		.Process(this->TeamLeader)
		.Process(this->PreviousScriptList)
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

void TeamExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TeamLeader, ptr);
}

// =============================
// container

TeamExt::ExtContainer::ExtContainer() : Container("TeamClass") { }
TeamExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//Everything InitEd beside the Vector below this address
DEFINE_HOOK(0x6E8B46, TeamClass_CTOR, 0x7)
{
	GET(TeamClass*, pThis, ESI);

	TeamExt::ExtMap.TryAllocate(pThis);

	return 0;
}

//before `test` i hope not crash the game ,..
DEFINE_HOOK(0x6E8EC6, TeamClass_DTOR, 0x9)
{
	GET(TeamClass*, pThis, ESI);

	TeamExt::ExtMap.Remove(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6EC450, TeamClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6EC540, TeamClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TeamClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TeamExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6EC52F, TeamClass_Load_Suffix, 0x6)
{
	TeamExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6EC55A, TeamClass_Save_Suffix, 0x5)
{
	TeamExt::ExtMap.SaveStatic();
	return 0;
}

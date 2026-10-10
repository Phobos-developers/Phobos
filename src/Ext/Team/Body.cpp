#include "Body.h"

TeamExt::ExtContainer TeamExt::ExtMap;

// =============================
// load / save

template <typename T>
void TeamExt::Serialize(T& Stm)
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
		.Process(this->AngerNodeModifier)
		.Process(this->OnlyTargetHouseEnemy)
		.Process(this->OnlyTargetHouseEnemyMode)
		;
}

void TeamExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

void TeamExt::OnDetach(FootClass* pTarget, bool removed)
{
	if (removed)
		AnnounceInvalidPointer(this->TeamLeader, pTarget);
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


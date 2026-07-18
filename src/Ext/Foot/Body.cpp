#include "Body.h"

// =============================
// load / save

template <typename T>
void FootExt::Serialize(T& Stm)
{
	Stm
		.Process(this->LastKillWasTeamTarget)
		.Process(this->LastWarpDistance)
		.Process(this->JumpjetSpeed)
		.Process(this->IsInTunnel)
		.Process(this->OriginalPassengerOwner)
		.Process(this->HasRemainingWarpInDelay)
		.Process(this->LastWarpInDelay)
		.Process(this->IsBeingChronoSphered)
		.Process(this->LastSensorsMapCoords)
		.Process(this->TiberiumEater_Timer)
		.Process(this->ResetLocomotor)
		.Process(this->JumpjetStraightAscend)
		;
}

void FootExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void FootExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

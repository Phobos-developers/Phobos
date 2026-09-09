#include "Body.h"

AircraftTypeExt::ExtContainer AircraftTypeExt::ExtMap;

// =============================
// load / save

void AircraftTypeExt::Initialize()
{
	TechnoTypeExt::Initialize();

	this->Missile_TakeOffAnim = AnimTypeClass::Find("V3TAKOFF");
}

void AircraftTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	TechnoTypeExt::LoadFromINIFile(pINI);

	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->VoicePickup.Read(exINI, pSection, "VoicePickup");
	this->SpawnFromEdge.Read(exINI, pSection, "SpawnFromEdge");
	this->RetreatToEdge.Read(exINI, pSection, "RetreatToEdge");
	this->SpawnDistanceFromTarget.Read(exINI, pSection, "SpawnDistanceFromTarget");
	this->SpawnHeight.Read(exINI, pSection, "SpawnHeight");
	this->LandingDir.Read(exINI, pSection, "LandingDir");
	this->CurleyShuffle.Read(exINI, pSection, "CurleyShuffle");
	this->ExtendedAircraftMissions.Read(exINI, pSection, "ExtendedAircraftMissions");
	this->ExtendedAircraftMissions_SmoothMoving.Read(exINI, pSection, "ExtendedAircraftMissions.SmoothMoving");
	this->ExtendedAircraftMissions_EarlyDescend.Read(exINI, pSection, "ExtendedAircraftMissions.EarlyDescend");
	this->ExtendedAircraftMissions_RearApproach.Read(exINI, pSection, "ExtendedAircraftMissions.RearApproach");
	this->ExtendedAircraftMissions_FastScramble.Read(exINI, pSection, "ExtendedAircraftMissions.FastScramble");
	this->ExtendedAircraftMissions_UnlandDamage.Read(exINI, pSection, "ExtendedAircraftMissions.UnlandDamage");
	this->FiringForceScatter.Read(exINI, pSection, "FiringForceScatter");
	this->ParadropDelay.Read(exINI, pSection, "ParadropDelay");
	this->ParadropEndDelay.Read(exINI, pSection, "ParadropEndDelay");
	this->FlyNoWobbles.Read(exINI, pSection, "FlyNoWobbles");
	this->IsALoaner.Read(exINI, pSection, "IsALoaner");
	this->LandingAnim.Read(exINI, pSection, "LandingAnim");
	this->Missile_Cruise.Read(exINI, pSection, "Missile.Cruise");
	this->Missile_TakeOffSeparation.Read(exINI, pSection, "Missile.TakeOffSeparation");
	this->Missile_TakeOffAnim.Read(exINI, pSection, "Missile.TakeOffAnim");
}

template <typename T>
void AircraftTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->VoicePickup)
		.Process(this->SpawnFromEdge)
		.Process(this->RetreatToEdge)
		.Process(this->SpawnDistanceFromTarget)
		.Process(this->SpawnHeight)
		.Process(this->LandingDir)
		.Process(this->CurleyShuffle)
		.Process(this->ExtendedAircraftMissions)
		.Process(this->ExtendedAircraftMissions_SmoothMoving)
		.Process(this->ExtendedAircraftMissions_EarlyDescend)
		.Process(this->ExtendedAircraftMissions_RearApproach)
		.Process(this->ExtendedAircraftMissions_FastScramble)
		.Process(this->ExtendedAircraftMissions_UnlandDamage)
		.Process(this->FiringForceScatter)
		.Process(this->ParadropDelay)
		.Process(this->ParadropEndDelay)
		.Process(this->FlyNoWobbles)
		.Process(this->IsALoaner)
		.Process(this->LandingAnim)
		.Process(this->Missile_Cruise)
		.Process(this->Missile_TakeOffAnim)
		.Process(this->Missile_TakeOffSeparation)
		;
}

void AircraftTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AircraftTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

AircraftTypeExt::ExtContainer::ExtContainer() : Container("AircraftTypeClass") { }
AircraftTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x41C8C0, AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

// The extension chain is read at the end of each concrete type class's LoadFromINI,
// once every native field - inherited and own alike - has been parsed.
DEFINE_HOOK(0x41CD82, AircraftTypeClass_LoadFromINI, 0x7)
{
	GET(AircraftTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x98);

	if (auto const pExt = AircraftTypeExt::TryFetch(pItem))
		pExt->LoadFromINI(pINI);

	return 0;
}

// Hooked after the base destructor call in both destructor bodies; the second site
// is the tail of the standalone body (pop/pop/retn, safe to steal - the bytes after
// it are alignment padding that is never executed).
DEFINE_HOOK_AGAIN(0x41CA96, AircraftTypeClass_DTOR, 0x3)
DEFINE_HOOK(0x41D056, AircraftTypeClass_DTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Remove(pItem);

	return 0;
}

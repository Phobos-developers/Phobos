#include "Body.h"

#include <Ext/Cell/Body.h>
#include <Ext/InfantryType/Body.h>

InfantryExt::ExtContainer InfantryExt::ExtMap;

// Returns hardcoded prone/deployed FLH overrides for infantry, if set.
CoordStruct InfantryExt::GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	auto const pTypeExt = InfantryTypeExt::Fetch(pThis->Type);
	Nullable<CoordStruct> pickedFLH;

	if (pThis->IsDeployed())
	{
		if (weaponIndex == 0)
			pickedFLH = pTypeExt->DeployedPrimaryFireFLH;
		else if (weaponIndex == 1)
			pickedFLH = pTypeExt->DeployedSecondaryFireFLH;
	}
	else
	{
		if (pThis->Crawling)
		{
			if (weaponIndex == 0)
				pickedFLH = pTypeExt->PronePrimaryFireFLH;
			else if (weaponIndex == 1)
				pickedFLH = pTypeExt->ProneSecondaryFireFLH;
		}
	}

	if (pickedFLH.isset())
	{
		FLH = pickedFLH.Get();
		FLHFound = true;
	}

	return FLH;
}

void InfantryExt::MoveToCellCenter(InfantryClass* pThis, CoordStruct const& location)
{
	if (!pThis->Locomotor)
		return;

	const auto pCell = MapClass::Instance.TryGetCellAt(location);
	const auto center = pCell->GetCenterCoords();
	if (center == location)
		return;

	// Infantry is parked on a sub-position of its cell and the converted
	// non-infantry must sit on the cell centre instead, so the infantry has to
	// walk there first.
	//
	// MoveTo cannot do it. The walk locomotor's AI (0x75AEC0) asks the
	// pathfinder for a route first (FootClass::UpdatePathfinding, 0x4D3920) and
	// a destination inside the infantry's own cell yields none, so it falls
	// through to a distance test (0x75B046). A sub-position -> centre hop is
	// only ~90 leptons, well under the "already arrived" threshold at
	// Rules+0x1718, so the AI calls foot->vt[0x480](0, 1) and stops: the order
	// is silently dropped.
	//
	// ILocomotion::Force_Immediate_Destination is the generic route to that
	// routine, and it is a plain ILocomotion method rather than a
	// WalkLocomotionClass speciality: it sits at vtable slot 0x78 for every
	// locomotor (identified by the slot's stack-cleanup arity matching
	// `__stdcall (CoordStruct)`, i.e. RET 0x10). The other ten locomotors share
	// a no-op stub (0x55AC00, a bare RET 0x10), while the walk locomotor
	// overrides it at 0x75AE30 with a call to FUN_0075C240 - the same
	// "resolve this cell's sub-position and set HeadToCoord" routine the AI
	// reaches at 0x75BC13.
	//
	// FUN_0075C240 runs the coordinate through
	// CellClass::FindInfantrySubposition, which the hook in Ext/Cell/Hooks.cpp
	// makes return the centre while InfantryCenterAllowed is raised.
	//
	// Note that YRpp annotates this method with "Don't use this function."
	// (Interfaces.h). Its behaviour here is exactly what is needed and it does
	// not set IsMoving, relying on the AI picking HeadToCoord up on the next
	// frame, so it is worth in-game verification rather than assuming.
	//
	// The flag has to stay raised until the infantry actually arrives: while it
	// walks, the AI re-enters FUN_0075C240 every frame (0x75BE18, with the
	// Empty sentinel, so the routine re-resolves the foot's current position)
	// and would otherwise overwrite HeadToCoord with a corner again.
	auto const pTargetCellExt = CellExt::Fetch(pCell);

	pThis->PlayAnim(Sequence::Ready, true);

	pTargetCellExt->InfantryCenterAllowed++;
	pThis->Locomotor->Force_Immediate_Destination(center);
	pTargetCellExt->InfantryCenterAllowed--;
}

void InfantryExt::HandlePendingConvertToTypeAtCenter(InfantryClass* pThis)
{
	if (pThis->CurrentMission == Mission::Move)
		return;

	auto const pThisExt = InfantryExt::Fetch(pThis);
	if (!pThisExt->PendingConvertToTypeAtCenter)
		return;

	if (!TechnoExt::ConvertToType(pThis, pThisExt->PendingConvertToTypeAtCenter))
		return;

	pThisExt->PendingConvertToTypeAtCenter = nullptr;
}

// =============================
// load / save

template <typename T>
void InfantryExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SkipTargetChangeResetSequence)
		.Process(this->HasDeployConverted)
		.Process(this->HasUndeployConverted)
		;
}

void InfantryExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void InfantryExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

InfantryExt::ExtContainer::ExtContainer() : Container("InfantryClass") { }
InfantryExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x517A60, InfantryClass_CTOR, 0xE)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK(0x517F81, InfantryClass_DTOR, 0x8)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Remove(pItem);

	return 0;
}

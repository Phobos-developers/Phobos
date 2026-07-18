#include "Body.h"

UnitExt::ExtContainer UnitExt::ExtMap;

// =============================
// load / save

template <typename T>
void UnitExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SubterraneanHarvStatus)
		.Process(this->SubterraneanHarvRallyPoint)
		.Process(this->ReceiveDamage)
		.Process(this->DeployFireTimer)
		.Process(this->KeepTargetOnMove)
		.Process(this->SimpleDeployerAnimationTimer)
		;
}

// Subterranean harvester factory exit state machine.
void UnitExt::UpdateSubterraneanHarvester()
{
	auto const pThis = this->OwnerObject();

	// Unnecessary for AI players.
	if (!pThis->Owner->IsControlledByHuman())
		return;

	switch (this->SubterraneanHarvStatus)
	{
	case 0: // No state to handle.
		break;
	case 1: // Unit has been created.
		// If we're still in the factory do not advance.
		if (pThis->HasAnyLink())
			break;

		pThis->ClearNavigationList();

		// If we have rally point available, move to it and advance to next state, otherwise end here.
		if (this->SubterraneanHarvRallyPoint)
		{
			pThis->SetDestination(this->SubterraneanHarvRallyPoint, false);
			pThis->QueueMission(Mission::Move, true);
			this->SubterraneanHarvRallyPoint = nullptr;
			this->SubterraneanHarvStatus = 2;
			break;
		}
		else
		{
			this->SubterraneanHarvStatus = 0;
		}

		break;
	case 2: // Out of factory and on move.
		// If we're still moving don't start harvesting.
		if (pThis->Destination || pThis->CurrentMission == Mission::Move)
			break;

		// If harvester stops moving and becomes anything except idle, reset the state machine.
		if (pThis->CurrentMission != Mission::Guard)
		{
			this->SubterraneanHarvStatus = 0;
			break;
		}

		// Go harvest ore.
		pThis->ClearNavigationList();
		pThis->QueueMission(Mission::Harvest, true);
		this->SubterraneanHarvStatus = 0;
		break;
	default:
		break;
	}
}

// Resets target if KeepTargetOnMove unit moves beyond weapon range.
void UnitExt::UpdateKeepTargetOnMove()
{
	if (!this->KeepTargetOnMove)
		return;

	auto const pThis = this->OwnerObject();

	if (!pThis->Target)
	{
		this->KeepTargetOnMove = false;
		return;
	}

	const auto pTypeExt = this->TypeExtData;

	if (!pTypeExt->KeepTargetOnMove)
	{
		pThis->SetTarget(nullptr);
		this->KeepTargetOnMove = false;
		return;
	}

	if (pThis->CurrentMission == Mission::Guard)
	{
		if (!pTypeExt->KeepTargetOnMove_NoMorePursuit)
		{
			pThis->QueueMission(Mission::Attack, false);
			this->KeepTargetOnMove = false;
			return;
		}
	}
	else if (pThis->CurrentMission != Mission::Move)
	{
		return;
	}

	const int weaponIndex = pTypeExt->KeepTargetOnMove_Weapon >= 0 ? pTypeExt->KeepTargetOnMove_Weapon : pThis->SelectWeapon(pThis->Target);

	if (auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		const int extraDistance = static_cast<int>(pTypeExt->KeepTargetOnMove_ExtraDistance.Get());
		const int range = pWeapon->Range;
		pWeapon->Range += extraDistance; // Temporarily adjust weapon range based on the extra distance.

		if (!pThis->IsCloseEnough(pThis->Target, weaponIndex))
		{
			pThis->SetTarget(nullptr);
			this->KeepTargetOnMove = false;
		}

		pWeapon->Range = range;
	}
}

void UnitExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void UnitExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

UnitExt::ExtContainer::ExtContainer() : Container("UnitClass") { }
UnitExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7353D3, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x7359DA, UnitClass_DTOR, 0x9)
DEFINE_HOOK(0x735967, UnitClass_DTOR, 0x9)
{
	GET(UnitClass*, pItem, ESI);

	UnitExt::ExtMap.Remove(pItem);

	return 0;
}

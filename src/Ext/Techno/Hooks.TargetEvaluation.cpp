#include "Body.h"
#include <Interop/TechnoExt.h>

// Cursor & target acquisition stuff not directly tied to other features can go here.

#pragma region TargetAcquisition

DEFINE_HOOK(0x7098B9, TechnoClass_TargetSomethingNearby_AutoFire, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pExt->AutoTargetOwnPosition)
	{
		if (pExt->AutoTargetOwnPosition_Self)
			pThis->SetTarget(pThis);
		else
			pThis->SetTarget(pThis->GetCell());

		return 0x7099B8;
	}

	return 0;
}

static FireError __fastcall TechnoClass_TargetSomethingNearby_CanFire_Wrapper(TechnoClass* pThis, void* _, AbstractClass* pTarget, int weaponIndex, bool ignoreRange)
{
	return TechnoExt::GetFireErrorIgnoreDisableWeapons(pThis, pTarget, weaponIndex, ignoreRange);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x7098E6, TechnoClass_TargetSomethingNearby_CanFire_Wrapper);

#pragma endregion

#pragma region MapZone

namespace MapZoneTemp
{
	TargetZoneScanType zoneScanType;
}

DEFINE_HOOK(0x6F9C67, TechnoClass_GreatestThreat_MapZoneSetContext, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	MapZoneTemp::zoneScanType = pTypeExt->TargetZoneScanType;

	return 0;
}

DEFINE_HOOK(0x6F7E47, TechnoClass_EvaluateObject_MapZone, 0x7)
{
	enum { AllowedObject = 0x6F7EA2, DisallowedObject = 0x6F894F };

	GET(TechnoClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);
	GET(const int, zone, EBP);

	if (auto const pTechno = abstract_cast<TechnoClass*>(pObject))
	{
		if (!TechnoExt::AllowedTargetByZone(pThis, pTechno, MapZoneTemp::zoneScanType, nullptr, true, zone))
			return DisallowedObject;
	}

	return AllowedObject;
}

// Fix the hardcode of healing weapon can't acquire in air target.
DEFINE_HOOK(0x6F9222, TechnoClass_SelectAutoTarget_HealingTargetAir, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return pThis->CombatDamage(-1) < 0 ? 0x6F922E : 0;
}

#pragma endregion

#pragma region Walls

DEFINE_HOOK(0x70095A, TechnoClass_WhatAction_WallWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(OverlayTypeClass*, pOverlayTypeClass, STACK_OFFSET(0x2C, -0x18));

	R->EAX(pThis->GetWeapon(TechnoExt::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return 0;
}

DEFINE_HOOK(0x51C1F1, InfantryClass_CanEnterCell_WallWeapon, 0x5)
{
	enum { SkipGameCode = 0x51C1FE };

	GET(InfantryClass*, pThis, EBP);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExt::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

DEFINE_HOOK(0x73F495, UnitClass_CanEnterCell_WallWeapon, 0x6)
{
	enum { SkipGameCode = 0x73F4A1 };

	GET(UnitClass*, pThis, EBX);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExt::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

namespace CellEvalTemp
{
	int weaponIndex;
}

DEFINE_HOOK(0x6F8C9D, TechnoClass_EvaluateCell_SetContext, 0x7)
{
	GET(const int, weaponIndex, EAX);

	CellEvalTemp::weaponIndex = weaponIndex;

	return 0;
}

static WeaponStruct* __fastcall TechnoClass_EvaluateCellGetWeaponWrapper(TechnoClass* pThis)
{
	return pThis->GetWeapon(CellEvalTemp::weaponIndex);
}

static int __fastcall TechnoClass_EvaluateCellGetWeaponRangeWrapper(TechnoClass* pThis, void* _, int weaponIndex)
{
	return pThis->GetWeaponRange(CellEvalTemp::weaponIndex);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x6F8CE3, TechnoClass_EvaluateCellGetWeaponWrapper);
DEFINE_FUNCTION_JUMP(CALL6, 0x6F8DD2, TechnoClass_EvaluateCellGetWeaponRangeWrapper);

#pragma endregion

#pragma region AggressiveAttackMove

static inline bool CheckAttackMoveCanResetTarget(FootClass* pThis)
{
	const auto pTarget = pThis->Target;

	if (!pTarget || pTarget == pThis->MegaTarget)
		return false;

	const auto pTargetTechno = abstract_cast<TechnoClass*, true>(pTarget);

	if (!pTargetTechno || pTargetTechno->IsArmed())
		return false;

	if (pThis->TargetingTimer.InProgress())
		return false;

	const auto pPrimaryWeapon = pThis->GetWeapon(0)->WeaponType;

	if (!pPrimaryWeapon)
		return false;

	const auto pNewTarget = abstract_cast<TechnoClass*>(pThis->GreatestThreat(ThreatType::Range, &pThis->Location, false));

	if (!pNewTarget || pNewTarget->GetTechnoType() == pTargetTechno->GetTechnoType())
		return false;

	const auto pSecondaryWeapon = pThis->GetWeapon(1)->WeaponType;

	if (!pSecondaryWeapon || !pSecondaryWeapon->NeverUse) // Melee unit's virtual scanner
		return true;

	return pSecondaryWeapon->Range <= pPrimaryWeapon->Range;
}

DEFINE_HOOK(0x4DF3A0, FootClass_UpdateAttackMove_SelectNewTarget, 0x6)
{
	GET(FootClass* const, pThis, ECX);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->TypeExtData->AttackMove_UpdateTarget.Get(RulesExt::Global()->AttackMove_UpdateTarget)
		&& CheckAttackMoveCanResetTarget(pThis))
	{
		pThis->Target = nullptr;
		pThis->HaveAttackMoveTarget = false;
		pExt->UpdateGattlingRateDownReset();
	}

	return 0;
}

DEFINE_HOOK(0x6F85AB, TechnoClass_CanAutoTargetObject_AggressiveAttackMove, 0x6)
{
	enum { ContinueCheck = 0x6F85BA, CanTarget = 0x6F8604 };

	GET(TechnoClass* const, pThis, EDI);

	// Now, it is possible to customize which types of national active attacks on non-threatening buildings, so this part has been commented out.
	// The new judgment code is in TechnoClass_CanAutoTarget_AttackNoThreatBuildings of Hook.cpp.
	// if (!pThis->Owner->IsControlledByHuman())
	//	return CanTarget;

	if (!pThis->MegaMissionIsAttackMove())
		return ContinueCheck;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->TypeExtData->AttackMove_Aggressive.Get(RulesExt::Global()->AttackMove_Aggressive) ? CanTarget : ContinueCheck;
}

#pragma endregion

#pragma region HealingWeapons

#pragma region TechnoClass_EvaluateObject

namespace EvaluateObjectTemp
{
	WeaponTypeClass* PickedWeapon = nullptr;
}

DEFINE_HOOK(0x6F7E24, TechnoClass_EvaluateObject_SetContext, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBP);

	EvaluateObjectTemp::PickedWeapon = pWeapon;

	return 0;
}

static double __fastcall HealthRatio_Wrapper(TechnoClass* pTechno)
{
	double result = pTechno->GetHealthPercentage();

	if (result >= 1.0)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pTechno);

		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsActive())
			{
				const auto pWH = EvaluateObjectTemp::PickedWeapon ? EvaluateObjectTemp::PickedWeapon->Warhead : nullptr;
				const auto pFoot = abstract_cast<FootClass*, true>(pTechno);

				if (!pShieldData->CanBePenetrated(pWH) || ((pFoot && pFoot->ParasiteEatingMe)))
					result = pShieldData->GetHealthRatio();
			}
		}
	}

	return result;
}

DEFINE_FUNCTION_JUMP(CALL, 0x6F7F51, HealthRatio_Wrapper)

#pragma endregion

class AresScheme
{
	static inline ObjectClass* LinkedObj = nullptr;
public:
	static void __cdecl Prefix(TechnoClass* pThis, ObjectClass* pObj, int nWeaponIndex, bool considerEngineers)
	{
		if (LinkedObj)
			return;

		if (considerEngineers && CanApplyEngineerActions(pThis, pObj))
			return;

		if (nWeaponIndex < 0)
			nWeaponIndex = pThis->SelectWeapon(pObj);

		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
		{
			const auto pExt = TechnoExt::ExtMap.Find(pTechno);

			if (const auto pShieldData = pExt->Shield.get())
			{
				if (pShieldData->IsActive())
				{
					const auto pWeapon = pThis->GetWeapon(nWeaponIndex)->WeaponType;
					const auto pFoot = abstract_cast<FootClass*, true>(pTechno);

					if (pWeapon && (!pShieldData->CanBePenetrated(pWeapon->Warhead) || (pFoot && pFoot->ParasiteEatingMe)))
					{
						const auto shieldRatio = pExt->Shield->GetHealthRatio();

						if (shieldRatio < 1.0)
						{
							LinkedObj = pObj;
							--LinkedObj->Health;
						}
					}
				}
			}
		}
	}

	static void __cdecl Suffix()
	{
		if (LinkedObj)
		{
			++LinkedObj->Health;
			LinkedObj = nullptr;
		}
	}

private:
	static bool CanApplyEngineerActions(TechnoClass* pThis, ObjectClass* pTarget)
	{
		const auto pInf = abstract_cast<InfantryClass*, true>(pThis);
		const auto pBuilding = abstract_cast<BuildingClass*>(pTarget);

		if (!pInf || !pBuilding)
			return false;

		const bool allied = HouseClass::CurrentPlayer->IsAlliedWith(pBuilding);
		const auto pType = pBuilding->Type;

		if (allied && pType->Repairable)
			return true;

		if (!allied && pType->Capturable
			&& (!pBuilding->Owner->Type->MultiplayPassive
				|| !pType->CanBeOccupied
				|| pBuilding->IsBeingWarpedOut()))
		{
			return true;
		}

		return false;
	}
};

static FireError __fastcall UnitClass__GetFireError_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pThis, pObj, nWeaponIndex, false);
	auto const result = pThis->UnitClass::GetFireError(pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6030, UnitClass__GetFireError_Wrapper)

static FireError __fastcall InfantryClass__GetFireError_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pThis, pObj, nWeaponIndex, false);
	auto const result = pThis->InfantryClass::GetFireError(pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB418, InfantryClass__GetFireError_Wrapper)

static Action __fastcall UnitClass__WhatAction_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pThis, pObj, -1, false);
	auto const result = pThis->UnitClass::MouseOverObject(pObj, ignoreForce);
	AresScheme::Suffix();
	return result;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5CE4, UnitClass__WhatAction_Wrapper)

static Action __fastcall InfantryClass__WhatAction_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pThis, pObj, -1, pThis->Type->Engineer);
	auto const result = pThis->InfantryClass::MouseOverObject(pObj, ignoreForce);
	AresScheme::Suffix();
	return result;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB0CC, InfantryClass__WhatAction_Wrapper)

#pragma endregion

#pragma region ThreatEvaluation

// Current target may hurt me.
static inline bool IsAThreatToMe(TechnoClass* const pTechno, AbstractClass* const pTarget, int weaponIndex = -1)
{
	if (const auto pTechnoTarget = abstract_cast<TechnoClass*>(pTarget))
	{
		auto pTypeExt = TechnoExt::ExtMap.Find(pTechnoTarget)->TypeExtData;

		if (pTypeExt->AlwaysConsideredThreat)
			return true;

		if (weaponIndex < 0)
			weaponIndex = pTechnoTarget->SelectWeapon(pTechno);

		if (!pTechnoTarget->GetWeapon(weaponIndex)->WeaponType)
			return false;

		const auto error = pTechnoTarget->GetFireError(pTechno, weaponIndex, true);
		return pTechnoTarget->WhatAmI() == AbstractType::Building ? (error != FireError::ILLEGAL) && (error != FireError::RANGE) : (error != FireError::ILLEGAL);
	}

	return false;
}

// Decide the facing to check for firing.
static inline FacingClass* GetFireFacing(TechnoClass* const pTechno)
{
	if (!pTechno)
		return nullptr;

	switch (pTechno->WhatAmI())
	{
		case AbstractType::Building:
			return &pTechno->PrimaryFacing;
		case AbstractType::Unit:
		{
			if (pTechno->GetTechnoType()->Turret)
				return &pTechno->SecondaryFacing;
			else
				return &pTechno->PrimaryFacing;
		}
		case AbstractType::Infantry:
			return nullptr;
		case AbstractType::Aircraft:
			return &pTechno->SecondaryFacing;
		default:
			return nullptr;
	}
}

DEFINE_HOOK(0x70CF87, TechnoClass_ThreatCoefficient_CanAttackMeThreatBonus, 0x9)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(TechnoClass* const, pTarget, ESI);
	REF_STACK(double, totalThreat, STACK_OFFSET(0x58, -0x48));

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = pExt->TypeExtData;

	if (!pTypeExt->ExtraThreat_Enabled)
		return 0;

	auto ApplyIsThreatBonus = [pTypeExt, pThis, pTarget, &totalThreat]()
		{
			double bonus = pTypeExt->ExtraThreat_IsThreat.Get(RulesExt::Global()->ExtraThreat_IsThreat);

			if (bonus == 0.0)
				return;

			if (!IsAThreatToMe(pThis, pTarget))
				return;

			totalThreat += bonus;
		};
	ApplyIsThreatBonus();

	auto ApplyInRangeBonus = [pTypeExt, pThis, pTarget, &totalThreat]()
		{
			double bonus1 = pTypeExt->ExtraThreat_InRange.Get(RulesExt::Global()->ExtraThreat_InRange);
			double dist = pThis->DistanceFrom(pTarget) / 256.0;
			double bonus2 = dist * pTypeExt->ExtraThreatCoefficient_InRangeDistance.Get(RulesExt::Global()->ExtraThreatCoefficient_InRangeDistance);
			double bonus = bonus1 + bonus2;

			if (bonus == 0.0)
				return;

			if (!pThis->IsCloseEnoughToAttack(pTarget))
				return;

			totalThreat += bonus;
		};
	ApplyInRangeBonus();

	auto ApplyFacingBonus = [pTypeExt, pThis, pTarget, &totalThreat]()
		{
			auto pFacing = GetFireFacing(pThis);

			if (!pFacing)
				return;

			double bonus = pTypeExt->ExtraThreatCoefficient_Facing.Get(RulesExt::Global()->ExtraThreatCoefficient_Facing);

			if (bonus == 0.0)
				return;

			DirStruct dir = DirStruct();
			int deltaFacing = 32768 - std::abs(std::abs(pThis->GetTargetDirection(&dir, pTarget)->Raw - pFacing->Current().Raw) - 32768);
			totalThreat += deltaFacing * bonus;
		};
	ApplyFacingBonus();

	auto ApplyLastTargetDistanceBonus = [pExt, pTypeExt, pThis, pTarget, &totalThreat]()
		{
			double bonus = pTypeExt->ExtraThreatCoefficient_DistanceToLastTarget.Get(RulesExt::Global()->ExtraThreatCoefficient_DistanceToLastTarget);

			if (bonus == 0.0)
				return;

			if (pExt->LastTargetCrd == CoordStruct::Empty)
				return;

			double distToLastTarget = pTarget->GetCoords().DistanceFrom(pExt->LastTargetCrd) / 256.0;
			totalThreat += distToLastTarget * bonus;
		};
	ApplyLastTargetDistanceBonus();

	for (auto const& cb : TechnoExtInterop::CalculateExtraThreatCallbacks)
	{
		totalThreat = cb(pThis, pTarget, totalThreat);
	}

	return 0;
}


#pragma endregion

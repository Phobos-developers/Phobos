#include <AircraftClass.h>
#include <AircraftTrackerClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <TechnoClass.h>
#include <InfantryClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <VoxelAnimClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <FlyLocomotionClass.h>
#include <JumpjetLocomotionClass.h>
#include <TeleportLocomotionClass.h>
#include <BombClass.h>
#include <ParticleSystemClass.h>
#include <WarheadTypeClass.h>
#include <HashTable.h>
#include <TunnelLocomotionClass.h>
#include <TacticalClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Utilities/TemplateDef.h>

/*
	Allow usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable

	When TileSet number crosses 255 in theater INI files, the NE-SW broken bridges
	become non-repairable. The reason is that the NonMarbleMadnessTile array of size 256
	overflows when filled and affects the variables like BridgeTopRight1 and BridgeBottomLeft2
	that come after it. This patch removes the filling of the unused NonMarbleMadnessTile array
	and its references.

	Author : E1 Elite
*/
DEFINE_JUMP(LJMP, 0x545CE2, 0x545CE9) //Phobos_BugFixes_Tileset255_RemoveNonMMArrayFill
DEFINE_JUMP(LJMP, 0x546C23, 0x546C8B) //Phobos_BugFixes_Tileset255_RefNonMMArray

// Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
// Fixes recursive passenger kills not being accredited
// to proper techno but to their transports
DEFINE_PATCH(0x707CF2, 0x55);

//Fix the bug that parasite will vanish if it missed its target when its previous cell is occupied.
DEFINE_HOOK(0x62AA32, ParasiteClass_TryInfect_MissBehaviorFix, 0x5)
{
	GET(bool, isReturnSuccess, EAX);
	GET(ParasiteClass* const, pParasite, ESI);

	const auto pParasiteTechno = pParasite->Owner;

	if (isReturnSuccess || !pParasiteTechno)
		return 0;

	const auto pType = pParasiteTechno->GetTechnoType();

	if (!pType)
		return 0;

	const auto cell = MapClass::Instance.NearByLocation(pParasiteTechno->LastMapCoords, pType->SpeedType, -1,
		pType->MovementZone, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

	if (cell != CellStruct::Empty) // Cell2Coord makes X/Y values of CoordStruct non-zero, additional checks are required
		R->AL(pParasiteTechno->Unlimbo(CellClass::Cell2Coord(cell), DirType::North));

	return 0;
}

// WWP's shit code! Wrong check.
// To avoid units dying when they are already dead.
DEFINE_HOOK(0x5F53AA, ObjectClass_ReceiveDamage_DyingFix, 0x6)
{
	enum { PostMortem = 0x5F583E, ContinueCheck = 0x5F53B0 };

	GET(int, health, EAX);
	GET(ObjectClass*, pThis, ESI);

	if (health <= 0 || !pThis->IsAlive)
		return PostMortem;

	return ContinueCheck;
}

DEFINE_HOOK(0x4D7431, FootClass_ReceiveDamage_DyingFix, 0x5)
{
	GET(FootClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	if (result != DamageState::PostMortem && (pThis->IsSinking || (!pThis->IsAttackedByLocomotor && pThis->IsCrashing)))
		R->EAX(DamageState::PostMortem);

	return 0;
}

DEFINE_HOOK(0x737D57, UnitClass_ReceiveDamage_DyingFix, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (result == DamageState::NowDead && pThis->IsAttackedByLocomotor && pThis->GetTechnoType()->Crashable)
		pThis->IsAttackedByLocomotor = false;

	if (result != DamageState::PostMortem && pThis->DeathFrameCounter > 0)
		R->EAX(DamageState::PostMortem);

	return 0;
}

// Restore DebrisMaximums logic (issue #109)
// Author: Otamaa
DEFINE_HOOK(0x702299, TechnoClass_ReceiveDamage_DebrisMaximumsFix, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	auto pType = pThis->GetTechnoType();

	// If DebrisMaximums has one value, then legacy behavior is used
	if (pType->DebrisMaximums.Count == 1 &&
		pType->DebrisMaximums.GetItem(0) > 0 &&
		pType->DebrisTypes.Count > 0)
	{
		return 0;
	}

	auto totalSpawnAmount = ScenarioClass::Instance->Random.RandomRanged(
		pType->MinDebris, pType->MaxDebris);

	if (pType->DebrisTypes.Count > 0 && pType->DebrisMaximums.Count > 0)
	{
		auto cord = pThis->GetCoords();
		for (int currentIndex = 0; currentIndex < pType->DebrisTypes.Count; ++currentIndex)
		{
			if (pType->DebrisMaximums.GetItem(currentIndex) > 0)
			{
				int adjustedMaximum = Math::min(pType->DebrisMaximums.GetItem(currentIndex), pType->MaxDebris);
				int amountToSpawn = abs(ScenarioClass::Instance->Random.Random()) % (adjustedMaximum + 1); //0x702337
				amountToSpawn = Math::min(amountToSpawn, totalSpawnAmount);
				totalSpawnAmount -= amountToSpawn;

				for (; amountToSpawn > 0; --amountToSpawn)
				{
					GameCreate<VoxelAnimClass>(pType->DebrisTypes.GetItem(currentIndex),
						&cord, pThis->Owner);
				}

				if (totalSpawnAmount < 1)
					break;
			}
		}
	}

	R->EBX(totalSpawnAmount);

	return 0x7023E5;
}

// issue #250: Building placement hotkey not responding
// Author: Uranusian
DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBD5 + 7); // DisplayClass_MouseLeftRelease_HotkeyFix

DEFINE_HOOK(0x4FB2DE, HouseClass_PlaceObject_HotkeyFix, 0x6)
{
	GET(TechnoClass*, pObject, ESI);

	pObject->ClearSidebarTabObject();

	return 0;
}

// Issue #46: Laser is mirrored relative to FireFLH
// Author: Starkku
DEFINE_HOOK(0x6FF2BE, TechnoClass_FireAt_BurstOffsetFix_1, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	--pThis->CurrentBurstIndex;

	return 0x6FF2D1;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_BurstOffsetFix_2, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_BASE(int, weaponIndex, 0xC);

	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pThis->GetWeapon(weaponIndex)->WeaponType->Burst;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ForceFullRearmDelay)
	{
		pExt->ForceFullRearmDelay = false;
		pThis->CurrentBurstIndex = 0;
	}

	return 0;
}

// issue #290: Undeploy building into a unit plays EVA_NewRallyPointEstablished
// Author: secsome
DEFINE_HOOK(0x44377E, BuildingClass_ActiveClickWith, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(CellStruct*, pCell, STACK_OFFSET(0x84, 0x8));

	if (pThis->GetTechnoType()->UndeploysInto)
		pThis->SetRallypoint(pCell, false);
	else if (pThis->IsUnitFactory())
		pThis->SetRallypoint(pCell, true);

	return 0x4437AD;
}

// issue #232: Naval=yes overrides WaterBound=no and prevents move orders onto Land cells
// Author: Uranusian
DEFINE_JUMP(LJMP, 0x47CA05, 0x47CA33); // CellClass_IsClearToBuild_SkipNaval

// Check WaterBound when setting rally points / undeploying instead of just Naval.
DEFINE_HOOK(0x4438B4, BuildingClass_SetRallyPoint_Naval, 0x6)
{
	enum { IsNaval = 0x4438BC, NotNaval = 0x4438C9 };

	GET(BuildingTypeClass*, pBuildingType, EAX);
	GET_STACK(bool, playEVA, STACK_OFFSET(0xA4, 0x8));
	REF_STACK(SpeedType, spdtp, STACK_OFFSET(0xA4, -0x84));
	if (!playEVA)// assuming the hook above is the only place where it's set to false when UndeploysInto
	{
		if (auto pInto = pBuildingType->UndeploysInto)// r u sure this is not too OP?
		{
			R->ESI(pInto->MovementZone);
			spdtp = pInto->SpeedType;
			return NotNaval;
		}
	}

	if (pBuildingType->Naval || pBuildingType->SpeedType == SpeedType::Float)
		return IsNaval;

	return NotNaval;
}

DEFINE_HOOK(0x6DAAB2, TacticalClass_DrawRallyPointLines_NoUndeployBlyat, 0x6)
{
	GET(BuildingClass*, pBld, EDI);
	if (pBld->ArchiveTarget && pBld->CurrentMission != Mission::Selling)
		return 0x6DAAC0;
	return 0x6DAD45;
}

// bugfix: DeathWeapon not properly detonates
// Author: Uranusian
DEFINE_HOOK(0x70D77F, TechnoClass_FireDeathWeapon_ProjectileFix, 0x8)
{
	GET(BulletClass*, pBullet, EBX);
	GET(CoordStruct*, pCoord, EAX);

	pBullet->SetLocation(*pCoord);
	pBullet->Explode(true);

	return 0x70D787;
}

// Fix the crash of TemporalTargetingMe related "stack dump starts with 0051BB7D"
// Author: secsome
DEFINE_HOOK_AGAIN(0x43FCF9, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // BuildingClass
DEFINE_HOOK_AGAIN(0x414BDB, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // AircraftClass
DEFINE_HOOK_AGAIN(0x736204, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // UnitClass
DEFINE_HOOK(0x51BB6E, TechnoClass_AI_TemporalTargetingMe_Fix, 0x6) // InfantryClass
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->TemporalTargetingMe)
	{
		// Also check for vftable here to guarantee the TemporalClass not being destoryed already.
		if (VTable::Get(pThis->TemporalTargetingMe) == TemporalClass::AbsVTable)
			pThis->TemporalTargetingMe->Update();
		else // It should had being warped out, delete this object
		{
			pThis->TemporalTargetingMe = nullptr;
			pThis->Limbo();
			pThis->UnInit();
		}
	}

	return R->Origin() + 0xF;
}

// Fix the issue that AITriggerTypes do not recognize building upgrades
// Author: Uranusian
DEFINE_HOOK_AGAIN(0x41EEE3, AITriggerTypeClass_Condition_SupportPowersup, 0x7)	//AITriggerTypeClass_OwnerHouseOwns_SupportPowersup
DEFINE_HOOK(0x41EB43, AITriggerTypeClass_Condition_SupportPowersup, 0x7)		//AITriggerTypeClass_EnemyHouseOwns_SupportPowersup
{
	GET(HouseClass*, pHouse, EDX);
	GET(int, idxBld, EBP);
	auto const pType = BuildingTypeClass::Array[idxBld];
	int count = BuildingTypeExt::GetUpgradesAmount(pType, pHouse);

	if (count == -1)
		count = pHouse->ActiveBuildingTypes.GetItemCount(idxBld);

	R->EAX(count);

	return R->Origin() + 0xC;
}

// Dehardcode the stupid Wall-Gate relationships
// Author: Uranusian
DEFINE_HOOK(0x441053, BuildingClass_Unlimbo_EWGate, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	return RulesClass::Instance->EWGates.FindItemIndex(pThis) == -1 ? 0 : 0x441065;
}

DEFINE_HOOK(0x4410E1, BuildingClass_Unlimbo_NSGate, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	return RulesClass::Instance->NSGates.FindItemIndex(pThis) == -1 ? 0 : 0x4410F3;
}

DEFINE_HOOK(0x480552, CellClass_AttachesToNeighbourOverlay_Gate, 0x7)
{
	GET(CellClass*, pThis, EBP);
	GET(int, idxOverlay, EBX);
	GET_STACK(int, state, STACK_OFFSET(0x10, 0x8));
	bool isWall = idxOverlay != -1 && OverlayTypeClass::Array.GetItem(idxOverlay)->Wall;
	enum { Attachable = 0x480549 };

	if (isWall)
	{
		for (auto pObject = pThis->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->Health > 0)
			{
				if (auto pBuilding = abstract_cast<BuildingClass*, true>(pObject))
				{
					auto pBType = pBuilding->Type;
					if ((RulesClass::Instance->EWGates.FindItemIndex(pBType) != -1) && (state == 2 || state == 6))
						return Attachable;
					else if ((RulesClass::Instance->NSGates.FindItemIndex(pBType) != -1) && (state == 0 || state == 4))
						return Attachable;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x415F5C, AircraftClass_FireAt_SpeedModifiers, 0xA)
{
	GET(AircraftClass*, pThis, EDI);

	if (const auto pLocomotor = locomotion_cast<FlyLocomotionClass*>(pThis->Locomotor))
	{
		double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pThis);
		R->EAX(static_cast<int>(currentSpeed));
	}

	return 0;
}

DEFINE_HOOK(0x4CDA78, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ESI);

	double currentSpeed = pThis->LinkedTo->GetTechnoType()->Speed * pThis->CurrentSpeed *
		TechnoExt::GetCurrentSpeedMultiplier(pThis->LinkedTo);

	R->EAX(static_cast<int>(currentSpeed));

	return 0;
}

DEFINE_HOOK(0x4CE4BF, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
{
	GET(FlyLocomotionClass*, pThis, ECX);

	double currentSpeed = pThis->LinkedTo->GetTechnoType()->Speed * pThis->CurrentSpeed *
		TechnoExt::GetCurrentSpeedMultiplier(pThis->LinkedTo);

	R->EAX(static_cast<int>(currentSpeed));

	return 0;
}

DEFINE_HOOK(0x54D138, JumpjetLocomotionClass_Movement_AI_SpeedModifiers, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	double multiplier = TechnoExt::GetCurrentSpeedMultiplier(pThis->LinkedTo);
	pThis->Speed = (int)(pThis->LinkedTo->GetTechnoType()->JumpjetSpeed * multiplier);

	return 0;
}

DEFINE_HOOK(0x73B2A2, UnitClass_DrawObject_DrawerBlitterFix, 0x6)
{
	enum { SkipGameCode = 0x73B2C3 };

	GET(UnitClass* const, pThis, ESI);
	GET(BlitterFlags, blitterFlags, EDI);

	R->EAX(pThis->GetDrawer()->SelectPlainBlitter(blitterFlags));

	return SkipGameCode;
}

// Set all bullet params (Bright) from weapon for nuke carrier weapon.
DEFINE_HOOK(0x44CABA, BuildingClass_Mission_Missile_BulletParams, 0x7)
{
	enum { SkipGameCode = 0x44CAF2 };

	GET(BuildingClass* const, pThis, ESI);
	GET(CellClass* const, pTarget, EAX);

	auto pWeapon = SuperWeaponTypeClass::Array.GetItem(pThis->FiringSWType)->WeaponType;
	BulletClass* pBullet = nullptr;

	if (pWeapon)
		pBullet = pWeapon->Projectile->CreateBullet(pTarget, pThis, pWeapon->Damage, pWeapon->Warhead, 255, pWeapon->Bright);

	R->EAX(pBullet);
	R->EBX(pWeapon);
	return SkipGameCode;
}

// Set all bullet params (Bright) from weapon for nuke payload weapon.
DEFINE_HOOK(0x46B3E6, BulletClass_NukeMaker_BulletParams, 0x8)
{
	enum { SkipGameCode = 0x46B40D };

	GET_STACK(BulletClass* const, pThis, STACK_OFFSET(0x70, -0x60));
	GET_STACK(TechnoClass* const, pOwner, STACK_OFFSET(0x74, -0x50));
	GET(WeaponTypeClass* const, pWeapon, ESI);
	GET(AbstractClass* const, pTarget, EBX);

	pThis->Construct(pWeapon->Projectile, pTarget, pOwner, pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright);

	R->EDI(pThis);
	return SkipGameCode;
}

DEFINE_HOOK(0x6FA781, TechnoClass_AI_SelfHealing_BuildingGraphics, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
	{
		pBuilding->Mark(MarkType::Change);
		pBuilding->ToggleDamagedAnims(false);
	}

	return 0;
}

// Mitigate DeploysInto vehicles getting stuck trying to deploy while using deploy AI script action
DEFINE_HOOK(0x6ED6E5, TeamClass_TMission_Deploy_DeploysInto, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	// Handle searching for free space in Hunt mission handler
	pThis->ForceMission(Mission::Hunt);
	pThis->MissionStatus = 2; // Tells UnitClass::Mission_Hunt to omit certain checks.

	return 0;
}

DEFINE_HOOK(0x73EFD8, UnitClass_Mission_Hunt_DeploysInto, 0x6)
{
	enum { SkipToDeploy = 0x73F015 };

	GET(UnitClass*, pThis, ESI);

	// Skip MCV-specific & player control checks if coming from deploy script action.
	if (pThis->MissionStatus == 2)
	{
		pThis->MissionStatus = 0;
		return SkipToDeploy;
	}

	return 0;
}

// Fixes an issue in TechnoClass::Record_The_Kill that prevents vehicle kills from being recorded
// correctly if killed by damage that has owner house but no owner techno (animation warhead damage, radiation with owner etc.
// Author: Starkku
DEFINE_JUMP(LJMP, 0x7032BA, 0x7032C6);

namespace FetchBomb
{
	BombClass* pThisBomb;
}

// Fetch the BombClass context From earlier adress
DEFINE_HOOK(0x438771, BombClass_Detonate_SetContext, 0x6)
{
	GET(BombClass*, pThis, ESI);

	FetchBomb::pThisBomb = pThis;

	// Also adjust detonation coordinate.
	CoordStruct coords = pThis->Target->GetCenterCoords();

	R->EDX(&coords);
	return 0;
}

static DamageAreaResult __fastcall _BombClass_Detonate_DamageArea
(
	CoordStruct* pCoord,
	int nDamage, //Ares Manipulate the push stack here for Custom bomb
	TechnoClass* pSource,
	WarheadTypeClass* pWarhead, //Ares Manipulate the push stack here for Custom bomb
	bool AffectTiberium, //true
	HouseClass* pSourceHouse //nullptr
)
{
	auto const pThisBomb = FetchBomb::pThisBomb;
	auto nCoord = *pCoord;
	auto nDamageAreaResult = WarheadTypeExt::ExtMap.Find(pWarhead)->DamageAreaWithTarget
	(nCoord, nDamage, pSource, pWarhead, pWarhead->Tiberium, pThisBomb->OwnerHouse, abstract_cast<TechnoClass*>(pThisBomb->Target));
	auto nLandType = MapClass::Instance.GetCellAt(nCoord)->LandType;

	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, nLandType, nCoord))
	{
		auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, 0x2600, -15, false);

		AnimExt::SetAnimOwnerHouseKind(pAnim, pThisBomb->OwnerHouse,
			pThisBomb->Target ? pThisBomb->Target->GetOwningHouse() : nullptr, false);

		if (!pAnim->Owner)
		{
			pAnim->Owner = pThisBomb->OwnerHouse;
		}

		AnimExt::ExtMap.Find(pAnim)->SetInvoker(pThisBomb->Owner);

	}

	return nDamageAreaResult;
}

DEFINE_HOOK(0x6F5201, TechnoClass_DrawExtras_IvanBombImage, 0x6)
{
	GET(TechnoClass*, pThis, EBP);

	auto coords = pThis->GetCenterCoords();

	R->EAX(&coords);
	return 0;
}

// skip the Explosion Anim block and clean up the context
DEFINE_HOOK(0x4387A8, BombClass_Detonate_ExplosionAnimHandled, 0x5)
{
	FetchBomb::pThisBomb = nullptr;
	return 0x438857;
}

// redirect MapClass::DamageArea call to our dll for additional functionality and checks
DEFINE_FUNCTION_JUMP(CALL, 0x4387A3, _BombClass_Detonate_DamageArea);

// BibShape checks for BuildingClass::BState which needs to not be 0 (constructing) for bib to draw.
// It is possible for BState to be 1 early during construction for frame or two which can result in BibShape being drawn during buildup, which somehow depends on length of buildup.
// Trying to fix this issue at its root is problematic and most of the time causes buildup to play twice, it is simpler to simply fix the BibShape to not draw until the buildup is done - Starkku
DEFINE_HOOK(0x43D874, BuildingClass_Draw_BuildupBibShape, 0x6)
{
	enum { DontDrawBib = 0x43D8EE };

	GET(BuildingClass*, pThis, ESI);

	if (!pThis->ActuallyPlacedOnMap)
		return DontDrawBib;

	return 0;
}
// Fix techno target coordinates (used for fire angle calculations, target lines etc) to take building target coordinate offsets into accord.
// This, for an example, fixes a vanilla bug where Destroyer has trouble targeting Naval Yards with its cannon weapon from certain angles.
DEFINE_HOOK(0x70BCE6, TechnoClass_GetTargetCoords_BuildingFix, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pThis->Target))
	{
		const auto coords = pBuilding->GetTargetCoords();
		R->EAX(&coords);
	}

	return 0;
}

// Fixes C4=no amphibious infantry being killed in water if Chronoshifted/Paradropped there.
DEFINE_HOOK(0x51A996, InfantryClass_PerCellProcess_KillOnImpassable, 0x5)
{
	enum { ContinueChecks = 0x51A9A0, SkipKilling = 0x51A9EB };

	GET(InfantryClass*, pThis, ESI);
	GET(LandType, landType, EBX);

	if (landType == LandType::Rock)
		return ContinueChecks;

	if (landType == LandType::Water)
	{
		float multiplier = GroundType::Array[static_cast<int>(landType)].Cost[static_cast<int>(pThis->Type->SpeedType)];

		if (multiplier == 0.0)
			return ContinueChecks;
	}

	return SkipKilling;
}

// Fixes broken Upgrade logic to allow SpySat=Yes Code=Otamma / Tested DOOM3DGUY
DEFINE_HOOK(0x508F82, HouseClass_AI_CheckSpySat_IncludeUpgrades, 0x6)
{
	enum { AdvanceLoop = 0x508FF6, Continue = 0x508F91 };

	GET(BuildingClass const*, pBuilding, ECX);

	if (!pBuilding->Type->SpySat)
	{
		for (const auto& pUpgrade : pBuilding->Upgrades)
		{
			if (pUpgrade && pUpgrade->SpySat)
				return Continue;
		}

		return AdvanceLoop;
	}

	return Continue;
}

// BuildingClass_What_Action() - Fix no attack cursor if AG=no projectile on primary
DEFINE_JUMP(LJMP, 0x447380, 0x44739E);
DEFINE_JUMP(LJMP, 0x447709, 0x447727);

// Do not display SuperAnimThree for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
DEFINE_HOOK(0x44643E, BuildingClass_Place_SuperAnim, 0x6)
{
	enum { UseSuperAnimOne = 0x4464F6 };

	GET(BuildingClass*, pThis, EBP);
	GET(SuperClass*, pSuper, EAX);

	if (pSuper->RechargeTimer.StartTime == 0 && pSuper->RechargeTimer.TimeLeft == 0 && !SWTypeExt::ExtMap.Find(pSuper->Type)->SW_InitialReady)
	{
		R->ECX(pThis);
		return UseSuperAnimOne;
	}

	return 0;
}

// Do not advance SuperAnim for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
DEFINE_HOOK(0x451033, BuildingClass_AnimationAI_SuperAnim, 0x6)
{
	enum { SkipSuperAnimCode = 0x451048 };

	GET(SuperClass*, pSuper, EAX);

	if (pSuper->RechargeTimer.StartTime == 0 && pSuper->RechargeTimer.TimeLeft == 0 && !SWTypeExt::ExtMap.Find(pSuper->Type)->SW_InitialReady)
		return SkipSuperAnimCode;

	return 0;
}

// Stops INI parsing for Anim/BuildingTypeClass on game startup, will only be read on scenario load later like everything else.
DEFINE_JUMP(LJMP, 0x52C9C4, 0x52CA37);

// Fixes second half of Colors list not getting retinted correctly by map triggers, superweapons etc.
DEFINE_HOOK(0x53AD85, IonStormClass_AdjustLighting_ColorSchemes, 0x5)
{
	enum { SkipGameCode = 0x53ADD6 };

	GET_STACK(bool, tint, STACK_OFFSET(0x20, 0x8));
	GET(HashIterator*, it, ECX);
	GET(int, red, EBP);
	GET(int, green, EDI);
	GET(int, blue, EBX);

	int paletteCount = 0;

	for (auto pSchemes = ColorScheme::GetPaletteSchemesFromIterator(it); pSchemes; pSchemes = ColorScheme::GetPaletteSchemesFromIterator(it))
	{
		for (int i = 1; i < pSchemes->Count; i += 2)
		{
			auto pScheme = pSchemes->GetItem(i);
			pScheme->LightConvert->UpdateColors(red, green, blue, tint);
		}

		paletteCount++;
	}

	if (paletteCount > 0)
	{
		int schemeCount = ColorScheme::GetNumberOfSchemes();
		Debug::Log("Recalculated %d extra palettes across %d color schemes (total: %d).\n", paletteCount, schemeCount, schemeCount * paletteCount);
	}

	return SkipGameCode;
}

// Fixes a literal edge-case in passability checks to cover cells with bridges that are not accessible when moving on the bridge and
// normally not even attempted to enter but things like MapClass::NearByLocation() can still end up trying to pick.
DEFINE_HOOK(0x4834E5, CellClass_IsClearToMove_BridgeEdges, 0x5)
{
	enum { IsNotClear = 0x48351E };

	GET(CellClass*, pThis, ESI);
	GET(int, level, EAX);
	GET(bool, isBridge, EBX);

	if (isBridge && pThis->ContainsBridge() && (level == -1 || level == pThis->Level + CellClass::BridgeLevels)
		&& !(pThis->Flags & CellFlags::Unknown_200))
	{
		return IsNotClear;
	}

	return 0;
}

// Fix DeployToFire not working properly for WaterBound DeploysInto buildings and not recalculating position on land if can't deploy.
DEFINE_HOOK(0x4D580B, FootClass_ApproachTarget_DeployToFire, 0x6)
{
	enum { SkipGameCode = 0x4D583F };

	GET(UnitClass*, pThis, EBX);

	R->EAX(TechnoExt::CanDeployIntoBuilding(pThis, true));

	return SkipGameCode;
}

DEFINE_HOOK(0x741050, UnitClass_CanFire_DeployToFire, 0x6)
{
	enum { SkipGameCode = 0x7410B7, MustDeploy = 0x7410A8 };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->DeployToFire && pThis->CanDeployNow() && !TechnoExt::CanDeployIntoBuilding(pThis, true))
		return MustDeploy;

	return SkipGameCode;
}

// Fixed position and layer of info tip and reveal production cameo on selected building
// Author: Belonit
#pragma region DrawInfoTipAndSpiedSelection

// skip call DrawInfoTipAndSpiedSelection
// Note that Ares have the TacticalClass_DrawUnits_ParticleSystems hook at 0x6D9427
DEFINE_JUMP(LJMP, 0x6D9430, 0x6D95A1); // Tactical_RenderLayers

// Call DrawInfoTipAndSpiedSelection in new location
DEFINE_HOOK(0x6D9781, Tactical_RenderLayers_DrawInfoTipAndSpiedSelection, 0x5)
{
	GET(BuildingClass*, pBuilding, EBX);
	GET(Point2D*, pLocation, EAX);

	if (pBuilding->IsSelected && pBuilding->IsOnMap && pBuilding->WhatAmI() == AbstractType::Building)
	{
		const int foundationHeight = pBuilding->Type->GetFoundationHeight(0);
		const int typeHeight = pBuilding->Type->Height;
		const int yOffest = (Unsorted::CellHeightInPixels * (foundationHeight + typeHeight)) >> 2;

		Point2D centeredPoint = { pLocation->X, pLocation->Y - yOffest };
		pBuilding->DrawInfoTipAndSpiedSelection(&centeredPoint, &DSurface::ViewBounds);
	}

	return 0;
}
#pragma endregion DrawInfoTipAndSpiedSelection

#include <intrin.h>
bool __fastcall BuildingClass_SetOwningHouse_Wrapper(BuildingClass* pThis, void*, HouseClass* pHouse, bool announce)
{
	// Fix : Suppress capture EVA event if ConsideredVehicle=yes
	if(announce) announce = !pThis->IsStrange();

	bool res = reinterpret_cast<bool(__thiscall*)(BuildingClass*, HouseClass*, bool)>(0x448260)(pThis, pHouse, announce);
	// TODO: something goes wrong in TAction 36, fix it later
	DWORD const caller =(DWORD) _ReturnAddress();
	if(caller > 0x6E0C91 || caller < 0x6E0B60)
	if (res && (pThis->Type->Powered || pThis->Type->PoweredSpecial))
		reinterpret_cast<void(__thiscall*)(BuildingClass*)>(0x4549B0)(pThis);
	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4290, BuildingClass_SetOwningHouse_Wrapper);

// Fix a glitch related to incorrect target setting for missiles
// Author: Belonit
DEFINE_HOOK(0x6B75AC, SpawnManagerClass_AI_SetDestinationForMissiles, 0x5)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(TechnoClass*, pSpawnTechno, EDI);

	CoordStruct coord = pSpawnManager->Target->GetCenterCoords();
	CellClass* pCellDestination = MapClass::Instance.TryGetCellAt(coord);

	pSpawnTechno->SetDestination(pCellDestination, true);

	return 0x6B75BC;
}

DEFINE_HOOK(0x689EB0, ScenarioClass_ReadMap_SkipHeaderInCampaign, 0x6)
{
	return SessionClass::IsCampaign() ? 0x689FC0 : 0;
}

#pragma region save_load

//Skip incorrect load ctor call in various Load
DEFINE_JUMP(LJMP, 0x719CBC, 0x719CD8);//Teleport, notorious CLEG frozen state removal on loading game
DEFINE_JUMP(LJMP, 0x72A16A, 0x72A186);//Tunnel, not a big deal
DEFINE_JUMP(LJMP, 0x663428, 0x663445);//Rocket, not a big deal
DEFINE_JUMP(LJMP, 0x5170CE, 0x5170E0);//Hover, not a big deal
DEFINE_JUMP(LJMP, 0x65B3F7, 0x65B416);//RadSite, no effect
DEFINE_JUMP(LJMP, 0x6F4317, 0x6F43AB);
DEFINE_JUMP(LJMP, 0x6F43B5, 0x6F43C7);//Techno (AirstrikeTimer,CloakProgress,TurretRecoil,BarrelRecoil)
DEFINE_JUMP(LJMP, 0x43B694, 0x43B6C3);//Building(RepairProgress)
// Save GameModeOptions in campaign modes
DEFINE_JUMP(LJMP, 0x67E3BD, 0x67E3D3); // Save
DEFINE_JUMP(LJMP, 0x67F72E, 0x67F744); // Load

#pragma endregion save_load

// An attempt to fix an issue where the ATC->CurrentVector does not contain every air Techno in given range that increases in frequency as the range goes up.
// Real talk: I have absolutely no clue how the original function works besides doing vector looping and manipulation, as far as I can tell it never even explicitly
// clears CurrentVector but somehow it only contains applicable items afterwards anyway. It is possible this one does not achieve everything the original does functionality and/or
// performance-wise but it does work and produces results with greater accuracy than the original for large ranges. - Starkku
DEFINE_HOOK(0x412B40, AircraftTrackerClass_FillCurrentVector, 0x5)
{
	enum { SkipGameCode = 0x413482 };

	GET(AircraftTrackerClass*, pThis, ECX);
	GET_STACK(CellClass*, pCell, 0x4);
	GET_STACK(int, range, 0x8);

	pThis->CurrentVector.Clear();

	if (range < 1)
		range = 1;

	auto const bounds = MapClass::Instance.MapCoordBounds;
	auto const mapCoords = pCell->MapCoords;
	int sectorWidth = bounds.Right / 20;
	int sectorHeight = bounds.Bottom / 20;
	int sectorIndexXStart = Math::clamp((mapCoords.X - range) / sectorWidth, 0, 19);
	int sectorIndexYStart = Math::clamp((mapCoords.Y - range) / sectorHeight, 0, 19);
	int sectorIndexXEnd = Math::clamp((mapCoords.X + range) / sectorWidth, 0, 19);
	int sectorIndexYEnd = Math::clamp((mapCoords.Y + range) / sectorHeight, 0, 19);

	for (int y = sectorIndexYStart; y <= sectorIndexYEnd; y++)
	{
		for (int x = sectorIndexXStart; x <= sectorIndexXEnd; x++)
		{
			for (auto const pTechno : pThis->TrackerVectors[y][x])
				pThis->CurrentVector.AddItem(pTechno);
		}
	}

	R->EAX(0); // The original function returns some number, hell if I know what (it is 0 most of the time though and never actually used for anything).
	return SkipGameCode;
}

// this fella was { 0, 0, 1 } before and somehow it also breaks both the light position a bit and how the lighting is applied when voxels rotate - Kerbiter
DEFINE_HOOK(0x753D86, VoxelCalcNormals_NullAdditionalVector, 0x0)
{
	REF_STACK(Vector3D<float>, secondaryLightVector, STACK_OFFSET(0xD8, -0xC0));

	if (RulesExt::Global()->UseFixedVoxelLighting)
		secondaryLightVector = { 0, 0, 0 };
	else
		secondaryLightVector = { 0, 0, 1 };

	return 0x753D9E;
}

DEFINE_HOOK(0x705D74, TechnoClass_GetRemapColour_DisguisePalette, 0x8)
{
	enum { SkipGameCode = 0x705D7C };

	GET(TechnoClass* const, pThis, ESI);

	auto pTechnoType = pThis->GetTechnoType();

	if (!pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
	{
		if (const auto pDisguise = TechnoTypeExt::GetTechnoType(pThis->Disguise))
			pTechnoType = pDisguise;
	}

	R->EAX(pTechnoType);

	return SkipGameCode;
}

// Fixes an edge case crash caused by temporal targeting enslaved infantry.
DEFINE_HOOK(0x71ADE4, TemporalClass_Release_SlaveTargetFix, 0x5)
{
	enum { ReturnFromFunction = 0x71AE47 };

	GET(TemporalClass* const, pThis, ESI);

	if (!pThis->Target)
		return ReturnFromFunction;

	return 0;
}

// In the following three places the distance check was hardcoded to compare with 20, 17 and 16 respectively,
// which means it didn't consider the actual speed of the unit. Now we check it and the units won't get stuck
// even at high speeds - NetsuNegi

DEFINE_HOOK(0x72958E, TunnelLocomotionClass_ProcessDigging_SlowdownDistance, 0x8)
{
	enum { KeepMoving = 0x72980F, CloseEnough = 0x7295CE };

	GET(TunnelLocomotionClass* const, pLoco, ESI);

	auto& currLoc = pLoco->LinkedTo->Location;
	int distance = (int) CoordStruct{currLoc.X - pLoco->Coords.X, currLoc.Y - pLoco->Coords.Y,0}.Magnitude() ;

	// The movement speed was actually also hardcoded here to 19, so the distance check made sense
	// It can now be customized globally or per TechnoType however - Starkku
	auto const pType = pLoco->LinkedTo->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	int speed = pTypeExt->SubterraneanSpeed >= 0 ? pTypeExt->SubterraneanSpeed : RulesExt::Global()->SubterraneanSpeed;

	// Calculate speed multipliers.
	pLoco->LinkedTo->SpeedPercentage = 1.0; // Subterranean locomotor doesn't normally use this so it would be 0.0 here and cause issues.
	int maxSpeed = pType->Speed;
	pType->Speed = speed;
	speed = pLoco->LinkedTo->GetCurrentSpeed();
	pType->Speed = maxSpeed;

	if (distance > speed)
	{
		REF_STACK(CoordStruct, newLoc, STACK_OFFSET(0x40, -0xC));
		double angle = -Math::atan2(currLoc.Y - pLoco->Coords.Y, pLoco->Coords.X - currLoc.X);
		newLoc = currLoc + CoordStruct { int((double)speed * Math::cos(angle)), int((double)speed * Math::sin(angle)), 0 };
		return 0x7298D3;
	}
	return 0x7295CE;
}

DEFINE_HOOK(0x75BD70, WalkLocomotionClass_ProcessMoving_SlowdownDistance, 0x9)
{
	enum { KeepMoving = 0x75BF85, CloseEnough = 0x75BD79 };

	GET(FootClass* const, pLinkedTo, ECX);
	GET(int const, distance, EAX);

	return distance >= pLinkedTo->GetCurrentSpeed() ? KeepMoving : CloseEnough;
}

DEFINE_HOOK(0x5B11DD, MechLocomotionClass_ProcessMoving_SlowdownDistance, 0x9)
{
	enum { KeepMoving = 0x5B14AA, CloseEnough = 0x5B11E6 };

	GET(FootClass* const, pLinkedTo, ECX);
	GET(int const, distance, EAX);

	return distance >= pLinkedTo->GetCurrentSpeed() ? KeepMoving : CloseEnough;
}

DEFINE_JUMP(LJMP, 0x517FF5, 0x518016); // Warhead with InfDeath=9 versus infantry in air

// Fixes docks not repairing docked aircraft unless they enter the dock first e.g just built ones.
// Also potential edge cases with unusual docking offsets, original had a distance check for 64 leptons which is replaced with IsInAir here.
DEFINE_HOOK(0x44985B, BuildingClass_Mission_Guard_UnitReload, 0x6)
{
	enum { AssignRepairMission = 0x449942 };

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pLink, EDI);

	if (pThis->Type->UnitReload && pLink->WhatAmI() == AbstractType::Aircraft && !pLink->IsInAir()
		&& pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
	{
		return AssignRepairMission;
	}

	return 0;
}

// Fix tileset parsing to not reset certain tileset indices for Lunar theater if the fix is enabled.
DEFINE_HOOK(0x546C95, IsometricTileTypeClass_ReadINI_LunarFixes, 0x6)
{
	enum { SkipGameCode = 0x546CBF };

	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0xA10, -0x9D8));

	if (pINI->ReadBool(GameStrings::General, "ApplyLunarFixes", false))
		return SkipGameCode;

	return 0;
}

// Fixes an edge case that affects AI-owned technos where they lose ally targets instantly even if they have AttackFriendlies=yes - Starkku
DEFINE_HOOK(0x6FA467, TechnoClass_AI_AttackFriendlies, 0x5)
{
	enum { SkipResetTarget = 0x6FA472 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->GetTechnoType()->AttackFriendlies
		&& pThis->Target->GetOwningHouse()->IsAlliedWith(pThis) // TODO
	)
		return SkipResetTarget;

	return 0;
}

// Starkku: These fix issues with follower train cars etc) indices being thrown off by preplaced vehicles not being created, having other vehicles as InitialPayload etc.
// This fix basically works by not using the global UnitClass array at all for setting the followers, only a list of preplaced units, successfully created or not.
#pragma region Follower

namespace UnitParseTemp
{
	std::vector<UnitClass*> ParsedUnits;
	bool WasCreated = false;
}

// Add vehicles successfully created to list of parsed vehicles.
DEFINE_HOOK(0x7435DE, UnitClass_ReadFromINI_Follower1, 0x6)
{
	GET(UnitClass*, pUnit, ESI);

	UnitParseTemp::ParsedUnits.push_back(pUnit);
	UnitParseTemp::WasCreated = true;

	return 0;
}

// Add vehicles that were not successfully created to list of parsed vehicles as well as to followers list.
DEFINE_HOOK(0x74364C, UnitClass_ReadFromINI_Follower2, 0x8)
{
	REF_STACK(TypeList<int>, followers, STACK_OFFSET(0xD0, -0xC0));

	if (!UnitParseTemp::WasCreated)
	{
		followers.AddItem(-1);
		UnitParseTemp::ParsedUnits.push_back(nullptr);
	}

	UnitParseTemp::WasCreated = false;

	return 0;
}

// Set followers based on parsed vehicles.
DEFINE_HOOK(0x743664, UnitClass_ReadFromINI_Follower3, 0x6)
{
	enum { SkipGameCode = 0x7436AC };

	REF_STACK(TypeList<int>, followers, STACK_OFFSET(0xCC, -0xC0));
	auto& units = UnitParseTemp::ParsedUnits;

	for (size_t i = 0; i < units.size(); i++)
	{
		auto const pUnit = units[i];

		if (!pUnit)
			continue;

		int followerIndex = followers[i];

		if (followerIndex < 0 || followerIndex >= static_cast<int>(units.size()))
		{
			pUnit->FollowerCar = nullptr;
		}
		else
		{
			auto const pFollower = units[followerIndex];
			pUnit->FollowerCar = pFollower;
			pFollower->IsFollowerCar = true;
		}
	}

	units.clear();

	return SkipGameCode;
}

#pragma endregion

#pragma region TeleportLocomotionOccupationFix

DEFINE_HOOK(0x71872C, TeleportLocomotionClass_MakeRoom_OccupationFix, 0x9)
{
	enum { SkipMarkOccupation = 0x71878F };

	GET(const LocomotionClass* const, pLoco, EBP);

	const auto pFoot = pLoco->LinkedTo;

	return (pFoot && pFoot->IsAlive && !pFoot->InLimbo && pFoot->Health > 0 && !pFoot->IsSinking) ? 0 : SkipMarkOccupation;
}

#pragma endregion

#pragma region AmphibiousHarvester

DEFINE_HOOK(0x73ED66, UnitClass_Mission_Harvest_PathfindingFix, 0x5)
{
	GET(UnitClass*, pThis, EBP);

	const auto pType = pThis->Type;

	if (!pType->Teleporter)
	{
		REF_STACK(SpeedType, speedType, STACK_OFFSET(0xA0, -0x98));
		REF_STACK(int, currentZoneType, STACK_OFFSET(0xA0, -0x94));
		REF_STACK(MovementZone, movementZone, STACK_OFFSET(0xA0, -0x90));

		speedType = pType->SpeedType;
		movementZone = pType->MovementZone;
		currentZoneType = MapClass::Instance.GetMovementZoneType(pThis->GetMapCoords(), movementZone, pThis->OnBridge);
	}

	return 0;
}

#pragma endregion

#pragma region StopEventFix

DEFINE_JUMP(LJMP, 0x4C756B, 0x4C757D); // Skip cell under bridge check

DEFINE_HOOK(0x4C75DA, EventClass_RespondToEvent_Stop, 0x6)
{
	enum { SkipGameCode = 0x4C762A };

	GET(TechnoClass* const, pTechno, ESI);

	// Check aircraft
	const auto pAircraft = abstract_cast<AircraftClass*>(pTechno);
	const bool commonAircraft = pAircraft && !pAircraft->Airstrike && !pAircraft->Spawned;
	const auto mission = pTechno->CurrentMission;

	// To avoid aircraft overlap by keep link if is returning or is in airport now.
	if (!commonAircraft || (mission != Mission::Sleep && mission != Mission::Guard && mission != Mission::Enter)
		|| !pAircraft->DockNowHeadingTo || (pAircraft->DockNowHeadingTo != pAircraft->GetNthLink()))
	{
		pTechno->SendToEachLink(RadioCommand::NotifyUnlink);
	}

	// To avoid technos being unable to stop in attack move mega mission
	if (pTechno->MegaMissionIsAttackMove())
		pTechno->ClearMegaMissionData();

	// Clearing the current target should still be necessary for all technos
	pTechno->SetTarget(nullptr);

	// Stop any enter action
	pTechno->QueueUpToEnter = nullptr;

	if (commonAircraft)
	{
		if (pAircraft->Type->AirportBound)
		{
			// To avoid `AirportBound=yes` aircraft with ammo at low altitudes cannot correctly receive stop command and queue Mission::Guard with a `Destination`.
			if (pAircraft->Ammo)
				pTechno->SetDestination(nullptr, true);

			// To avoid `AirportBound=yes` aircraft pausing in the air and let they returning to air base immediately.
			if (!pAircraft->DockNowHeadingTo || (pAircraft->DockNowHeadingTo != pAircraft->GetNthLink())) // If the aircraft have no valid dock, try to find a new one
				pAircraft->EnterIdleMode(false, true);
		}
		else if (pAircraft->Ammo)
		{
			// To avoid `AirportBound=no` aircraft ignoring the stop task or directly return to the airport.
			if (pAircraft->Destination && static_cast<int>(CellClass::Coord2Cell(pAircraft->Destination->GetCoords()).DistanceFromSquared(pAircraft->GetMapCoords())) > 2) // If the aircraft is moving, find the forward cell then stop in it
				pAircraft->SetDestination(pAircraft->GetCell()->GetNeighbourCell(static_cast<FacingType>(pAircraft->PrimaryFacing.Current().GetValue<3>())), true);
		}
		else if (!pAircraft->DockNowHeadingTo || (pAircraft->DockNowHeadingTo != pAircraft->GetNthLink()))
		{
			pAircraft->EnterIdleMode(false, true);
		}
		// Otherwise landing or idling normally without answering the stop command
	}
	else
	{
		const auto pFoot = abstract_cast<FootClass*, true>(pTechno);

		// Clear archive target for infantries and vehicles like receive a mega mission
		if (pFoot && !pAircraft)
			pTechno->SetArchiveTarget(nullptr);

		// Only stop when it is not under the bridge (meeting the original conditions which has been skipped)
		if (!pTechno->vt_entry_2B0() || pTechno->OnBridge || pTechno->IsInAir() || pTechno->GetCell()->SlopeIndex)
		{
			// To avoid foots stuck in Mission::Area_Guard
			if (pTechno->CurrentMission == Mission::Area_Guard && !pTechno->GetTechnoType()->DefaultToGuardArea)
				pTechno->QueueMission(Mission::Guard, true);

			// Check Jumpjets
			const auto pJumpjetLoco = pFoot ? locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor) : nullptr;

			// To avoid jumpjets falling into a state of standing idly by
			if (!pJumpjetLoco) // If is not jumpjet, clear the destination is enough
				pTechno->SetDestination(nullptr, true);
			else if (!pFoot->Destination) // When in attack move and have had a target, the destination will be cleaned up, enter the guard mission can prevent the jumpjets stuck in a status of standing idly by
				pTechno->QueueMission(Mission::Guard, true);
			else if (static_cast<int>(CellClass::Coord2Cell(pFoot->Destination->GetCoords()).DistanceFromSquared(pTechno->GetMapCoords())) > 2) // If the jumpjet is moving, find the forward cell then stop in it
				pTechno->SetDestination(pTechno->GetCell()->GetNeighbourCell(static_cast<FacingType>(pJumpjetLoco->LocomotionFacing.Current().GetValue<3>())), true);
			// Otherwise landing or idling normally without answering the stop command
		}
	}

	return SkipGameCode;
}

#pragma endregion

#pragma region UntetherFix

// Change enter to move when unlink
DEFINE_HOOK(0x6F4C50, TechnoClass_ReceiveCommand_NotifyUnlink, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(TechnoClass* const, pCall, STACK_OFFSET(0x18, 0x4));
	// If the link connection is cancelled and foot A is entering techno B, it may cause A and B to overlap
	if (!pCall->InLimbo // Has not already entered
		&& (pCall->AbstractFlags & AbstractFlags::Foot) // Is foot
		&& pCall->CurrentMission == Mission::Enter // Is entering
		&& static_cast<FootClass*>(pCall)->Destination == pThis) // Is entering techno B
	{
		pCall->SetDestination(pThis->GetCell(), false); // Set the destination at its feet
		pCall->QueueMission(Mission::Move, false); // Replace entering with moving
		pCall->NextMission(); // Immediately respond to the Mission::Move
	}

	return 0;
}

// Do not untether techno who have other tether link
DEFINE_HOOK(0x6F4BB3, TechnoClass_ReceiveCommand_RequestUntether, 0x7)
{
	// Place the hook after processing to prevent functions from calling each other and getting stuck in a dead loop.
	GET(TechnoClass* const, pThis, ESI);
	// The radio link capacity of some technos can be greater than 1 (like airport)
	// Here is a specific example, there may be other situations as well:
	// - Untether without check may result in `AirportBound=no` aircraft being unable to release from `IsTether` status.
	// - Specifically, all four aircraft are connected to the airport and have `RadioLink` settings, but when the first aircraft
	//   is `Unlink` from the airport, all subsequent aircraft will be stuck in `IsTether` status.
	// - This is because when both parties who are `RadioLink` to each other need to `Unlink`, they need to `Untether` first,
	//   and this requires ensuring that both parties have `IsTether` flag (0x6F4C50), otherwise `Untether` cannot be successful,
	//   which may lead to some unexpected situations.
	for (int i = 0; i < pThis->RadioLinks.Capacity; ++i)
	{
		if (const auto pLink = pThis->RadioLinks.Items[i])
		{
			if (pLink->IsTether) // If there's another tether link, reset flag to true
				pThis->IsTether = true; // Ensures that other links can be properly untether afterwards
		}
	}

	return 0;
}

#pragma endregion

#pragma region JumpjetShadowPointFix

Point2D *__stdcall JumpjetLoco_ILoco_Shadow_Point(ILocomotion * iloco, Point2D *pPoint)
{
	__assume(iloco != nullptr);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iloco);
	const auto pThis = pLoco->LinkedTo;
	const auto pCell = MapClass::Instance.GetCellAt(pThis->Location);
	auto height = pThis->Location.Z - MapClass::Instance.GetCellFloorHeight(pThis->Location);
	// Vanilla GetHeight check OnBridge flag, which can not work on jumpjet
	// Here, we simulate the drawing of an airplane for altitude calculation
	if (pCell->ContainsBridge()
		&& ((pCell->Flags & CellFlags::BridgeDir) && pCell->GetNeighbourCell(FacingType::North)->ContainsBridge()
			|| !(pCell->Flags & CellFlags::BridgeDir) && pCell->GetNeighbourCell(FacingType::West)->ContainsBridge()))
	{
		height -= CellClass::BridgeHeight;
	}

	*pPoint = Point2D { 0, TacticalClass::AdjustForZ(height) };
	return pPoint;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECD98, JumpjetLoco_ILoco_Shadow_Point);

#pragma endregion

#pragma region SpawnerFix

// Enable the carrier on the bridge to retrieve the aircraft normally
DEFINE_HOOK(0x4CF3F9, FlyLocomotionClass_FlightUpdate_FixFlightLevel, 0x5)
{
	enum { SkipGameCode = 0x4CF4D2 };

	GET(FlyLocomotionClass* const, pThis, EBP);

	const auto pFoot = pThis->LinkedTo;

	if (pFoot->GetMapCoords() == CellClass::Coord2Cell(pThis->MovingDestination) // Maintain height until on same cell to prevent poor visual display
		&& MapClass::Instance.GetCellAt(pFoot->Location)->ContainsBridge() // Only effective when on the bridge
		&& pThis->FlightLevel >= CellClass::BridgeHeight) // Not lower than the ground level
	{
		// Subtract the excess bridge height to allow the aircraft to return to the correct altitude
		pThis->FlightLevel -= CellClass::BridgeHeight;
	}

	return SkipGameCode;
}

// Let in air aircraft carrier ignore nearby elevated bridge check
DEFINE_HOOK(0x6FC617, TechnoClass_GetFireError_Spawner, 0x8)
{
	enum { ContinueCheck = 0x6FC61F, TemporaryCannotFire = 0x6FCD0E };

	GET(TechnoClass* const, pThis, ESI);
	GET(const bool, nearElevatedBridge, EAX);

	// In addition, the return value of the function has been changed to allow the aircraft carrier to retain the current target
	return (nearElevatedBridge && !pThis->IsInAir()) ? TemporaryCannotFire : ContinueCheck;
}

#pragma endregion

#pragma region TeamCloseRangeFix

int __fastcall Check2DDistanceInsteadOf3D(ObjectClass* pSource, void* _, AbstractClass* pTarget)
{
	// At present, it seems that aircraft use their own mapcoords and the team destination's mapcoords to check.
    // During the previous test, it was found that if the aircraft uses this and needs to return to the airport
	// with the script first, it will interrupt the remaining tasks for unknown reasons - CrimRecya
	return (pSource->IsInAir() && pSource->WhatAmI() != AbstractType::Aircraft) // Jumpjets or sth in the air
		? (pSource->DistanceFrom(pTarget) * 2) // 2D distance (2x is the bonus to units in the air)
		: pSource->DistanceFrom3D(pTarget); // 3D distance (vanilla)
}
DEFINE_FUNCTION_JUMP(CALL, 0x6EBCC9, Check2DDistanceInsteadOf3D);

#pragma endregion

// This shouldn't be here
// Author: tyuah8
DEFINE_HOOK_AGAIN(0x4AF94D, EndPiggyback_PowerOn, 0x7) // Drive
DEFINE_HOOK_AGAIN(0x54DADC, EndPiggyback_PowerOn, 0x5) // Jumpjet
DEFINE_HOOK_AGAIN(0x69F05D, EndPiggyback_PowerOn, 0x7) // Ship
DEFINE_HOOK(0x719F17, EndPiggyback_PowerOn, 0x5) // Teleport
{
	auto* iloco = R->Origin() == 0x719F17 ? R->ECX<ILocomotion*>() : R->EAX<ILocomotion*>();
	__assume(iloco!=nullptr);
	auto pLinkedTo = static_cast<LocomotionClass*>(iloco)->LinkedTo;
	if (!pLinkedTo->Deactivated && !pLinkedTo->IsUnderEMP())
		iloco->Power_On();
	else
		iloco->Power_Off();
	return 0;
}

// Suppress Ares' swizzle warning
size_t __fastcall HexStr2Int_replacement(const char* str)
{
	// Fake a pointer to trick Ares
	return std::hash<std::string_view>{}(str) & 0xFFFFFF;
}

DEFINE_FUNCTION_JUMP(CALL, 0x6E8305, HexStr2Int_replacement); // TaskForce
DEFINE_FUNCTION_JUMP(CALL, 0x6E5FA6, HexStr2Int_replacement); // TagType

#pragma region Sensors

DEFINE_HOOK(0x4DE839, FootClass_AddSensorsAt_Record, 0x6)
{
	GET(FootClass*, pThis, ESI);
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x34, 0x4));
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->LastSensorsMapCoords = *cell;

	return 0;
}

DEFINE_HOOK(0x4D8606, FootClass_UpdatePosition_Sensors, 0x6)
{
	enum { SkipGameCode = 0x4D8627 };

	GET(FootClass*, pThis, ESI);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto currentCell = pThis->GetMapCoords();

	if (pExt->LastSensorsMapCoords != currentCell)
	{
		pThis->RemoveSensorsAt(pExt->LastSensorsMapCoords);
		pThis->AddSensorsAt(currentCell);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x4DB36C, FootClass_Limbo_RemoveSensors, 0x5)
{
	enum { SkipGameCode = 0x4DB37C };

	GET(FootClass*, pThis, EDI);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	pThis->RemoveSensorsAt(pExt->LastSensorsMapCoords);

	return SkipGameCode;
}

DEFINE_HOOK(0x4DBEE7, FootClass_SetOwningHouse_RemoveSensors, 0x6)
{
	enum { SkipGameCode = 0x4DBF01 };

	GET(FootClass*, pThis, ESI);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	pThis->RemoveSensorsAt(pExt->LastSensorsMapCoords);

	return SkipGameCode;
}

// Bugfix: Jumpjet detect cloaked objects beneath
DEFINE_HOOK(0x54C036, JumpjetLocomotionClass_State3_UpdateSensors, 0x7)
{
	GET(FootClass* const, pLinkedTo, ECX);
	GET(CellStruct const, currentCell, EAX);

	// Copied from FootClass::UpdatePosition
	if (pLinkedTo->GetTechnoType()->SensorsSight)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pLinkedTo);
		CellStruct const lastCell = pExt->LastSensorsMapCoords;

		if (lastCell != currentCell)
		{
			pLinkedTo->RemoveSensorsAt(lastCell);
			pLinkedTo->AddSensorsAt(currentCell);
		}
	}
	// Something more may be missing

	return 0;
}

DEFINE_HOOK(0x54D06F, JumpjetLocomotionClass_ProcessCrashing_RemoveSensors, 0x5)
{
	GET(FootClass*, pLinkedTo, EAX);

	if (pLinkedTo->GetTechnoType()->SensorsSight)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pLinkedTo);
		pLinkedTo->RemoveSensorsAt(pExt->LastSensorsMapCoords);
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x688F8C, ScenarioClass_ScanPlaceUnit_CheckMovement, 0x5)
{
	enum { NotUsableArea = 0x688FB9 };

	GET(TechnoClass*, pTechno, EBX);
	LEA_STACK(CoordStruct*, pCoords, STACK_OFFSET(0x6C, -0x30));

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
		return 0;

	const auto pCell = MapClass::Instance.GetCellAt(*pCoords);
	const auto pTechnoType = pTechno->GetTechnoType();

	return pCell->IsClearToMove(pTechnoType->SpeedType, false, false, -1, pTechnoType->MovementZone, -1, 1) ? 0 : NotUsableArea;
}

DEFINE_HOOK(0x68927B, ScenarioClass_ScanPlaceUnit_CheckMovement2, 0x5)
{
	enum { NotUsableArea = 0x689295 };

	GET(TechnoClass*, pTechno, EDI);
	LEA_STACK(CoordStruct*, pCoords, STACK_OFFSET(0x6C, -0xC));

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
		return 0;

	const auto pCell = MapClass::Instance.GetCellAt(*pCoords);
	const auto pTechnoType = pTechno->GetTechnoType();

	return pCell->IsClearToMove(pTechnoType->SpeedType, false, false, -1, pTechnoType->MovementZone, -1, 1) ? 0 : NotUsableArea;
}

DEFINE_HOOK(0x446BF4, BuildingClass_Place_FreeUnit_NearByLocation, 0x6)
{
	enum { SkipGameCode = 0x446CD2 };

	GET(BuildingClass*, pThis, EBP);
	GET(UnitClass*, pFreeUnit, EDI);
	LEA_STACK(CellStruct*, outBuffer, STACK_OFFSET(0x68, -0x4C));
	const auto mapCoords = CellClass::Coord2Cell(pThis->Location);
	const auto movementZone = pFreeUnit->Type->MovementZone;
	const auto currentZone = MapClass::Instance.GetMovementZoneType(mapCoords, movementZone, false);

	R->EAX(MapClass::Instance.NearByLocation(*outBuffer, mapCoords, pFreeUnit->Type->SpeedType, currentZone, movementZone, false, 1, 1, true, true, false, false, CellStruct::Empty, false, false));
	return SkipGameCode;
}

DEFINE_HOOK(0x446D42, BuildingClass_Place_FreeUnit_NearByLocation2, 0x6)
{
	enum { SkipGameCode = 0x446E15 };

	GET(BuildingClass*, pThis, EBP);
	GET(UnitClass*, pFreeUnit, EDI);
	LEA_STACK(CellStruct*, outBuffer, STACK_OFFSET(0x68, -0x4C));
	const auto mapCoords = CellClass::Coord2Cell(pThis->Location);
	const auto movementZone = pFreeUnit->Type->MovementZone;
	const auto currentZone = MapClass::Instance.GetMovementZoneType(mapCoords, movementZone, false);

	R->EAX(MapClass::Instance.NearByLocation(*outBuffer, mapCoords, pFreeUnit->Type->SpeedType, currentZone, movementZone, false, 1, 1, false, true, false, false, CellStruct::Empty, false, false));
	return SkipGameCode;
}

DEFINE_HOOK(0x449462, BuildingClass_IsCellOccupied_UndeploysInto, 0x6)
{
	enum { SkipGameCode = 0x449487 };

	GET(BuildingTypeClass*, pType, EAX);
	LEA_STACK(CellStruct*, pDest, 0x4);
	const auto pCell = MapClass::Instance.GetCellAt(*pDest);
	const auto pUndeploysInto = pType->UndeploysInto;

	R->AL(pCell->IsClearToMove(pUndeploysInto->SpeedType, false, false, -1, pUndeploysInto->MovementZone, -1, 1));
	return SkipGameCode;
}

#pragma region XSurfaceFix

// Fix a crash at 0x7BAEA1 when trying to access a point outside of surface bounds.
class XSurfaceFake final : public XSurface
{
	int _GetPixel(Point2D const& point) const;
};

int XSurfaceFake::_GetPixel(Point2D const& point) const
{
	int color = 0;

	Point2D finalPoint = point;
	if (finalPoint.X > Width || finalPoint.Y > Height)
		finalPoint = Point2D::Empty;

	void* pointer = ((Surface*)this)->Lock(finalPoint.X, finalPoint.Y);
	if (pointer != nullptr)
	{
		if (BytesPerPixel == 2)
			color = *static_cast<unsigned short*>(pointer);
		else
			color = *static_cast<unsigned char*>(pointer);
		((Surface*)this)->Unlock();
	}
	return color;
}

DEFINE_FUNCTION_JUMP(CALL, 0x4A3E8A, XSurfaceFake::_GetPixel)
DEFINE_FUNCTION_JUMP(CALL, 0x4A3EB7, XSurfaceFake::_GetPixel);
DEFINE_FUNCTION_JUMP(CALL, 0x4A3F7C, XSurfaceFake::_GetPixel);
DEFINE_FUNCTION_JUMP(CALL, 0x642213, XSurfaceFake::_GetPixel);
DEFINE_FUNCTION_JUMP(CALL, 0x6423D6, XSurfaceFake::_GetPixel);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2098, XSurfaceFake::_GetPixel);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E212C, XSurfaceFake::_GetPixel);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E85FC, XSurfaceFake::_GetPixel);

#pragma endregion

#pragma region PointerExpired

// Fixed incorrect process in PointerExpired.
// Add checks for bRemoved.
DEFINE_HOOK(0x7077FD, TechnoClass_PointerExpired_SpawnOwnerFix, 0x6)
{
	GET_STACK(bool, removed, STACK_OFFSET(0x20, 0x8));
	// Skip the reset for SpawnOwner if !removed.
	return removed ? 0 : 0x707803;
}

DEFINE_HOOK(0x468503, BulletClass_PointerExpired_OwnerFix, 0x6)
{
	GET_STACK(bool, removed, STACK_OFFSET(0x1C, 0x8));
	// Skip the reset for Owner if !removed.
	return removed ? 0 : 0x468509;
}

DEFINE_HOOK(0x44E910, BuildingClass_PointerExpired_C4ExpFix, 0x6)
{
	GET_STACK(bool, removed, STACK_OFFSET(0xC, 0x8));
	// Skip the reset for C4AppliedBy if !removed.
	return removed ? 0 : 0x44E916;
}

DEFINE_HOOK(0x725961, AbstractClass_AnnouncePointerExpired_BombList, 0x6)
{
	GET(bool, removed, EDI);
	// Skip the call of BombListClass::PointerExpired if !removed.
	return removed ? 0 : 0x72596C;
}

// Changed the bRemoved arg in AbstractClass::AnnounceExpiredPointer calling.
// It should be false in most case.
namespace Disappear
{
	bool removed = false;
}

DEFINE_HOOK_AGAIN(0x6FCD95, SetDisappearContext, 0x6); // TechnoClass_PreUninit
DEFINE_HOOK(0x5F57A9, SetDisappearContext, 0x6) // ObjectClass_ReceiveDamage_NowDead
{
	Disappear::removed = true;
	return 0;
}

DEFINE_HOOK(0x5F530B, ObjectClass_Disappear_AnnounceExpiredPointer, 0x6)
{
	GET(ObjectClass*, pThis, ESI);
	GET_STACK(bool, removed, STACK_OFFSET(0x8, 0x4));
	R->ECX(pThis);
	// Do not working for buildings for now, because it will break some vanilla building tracking.
	// Hoping someone could investigate thoroughly and enable it for buildings.
	R->EDX(((pThis->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None) ? Disappear::removed : removed);
	Disappear::removed = false;
	return 0x5F5311;
}

// I think no one wants to see wild pointers caused by WW's negligence
DEFINE_HOOK(0x4D9A1B, FootClass_PointerExpired_RemoveDestination, 0x6)
{
	GET_STACK(bool, removed, STACK_OFFSET(0x1C, 0x8));

	if (removed)
		return 0x4D9ABD;

	R->BL(true);
	return 0x4D9A25;
}

namespace RemoveSpawneeHelper
{
	bool removed = false;
}

DEFINE_HOOK(0x707B23, TechnoClass_PointerExpired_RemoveSpawnee, 0x6)
{
	GET(SpawnManagerClass*, pSpawnManager, ECX);
	GET(AbstractClass*, pRemove, EBP);
	GET_STACK(bool, removed, STACK_OFFSET(0x20, 0x8));

	RemoveSpawneeHelper::removed = removed;
	pSpawnManager->UnlinkPointer(pRemove);
	RemoveSpawneeHelper::removed = false;

	return 0x707B29;
}

DEFINE_HOOK(0x6B7CE4, SpawnManagerClass_UnlinkPointer_RemoveSpawnee, 0x6)
{
	return RemoveSpawneeHelper::removed ? 0x6B7CF4 : 0;
}

#pragma endregion

// IsSonic wave drawing uses fixed-size arrays accessed with index that is determined based on factors like wave lifetime,
// distance of pixel from start coords etc. The result is that at certain distance invalid memory is being accessed leading to crashes.
// Easiest solution to this is simply clamping the final color index so that no memory beyond the size 14 color data buffer in WaveClass
// is being accessed with it. Last index of color data is uninitialized, changing that or trying to access it just results in glitchy behaviour
// so the cutoff is at 12 here instead of 13.
DEFINE_HOOK(0x75EE49, WaveClass_DrawSonic_CrashFix, 0x7)
{
	GET(int, colorIndex, EAX);

	if (colorIndex > 12)
		R->EAX(12);

	return 0;
}

// WW used SetDesired here, causing the barrel drawn incorrectly.
DEFINE_HOOK(0x6F6DEE, TechnoClass_Unlimbo_BarrelFacingBugFix, 0x7)
{
	enum { SkipGameCode = 0x6F6DFA };

	GET(DirStruct*, pDir, ECX);
	GET(TechnoClass*, pThis, ESI);

	pThis->BarrelFacing.SetCurrent(*pDir);

	return SkipGameCode;
}

namespace BulletDrawVoxelTemp
{
	ConvertClass* Convert = nullptr;
}

DEFINE_HOOK(0x46B19B, BulletClass_DrawVoxel_GetLightConvert, 0x6)
{
	GET(BulletClass*, pThis, EAX);

	if (pThis->Type->AnimPalette)
	{
		BulletDrawVoxelTemp::Convert = FileSystem::ANIM_PAL;
	}
	else if (pThis->Type->FirersPalette)
	{
		const int inheritColor = pThis->InheritedColor;
		const int colorIndex = inheritColor == -1 ? HouseClass::CurrentPlayer->ColorSchemeIndex : inheritColor;
		BulletDrawVoxelTemp::Convert = ColorScheme::Array.Items[colorIndex]->LightConvert;
	}

	return 0;
}

DEFINE_HOOK(0x46B212, BulletClass_DrawVoxel_SetLightConvert, 0x6)
{
	enum { SkipGameCode = 0x46B218 };

	const auto pConvert = BulletDrawVoxelTemp::Convert;

	if (!pConvert)
		return 0;

	R->ECX(pConvert);
	return SkipGameCode;
}

DEFINE_HOOK(0x46B23C, BulletClass_DrawVoxel_SetLightConvert2, 0x6)
{
	enum { SkipGameCode = 0x46B242 };

	const auto pConvert = BulletDrawVoxelTemp::Convert;

	if (!pConvert)
		return 0;

	BulletDrawVoxelTemp::Convert = nullptr;
	R->ECX(pConvert);
	return SkipGameCode;
}

#pragma region StructureFindingFix

// These functions should consider reachablity.
DEFINE_HOOK(0x4DFC39, FootClass_FindBioReactor_CheckValid, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	return pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x6;
}

DEFINE_HOOK(0x4DFED2, FootClass_FindGarrisonStructure_CheckValid, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x6;
}

DEFINE_HOOK(0x4E0024, FootClass_FindTankBunker_CheckValid, 0x8)
{
	GET(FootClass*, pThis, EDI);
	GET(BuildingClass*, pBuilding, ESI);

	return pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x8;
}

DEFINE_HOOK(0x4DFD92, FootClass_FindBattleBunker_CheckValid, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x8;
}

DEFINE_HOOK(0x4DFB28, FootClass_FindGrinder_CheckValid, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return pThis->IsInSameZoneAs(pBuilding) ? 0 : R->Origin() + 0x8;
}

#pragma endregion

DEFINE_HOOK_AGAIN(0x73A1D3, FootClass_UpdatePosition_EnterGrinderSound, 0x6)// UnitClass
DEFINE_HOOK(0x5198C3, FootClass_UpdatePosition_EnterGrinderSound, 0x6)// InfantryClass
{
	GET(BuildingClass*, pGrinder, EBX);
	const int enterSound = pGrinder->Type->EnterGrinderSound;

	if (enterSound >= 0)
	{
		R->ECX(enterSound);
		return R->Origin() + 0x6;
	}

	return 0;
}

DEFINE_HOOK(0x51A304, InfantryClass_UpdatePosition_EnterBioReactorSound, 0x6)
{
	enum { SkipGameCode = 0x51A30A };

	GET(BuildingClass*, pReactor, EDI);
	const int enterSound = pReactor->Type->EnterBioReactorSound;

	if (enterSound >= 0)
	{
		R->ECX(enterSound);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x44DBCF, BuildingClass_Mission_Unload_LeaveBioReactorSound, 0x6)
{
	enum { SkipGameCode = 0x44DBD5 };

	GET(BuildingClass*, pReactor, EBP);
	const int leaveSound = pReactor->Type->LeaveBioReactorSound;

	if (leaveSound >= 0)
	{
		R->ECX(leaveSound);
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x51A2AD, InfantryClass_UpdatePosition_EnterBuilding_CheckSize, 0x9)
{
	enum { CannotEnter = 0x51A4BF };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pDestination, EDI);

	return pDestination->Passengers.NumPassengers + 1 <= pDestination->Type->Passengers && static_cast<int>(pThis->GetTechnoType()->Size) <= pDestination->Type->SizeLimit ? 0 : CannotEnter;
}

DEFINE_HOOK(0x710352, FootClass_ImbueLocomotor_ResetUnloadingHarvester, 0x7)
{
	GET(FootClass*, pTarget, ESI);

	if (const auto pUnit = abstract_cast<UnitClass*>(pTarget))
		pUnit->Unloading = false;

	return 0;
}

DEFINE_JUMP(LJMP, 0x73C41B, 0x73C431)

DEFINE_HOOK(0x73C43F, UnitClass_DrawAsVXL_Shadow_IsLocomotorFix2, 0x6)
{
	enum { SkipGameCode = 0x73C445 };

	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, EAX);

	R->AL(pType->BalloonHover || pThis->IsAttackedByLocomotor);
	return SkipGameCode;
}

namespace RemoveCellContentTemp
{
	bool CheckBeforeUnmark = false;
}

DEFINE_HOOK(0x737F74, UnitClass_ReceiveDamage_NowDead_MarkUp, 0x6)
{
	enum { SkipGameCode = 0x737F80 };

	GET(UnitClass*, pThis, ESI);

	RemoveCellContentTemp::CheckBeforeUnmark = true;
	pThis->Mark(MarkType::Up);
	RemoveCellContentTemp::CheckBeforeUnmark = false;

	return SkipGameCode;
}

DEFINE_HOOK(0x47EAF7, CellClass_RemoveContent_BeforeUnmarkOccupationBits, 0x7)
{
	enum { ContinueCheck = 0x47EAFE, DontUnmark = 0x47EB8F };

	GET(CellClass*, pCell, EDI);
	GET_STACK(bool, onBridge, STACK_OFFSET(0x14, 0x8));

	if (RemoveCellContentTemp::CheckBeforeUnmark && (onBridge ? pCell->AltObject : pCell->FirstObject))
		return DontUnmark;

	GET(ObjectClass*, pContent, ESI);
	R->EAX(pContent->WhatAmI());
	return ContinueCheck;
}

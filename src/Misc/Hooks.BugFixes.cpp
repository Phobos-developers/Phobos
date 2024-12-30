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
	auto const pType = BuildingTypeClass::Array->Items[idxBld];
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
	bool isWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	enum { Attachable = 0x480549 };

	if (isWall)
	{
		for (auto pObject = pThis->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->Health > 0)
			{
				if (auto pBuilding = abstract_cast<BuildingClass*>(pObject))
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

	auto pWeapon = SuperWeaponTypeClass::Array->GetItem(pThis->FiringSWType)->WeaponType;
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
	auto nLandType = MapClass::Instance()->GetCellAt(nCoord)->LandType;

	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, nLandType, nCoord))
	{
		auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, 0x2600, -15, false);

		AnimExt::SetAnimOwnerHouseKind(pAnim, pThisBomb->OwnerHouse,
			pThisBomb->Target ? pThisBomb->Target->GetOwningHouse() : nullptr, false);

		if (!pAnim->Owner)
		{
			pAnim->Owner = pThisBomb->OwnerHouse;
		}

		if (const auto pExt = AnimExt::ExtMap.Find(pAnim))
			pExt->SetInvoker(pThisBomb->Owner);

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
DEFINE_JUMP(CALL, 0x4387A3, GET_OFFSET(_BombClass_Detonate_DamageArea));

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
	enum { SkipGameCode = 0x741086, MustDeploy = 0x7410A8 };

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


bool __fastcall BuildingClass_SetOwningHouse_Wrapper(BuildingClass* pThis, void*, HouseClass* pHouse, bool announce)
{
	// Fix : Suppress capture EVA event if ConsideredVehicle=yes
	if(announce) announce = !pThis->IsStrange();

	bool res = reinterpret_cast<bool(__thiscall*)(BuildingClass*, HouseClass*, bool)>(0x448260)(pThis, pHouse, announce);

	// Fix : update powered anims
	if (res && (pThis->Type->Powered || pThis->Type->PoweredSpecial))
		reinterpret_cast<void(__thiscall*)(BuildingClass*)>(0x4549B0)(pThis);
	return res;
}

DEFINE_JUMP(VTABLE, 0x7E4290, GET_OFFSET(BuildingClass_SetOwningHouse_Wrapper));
DEFINE_JUMP(LJMP, 0x6E0BD4, 0x6E0BFE);
DEFINE_JUMP(LJMP, 0x6E0C1D, 0x6E0C8B);//Simplify TAction 36

// Fix a glitch related to incorrect target setting for missiles
// Author: Belonit
DEFINE_HOOK(0x6B75AC, SpawnManagerClass_AI_SetDestinationForMissiles, 0x5)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(TechnoClass*, pSpawnTechno, EDI);

	CoordStruct coord = pSpawnManager->Target->GetCenterCoords();
	CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord);

	pSpawnTechno->SetDestination(pCellDestination, true);

	return 0x6B75BC;
}

DEFINE_HOOK(0x689EB0, ScenarioClass_ReadMap_SkipHeaderInCampaign, 0x6)
{
	return SessionClass::IsCampaign() ? 0x689FC0 : 0;
}

#pragma region save_load

//Skip incorrect load ctor call in various LocomotionClass_Load
DEFINE_JUMP(LJMP, 0x719CBC, 0x719CD8);//Teleport, notorious CLEG frozen state removal on loading game
DEFINE_JUMP(LJMP, 0x72A16A, 0x72A186);//Tunnel, not a big deal
DEFINE_JUMP(LJMP, 0x663428, 0x663445);//Rocket, not a big deal
DEFINE_JUMP(LJMP, 0x5170CE, 0x5170E0);//Hover, not a big deal
DEFINE_JUMP(LJMP, 0x65B3F7, 0x65B416);//RadSite, no effect

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

	auto const bounds = MapClass::Instance->MapCoordBounds;
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

DEFINE_HOOK(0x7295C5, TunnelLocomotionClass_ProcessDigging_SlowdownDistance, 0x9)
{
	enum { KeepMoving = 0x72980F, CloseEnough = 0x7295CE };

	GET(TunnelLocomotionClass* const, pLoco, ESI);
	GET(int const, distance, EAX);

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

	TunnelLocomotionClass::TunnelMovementSpeed = speed;

	return distance >= speed + 1 ? KeepMoving : CloseEnough;
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

#pragma region StopEventFix

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
		// Check Jumpjets
		const auto pFoot = abstract_cast<FootClass*>(pTechno);
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

	return SkipGameCode;
}

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
DEFINE_JUMP(CALL, 0x6E8305, GET_OFFSET(HexStr2Int_replacement)); // TaskForce
DEFINE_JUMP(CALL, 0x6E5FA6, GET_OFFSET(HexStr2Int_replacement)); // TagType

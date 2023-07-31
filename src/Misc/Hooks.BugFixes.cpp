#include <AircraftClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <TechnoClass.h>
#include <InfantryClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <VoxelAnimClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <FlyLocomotionClass.h>
#include <JumpjetLocomotionClass.h>
#include <BombClass.h>
#include <WarheadTypeClass.h>
#include <Ext/Rules/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Utilities/TemplateDef.h>

//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
DEFINE_HOOK(0x423365, Phobos_BugFixes_SHPShadowCheck, 0x8)
{
	GET(AnimClass*, pAnim, ESI);
	return (pAnim->Type->Shadow && pAnim->HasExtras) ?
		0x42336D :
		0x4233EE;
}

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

	if (pBuildingType->Naval || pBuildingType->SpeedType == SpeedType::Float)
		return IsNaval;

	return NotNaval;
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
		if (VTable::Get(pThis->TemporalTargetingMe) == 0x7F5180) // TemporalClass::`vtable`
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

	if (pThis->Type->Locomotor == LocomotionClass::CLSIDs::Fly)
	{
		if (const auto pLocomotor = static_cast<FlyLocomotionClass*>(pThis->Locomotor.GetInterfacePtr()))
		{
			double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
				TechnoExt::GetCurrentSpeedMultiplier(pThis);

			R->EAX(static_cast<int>(currentSpeed));
		}
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
	auto nDamageAreaResult = MapClass::Instance()->DamageArea
	(nCoord, nDamage, pSource, pWarhead, pWarhead->Tiberium, pThisBomb->OwnerHouse);
	auto nLandType = MapClass::Instance()->GetCellAt(nCoord)->LandType;

	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, nLandType, nCoord))
	{
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, 0x2600, -15, false))
		{
			if (AnimTypeExt::ExtMap.Find(pAnim->Type)->CreateUnit.Get())
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThisBomb->OwnerHouse,
					pThisBomb->Target ? pThisBomb->Target->GetOwningHouse() : nullptr, false);
			}
			else
			{
				pAnim->Owner = pThisBomb->OwnerHouse;
			}

			if (const auto pExt = AnimExt::ExtMap.Find(pAnim))
				pExt->SetInvoker(pThisBomb->Owner);
		}
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

// Fix railgun target coordinates potentially differing from actual target coords.
DEFINE_HOOK(0x70C6B5, TechnoClass_Railgun_TargetCoords, 0x5)
{
	GET(AbstractClass*, pTarget, EBX);

	auto coords = pTarget->GetCenterCoords();

	if (const auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
		coords = pBuilding->GetTargetCoords();
	else if (const auto pCell = abstract_cast<CellClass*>(pTarget))
		coords = pCell->GetCoordsWithBridge();

	R->EAX(&coords);
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

DEFINE_HOOK(0x56BD8B, MapClass_PlaceRandomCrate_Sampling, 0x5)
{
	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };

	int XP = 2 * MapClass::Instance->VisibleRect.X - MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance->VisibleRect.Width);
	int YP = 2 * MapClass::Instance->VisibleRect.Y + MapClass::Instance->MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance->VisibleRect.Height + 2);
	CellStruct candidate { (short)((XP + YP) / 2),(short)((YP - XP) / 2) };

	auto pCell = MapClass::Instance->TryGetCellAt(candidate);
	if (!pCell)
		return SkipSpawn;

	if (!MapClass::Instance->IsWithinUsableArea(pCell, true))
		return SkipSpawn;

	bool isWater = pCell->LandType == LandType::Water;
	if (isWater && RulesExt::Global()->CrateOnlyOnLand.Get())
		return SkipSpawn;

	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));
	cell = MapClass::Instance->NearByLocation(pCell->MapCoords,
		isWater ? SpeedType::Float : SpeedType::Track,
		-1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

	R->EAX(&cell);

	return SpawnCrate;
}

// Enable sorted add for Air/Top layers to fix issues with attached anims etc.
DEFINE_HOOK(0x4A9750, DisplayClass_Submit_LayerSort, 0x9)
{
	GET(Layer, layer, EDI);

	R->ECX(layer != Layer::Surface && layer != Layer::Underground);

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

// Only first half of the colorschemes array gets adjusted thanks to the count being wrong, quick and dirty fix.
DEFINE_HOOK(0x53AD97, IonStormClass_AdjustLighting_ColorCount, 0x6)
{
	GET(int, colorSchemesCount, EAX);

	R->EAX(colorSchemesCount * 2);

	return 0;
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

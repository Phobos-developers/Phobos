#include "Body.h"

#include <SpawnManagerClass.h>
#include <TunnelLocomotionClass.h>

#pragma region SlaveManagerClass

// Issue #601
// Author : TwinkleStar
DEFINE_HOOK(0x6B0C2C, SlaveManagerClass_FreeSlaves_SlavesFreeSound, 0x5)
{
	GET(TechnoClass*, pSlave, EDI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->GetTechnoType());
	int sound = pTypeExt->SlavesFreeSound.Get(RulesClass::Instance()->SlavesFreeSound);
	if (sound != -1)
		VocClass::PlayAt(sound, pSlave->Location);

	return 0x6B0C65;
}

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6)
{
	enum { KillTheSlave = 0x6B0BDF, ChangeSlaveOwner = 0x6B0BB4 };

	GET(InfantryClass*, pSlave, ESI);

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->Type))
	{
		switch (pTypeExt->Slaved_OwnerWhenMasterKilled.Get())
		{
		case SlaveChangeOwnerType::Suicide:
			return KillTheSlave;

		case SlaveChangeOwnerType::Master:
			R->EAX(pSlave->Owner);
			return ChangeSlaveOwner;

		case SlaveChangeOwnerType::Neutral:
			if (auto pNeutral = HouseClass::FindNeutral())
			{
				R->EAX(pNeutral);
				return ChangeSlaveOwner;
			}

		default: // SlaveChangeOwnerType::Killer
			return 0x0;
		}
	}

	return 0x0;
}

// Fix slaves cannot always suicide due to armor multiplier or something
DEFINE_PATCH(0x6B0BF7,
	0x6A, 0x01  // push 1       // ignoreDefense=false->true
);

#pragma endregion

#pragma region SpawnManagerClass

DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching && pThis->CountDockedSpawns() != 0)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset())
				R->EAX(std::min(pTypeExt->Spawner_DelayFrames.Get(), 10));
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B73BE, SpawnManagerClass_AI_SpawnTimer, 0x6)
DEFINE_HOOK(0x6B73AD, SpawnManagerClass_AI_SpawnTimer, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset())
				R->ECX(pTypeExt->Spawner_DelayFrames.Get());
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B769F, SpawnManagerClass_AI_InitDestination, 0x7)
DEFINE_HOOK(0x6B7600, SpawnManagerClass_AI_InitDestination, 0x6)
{
	enum { SkipGameCode1 = 0x6B760E, SkipGameCode2 = 0x6B76DE };

	GET(SpawnManagerClass* const, pThis, ESI);
	GET(AircraftClass* const, pSpawnee, EDI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

	if (pTypeExt->Spawner_AttackImmediately)
	{
		pSpawnee->SetTarget(pThis->Target);
		pSpawnee->QueueMission(Mission::Attack, true);
		pSpawnee->IsReturningFromAttackRun = false;
	}
	else
	{
		auto const mapCoords = pThis->Owner->GetMapCoords();
		auto const pCell = MapClass::Instance->GetCellAt(mapCoords);
		pSpawnee->SetDestination(pCell->GetNeighbourCell(FacingType::North), true);
		pSpawnee->QueueMission(Mission::Move, false);
	}

	return R->Origin() == 0x6B7600 ? SkipGameCode1 : SkipGameCode2;
}

DEFINE_HOOK(0x6B7282, SpawnManagerClass_AI_PromoteSpawns, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	if (pTypeExt->Promote_IncludeSpawns)
	{
		for (auto i : pThis->SpawnedNodes)
		{
			if (i->Unit && i->Unit->Veterancy.Veterancy < pThis->Owner->Veterancy.Veterancy)
				i->Unit->Veterancy.Add(pThis->Owner->Veterancy.Veterancy - i->Unit->Veterancy.Veterancy);
		}
	}

	return 0;
}

#pragma endregion

#pragma region WakeAnims

DEFINE_HOOK_AGAIN(0x69FEDC, Locomotion_Process_Wake, 0x6)  // Ship
DEFINE_HOOK_AGAIN(0x4B0814, Locomotion_Process_Wake, 0x6)  // Drive
DEFINE_HOOK(0x514AB4, Locomotion_Process_Wake, 0x6)  // Hover
{
	GET(ILocomotion* const, iloco, ESI);
	__assume(iloco != nullptr);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(static_cast<LocomotionClass*>(iloco)->LinkedTo->GetTechnoType());
	R->EDX(pTypeExt->Wake.Get(RulesClass::Instance->Wake));

	return R->Origin() + 0xC;
}

namespace GrappleUpdateTemp
{
	TechnoClass* pThis;
}

DEFINE_HOOK(0x629E9B, ParasiteClass_GrappleUpdate_MakeWake_SetContext, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	GrappleUpdateTemp::pThis = pThis;

	return 0;
}

DEFINE_HOOK(0x629FA3, ParasiteClass_GrappleUpdate_MakeWake, 0x6)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(GrappleUpdateTemp::pThis->GetTechnoType());
	R->EDX(pTypeExt->Wake_Grapple.Get(pTypeExt->Wake.Get(RulesClass::Instance->Wake)));

	return 0x629FA9;
}

DEFINE_HOOK(0x7365AD, UnitClass_Update_SinkingWake, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->ECX(pTypeExt->Wake_Sinking.Get(pTypeExt->Wake.Get(RulesClass::Instance->Wake)));

	return 0x7365B3;
}

DEFINE_HOOK(0x737F05, UnitClass_ReceiveDamage_SinkingWake, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->ECX(pTypeExt->Wake_Sinking.Get(pTypeExt->Wake.Get(RulesClass::Instance->Wake)));

	return 0x737F0B;
}

#pragma endregion

#pragma region TunnelLocomotionClass

DEFINE_HOOK(0x728F74, TunnelLocomotionClass_Process_KillAnims, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	const auto pExt = TechnoExt::ExtMap.Find(pLoco->LinkedTo);
	pExt->IsBurrowed = true;

	if (const auto pShieldData = pExt->Shield.get())
		pShieldData->SetAnimationVisibility(false);

	for (auto const& attachEffect : pExt->AttachedEffects)
	{
		attachEffect->SetAnimationTunnelState(false);
	}

	return 0;
}

DEFINE_HOOK(0x728E5F, TunnelLocomotionClass_Process_RestoreAnims, 0x7)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (pLoco->State == TunnelLocomotionClass::State::PreDigOut)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pLoco->LinkedTo);
		pExt->IsBurrowed = false;

		if (const auto pShieldData = pExt->Shield.get())
			pShieldData->SetAnimationVisibility(true);

		for (auto const& attachEffect : pExt->AttachedEffects)
		{
			attachEffect->SetAnimationTunnelState(true);
		}
	}

	return 0;
}

DEFINE_HOOK(0x728F89, TunnelLocomotionClass_Process_SubterraneanHeight1, 0x5)
{
	enum { Skip = 0x728FA6, Continue = 0x728F90 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

	if (height == pTypeExt->SubterraneanHeight.Get(RulesExt::Global()->SubterraneanHeight))
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x728FC6, TunnelLocomotionClass_Process_SubterraneanHeight2, 0x5)
{
	enum { Skip = 0x728FCD, Continue = 0x729021 };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, height, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

	if (height <= pTypeExt->SubterraneanHeight.Get(RulesExt::Global()->SubterraneanHeight))
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x728FF2, TunnelLocomotionClass_Process_SubterraneanHeight3, 0x6)
{
	enum { SkipGameCode = 0x72900C };

	GET(TechnoClass*, pLinkedTo, ECX);
	GET(int, heightOffset, EAX);
	REF_STACK(int, height, 0x14);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());
	int subtHeight = pTypeExt->SubterraneanHeight.Get(RulesExt::Global()->SubterraneanHeight);
	height -= heightOffset;

	if (height < subtHeight)
		height = subtHeight;

	return SkipGameCode;
}

DEFINE_HOOK(0x7295E2, TunnelLocomotionClass_ProcessStateDigging_SubterraneanHeight, 0xC)
{
	enum { SkipGameCode = 0x7295EE };

	GET(TechnoClass*, pLinkedTo, EAX);
	REF_STACK(int, height, STACK_OFFSET(0x44, -0x8));

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());
	height = pTypeExt->SubterraneanHeight.Get(RulesExt::Global()->SubterraneanHeight);

	return SkipGameCode;
}

#pragma endregion

#pragma region Disguise

DEFINE_HOOK(0x522790, InfantryClass_ClearDisguise_DefaultDisguise, 0x6)
{
	GET(InfantryClass*, pThis, ECX);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->DefaultDisguise)
	{
		pThis->Disguise = pExt->DefaultDisguise;
		pThis->Disguised = true;
		return 0x5227BF;
	}

	pThis->Disguised = false;

	return 0;
}

DEFINE_HOOK(0x74691D, UnitClass_UpdateDisguise_EMP, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	// Remove mirage disguise if under emp or being flipped, approximately 15 deg
	if (pThis->IsUnderEMP() || std::abs(pThis->AngleRotatedForwards) > 0.25 || std::abs(pThis->AngleRotatedSideways) > 0.25)
	{
		pThis->ClearDisguise();
		R->EAX(pThis->MindControlRingAnim);
		return 0x746AA5;
	}

	return 0x746931;
}

#pragma endregion

#pragma region Misc

// I must not regroup my forces.
DEFINE_HOOK(0x739920, UnitClass_TryToDeploy_DisableRegroupAtNewConYard, 0x6)
{
	enum { SkipRegroup = 0x73992B, DoNotSkipRegroup = 0 };

	auto const pRules = RulesExt::Global();

	return pRules->GatherWhenMCVDeploy ? DoNotSkipRegroup : SkipRegroup;
}

DEFINE_HOOK(0x736234, UnitClass_ChronoSparkleDelay, 0x5)
{
	R->ECX(RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x736239;
}

DEFINE_HOOK(0x51BAFB, InfantryClass_ChronoSparkleDelay, 0x5)
{
	R->ECX(RulesExt::Global()->ChronoSparkleDisplayDelay);
	return 0x51BB00;
}

DEFINE_HOOK_AGAIN(0x5F4718, ObjectClass_Select, 0x7)
DEFINE_HOOK(0x5F46AE, ObjectClass_Select, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	pThis->IsSelected = true;

	if (RulesExt::Global()->SelectionFlashDuration > 0 && pThis->GetOwningHouse()->IsControlledByCurrentPlayer())
		pThis->Flash(RulesExt::Global()->SelectionFlashDuration);

	return 0;
}

DEFINE_HOOK(0x51B20E, InfantryClass_AssignTarget_FireOnce, 0x6)
{
	enum { SkipGameCode = 0x51B255 };

	GET(InfantryClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pTarget && pExt->SkipTargetChangeResetSequence)
	{
		pThis->IsFiring = false;
		pExt->SkipTargetChangeResetSequence = false;
		return SkipGameCode;
	}

	return 0;
}

// Update attached anim layers after parent unit changes layer.
void __fastcall DisplayClass_Submit_Wrapper(DisplayClass* pThis, void* _, ObjectClass* pObject)
{
	pThis->Submit(pObject);

	if (auto const pTechno = abstract_cast<TechnoClass*>(pObject))
		TechnoExt::UpdateAttachedAnimLayers(pTechno);
}

DEFINE_JUMP(CALL, 0x54B18E, GET_OFFSET(DisplayClass_Submit_Wrapper));  // JumpjetLocomotionClass_Process
DEFINE_JUMP(CALL, 0x4CD4E7, GET_OFFSET(DisplayClass_Submit_Wrapper));  // FlyLocomotionClass_Update

// Fixes SecondaryFire / SecondaryProne sequences not remapping to WetAttack in water.
// Ideally there would be WetAttackSecondary but adding new sequences would be a big undertaking.
// Also adds a toggle for not using water sequences at all - Starkku
DEFINE_HOOK(0x51D7E0, InfantryClass_DoAction_Water, 0x5)
{
	enum { Continue= 0x51D7EC, SkipWaterSequences = 0x51D842, UseSwim = 0x51D83D, UseWetAttack = 0x51D82F };

	GET(InfantryClass*, pThis, ESI);
	GET(Sequence, sequence, EDI);

	R->EBP(0); // Restore overridden instructions.

	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->OnlyUseLandSequences)
		return SkipWaterSequences;

	if (sequence == Sequence::Walk || sequence == Sequence::Crawl) // Restore overridden instructions.
		return UseSwim;
	else if (sequence == Sequence::SecondaryFire || sequence == Sequence::SecondaryProne)
		return UseWetAttack;

	return Continue;
}

#pragma endregion

#include <GameOptionsClass.h>
#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Utilities/AresFunctions.h>
#include <Utilities/Macro.h>

static bool CheckRestrictions(FootClass* pUnit, bool isDeploying)
{
	// Movement restrictions.
	if (isDeploying && pUnit->Locomotor->Is_Moving_Now())
		return true;

	FacingClass* currentDir = &pUnit->PrimaryFacing;
	bool isJumpjet = false;

	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
	{
		// Jumpjet rotating is basically half a guarantee it is also moving and
		// may not be caught by the Is_Moving_Now() check.
		if (isDeploying && pJJLoco->LocomotionFacing.IsRotating())
			return true;

		currentDir = &pJJLoco->LocomotionFacing;
		isJumpjet = true;
	}

	// Facing restrictions.
	auto const pType = pUnit->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto const defaultFacing = (FacingType)(RulesClass::Instance->DeployDir >> 5);
	auto const facing = pTypeExt->DeployDir.Get(defaultFacing);

	if (facing == FacingType::None)
		return false;

	if (facing != (FacingType)currentDir->Current().GetFacing<8>())
	{
		auto dir = DirStruct();
		dir.SetFacing<8>((size_t)facing);

		if (isDeploying)
		{
			static_cast<UnitClass*>(pUnit)->Deploying = true;

			if (isJumpjet)
				currentDir->SetDesired(dir);

			pUnit->Locomotor->Do_Turn(dir);

			return true;
		}
		else
		{
			currentDir->SetDesired(dir);
		}
	}

	return false;
}

static bool HasDeployingAnim(UnitTypeClass* pUnitType)
{
	return pUnitType->DeployingAnim || TechnoTypeExt::ExtMap.Find(pUnitType)->DeployingAnims.size() > 0;
}

static void CreateDeployingAnim(UnitClass* pUnit, bool isDeploying)
{
	if (!pUnit->DeployAnim)
	{
		auto const pType = pUnit->Type;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		auto pAnimType = pUnit->Type->DeployingAnim;

		if (pTypeExt->DeployingAnims.size() > 0)
			pAnimType = GeneralUtils::GetItemForDirection<AnimTypeClass*>(pTypeExt->DeployingAnims, pUnit->PrimaryFacing.Current());

		auto const pAnim = GameCreate<AnimClass>(pAnimType, pUnit->Location, 0, 1, 0x600, 0,
			!isDeploying && pTypeExt->DeployingAnim_ReverseForUndeploy);

		pUnit->DeployAnim = pAnim;
		pAnim->SetOwnerObject(pUnit);
		AnimExt::SetAnimOwnerHouseKind(pAnim, pUnit->Owner, nullptr, false, true);
		AnimExt::ExtMap.Find(pAnim)->SetInvoker(pUnit);
		auto const pExt = TechnoExt::ExtMap.Find(pUnit);

		if (pTypeExt->DeployingAnim_UseUnitDrawer)
		{
			pAnim->LightConvert = pUnit->GetDrawer();
			pAnim->IsBuildingAnim = true; // Hack to make it use tint in drawing code.
		}

		// Set deploy animation timer. Required to be independent from animation lifetime due
		// to processing order / pointer invalidation etc. adding additional delay - simply checking
		// if the animation is still there wouldn't work well as it'd lag one frame behind I believe.
		int rate = pAnimType->Rate;

		if (pAnimType->Normalized)
			rate = GameOptionsClass::Instance.GetAnimSpeed(rate);

		auto& timer = pExt->SimpleDeployerAnimationTimer;

		if (pAnimType->Reverse || pAnim->Reverse)
			timer.Start(pAnim->Animation.Value * rate);
		else
			timer.Start(pAnimType->End * rate);
	}
}

// Full function reimplementation.
DEFINE_HOOK(0x739AC0, UnitClass_SimpleDeployer_Deploy, 0x6)
{
	enum { ReturnFromFunction = 0x739CC3 };

	GET(UnitClass*, pThis, ECX);

	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return ReturnFromFunction;

	if (!pThis->Deployed)
	{
		if (!pThis->InAir && pType->DeployToLand && pThis->GetHeight() > 0)
			pThis->InAir = true;

		if (pThis->Deploying && pThis->DeployAnim)
		{
			auto const pExt = TechnoExt::ExtMap.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = true;
				pThis->Deploying = false;
			}
		}
		else if (!pThis->InAir)
		{
			if (CheckRestrictions(pThis, true))
				return ReturnFromFunction;

			if (HasDeployingAnim(pType))
			{
				CreateDeployingAnim(pThis, true);
				pThis->Deploying = true;
			}
			else
			{
				pThis->Deployed = true;
				pThis->Deploying = false; // DeployDir != -1 + no DeployingAnim case needs this reset here.
			}
		}
	}

	if (pThis->Deployed)
	{
		int maxAmmo = -1;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		if (AresFunctions::ConvertTypeTo && pTypeExt->Convert_Deploy)
			maxAmmo = pTypeExt->Convert_Deploy->Ammo;

		TechnoExt::HandleOnDeployAmmoChange(pThis, maxAmmo);

		if (pType->DeploySound != -1)
			VocClass::PlayAt(pType->DeploySound, pThis->Location);
	}

	return ReturnFromFunction;
}

// Full function reimplementation.
DEFINE_HOOK(0x739CD0, UnitClass_SimpleDeployer_Undeploy, 0x6)
{
	enum { ReturnFromFunction = 0x739EBD };

	GET(UnitClass*, pThis, ECX);

	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return ReturnFromFunction;

	if (pThis->Deployed)
	{
		if (pThis->Undeploying && pThis->DeployAnim)
		{
			auto const pExt = TechnoExt::ExtMap.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = false;
				pThis->Undeploying = false;
				auto cell = CellStruct::Empty;
				pThis->NearbyLocation(&cell, pThis);
				auto const pCell = MapClass::Instance.GetCellAt(cell);
				pThis->SetDestination(pCell, true);
			}
		}
		else
		{
			if (HasDeployingAnim(pType))
			{
				CreateDeployingAnim(pThis, true);
				pThis->Undeploying = true;
			}
			else
			{
				pThis->Deployed = false;
			}
		}

		if (pThis->IsDisguised())
			pThis->Disguised = false;

		if (!pThis->Deployed)
		{
			TechnoExt::HandleOnDeployAmmoChange(pThis);

			if (pType->UndeploySound != -1)
				VocClass::PlayAt(pType->UndeploySound, pThis->Location);
		}
	}

	return ReturnFromFunction;
}

// Disable Ares Jumpjet DeployDir hook.
DEFINE_PATCH(0x54C767, 0x8B, 0x15, 0xE0, 0x71, 0x88, 0x00);

// Handle DeployDir for Jumpjet vehicles.
DEFINE_HOOK(0x54C76D, JumpjetLocomotionClass_Descending_DeployDir, 0x7)
{
	enum { SkipGameCode = 0x54C7A3 };

	GET(JumpjetLocomotionClass*, pThis, ESI);

	auto const pLinkedTo = pThis->LinkedTo;
	CheckRestrictions(pLinkedTo, false);

	return SkipGameCode;
}

// Disable DeployToLand=no forcing landing when idle due to what appears to be
// a code oversight and no need for DeployToLand=no to work in vanilla game.
DEFINE_HOOK(0x54BED4, JumpjetLocomotionClass_Hovering_DeployToLand, 0x7)
{
	enum { SkipGameCode = 0x54BEE0 };

	GET(JumpjetLocomotionClass*, pThis, ESI);
	GET(FootClass*, pLinkedTo, ECX);

	auto const pType = pLinkedTo->GetTechnoType();

	if (!pType->BalloonHover || pType->DeployToLand)
		pThis->State = JumpjetLocomotionClass::State::Descending;

	pLinkedTo->TryNextPlanningTokenNode();
	return SkipGameCode;
}

// Same as above but at a different state.
DEFINE_HOOK(0x54C2DF, JumpjetLocomotionClass_Cruising_DeployToLand, 0xA)
{
	enum { SkipGameCode = 0x54C4FD };

	GET(JumpjetLocomotionClass*, pThis, ESI);
	GET(FootClass*, pLinkedTo, ECX);

	auto const pType = pLinkedTo->GetTechnoType();

	if (!pType->BalloonHover || pType->DeployToLand)
	{
		pThis->CurrentHeight = 0;
		pThis->State = JumpjetLocomotionClass::State::Descending;
	}

	pLinkedTo->TryNextPlanningTokenNode();
	return SkipGameCode;
}

// Disable Ares hover locomotor bobbing processing DeployToLand hook.
DEFINE_PATCH(0x513EAA, 0xA1, 0xE0, 0x71, 0x88, 0x00);

// Handle DeployToLand for hover locomotor vehicles part #1.
DEFINE_HOOK(0x513EAF, HoverLocomotionClass_ProcessBobbing_DeployToLand1, 0x6)
{
	enum { SkipGameCode = 0x513ECD };

	GET(LocomotionClass*, pThis, ESI);

	if (pThis->LinkedTo->InAir)
		return SkipGameCode;

	return 0;
}

// Do not display hover bobbing when landed during deploying.
DEFINE_HOOK(0x513D2C, HoverLocomotionClass_ProcessBobbing_DeployToLand2, 0x6)
{
	enum { SkipBobbing = 0x513F2A };

	GET(LocomotionClass*, pThis, ECX);

	if (auto const pUnit = abstract_cast<UnitClass*>(pThis->LinkedTo))
	{
		if (pUnit->Deploying && pUnit->Type->DeployToLand)
			return SkipBobbing;
	}

	return 0;
}

// Disable Ares hover locomotor processing DeployToLand hook.
DEFINE_PATCH(0x514A21, 0x8B, 0x16, 0x56, 0xFF, 0x92, 0x80, 0x00, 0x00, 0x00);

// Handle DeployToLand for hover locomotor vehicles part #2.
DEFINE_HOOK(0x514A2A, HoverLocomotionClass_Process_DeployToLand, 0x8)
{
	enum { SkipGameCode = 0x514AC8, Continue = 0x514A32 };

	GET(ILocomotion*, pThis, ESI);
	GET(bool, isMoving, EAX);

	auto const pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;

	if (pLinkedTo->InAir)
	{
		auto const pType = pLinkedTo->GetTechnoType();

		if (pType->DeployToLand)
		{
			auto const landType = pLinkedTo->GetCell()->LandType;

			if (landType == LandType::Water || landType == LandType::Beach)
			{
				pLinkedTo->InAir = false;
				pLinkedTo->QueueMission(Mission::Guard, true);
			}

			if (isMoving)
			{
				pThis->Stop_Moving();
				pLinkedTo->SetDestination(nullptr, true);
			}

			CheckRestrictions(pLinkedTo, false);

			if (pLinkedTo->GetHeight() <= 0)
			{
				pLinkedTo->InAir = false;
				pThis->Mark_All_Occupation_Bits(MarkType::Up);
			}
		}
	}

	// Restore overridden instructions.
	return isMoving ? Continue : SkipGameCode;
}

// Disable Ares hover locomotor move processing DeployToLand hook.
DEFINE_PATCH(0x514DFE, 0x8D, 0x4C, 0x24, 0x10, 0x89, 0x46, 0x1C);

// Handle DeployToLand for hover locomotor vehicles part #3.
DEFINE_HOOK(0x514E05, HoverLocomotionClass_MoveTo_DeployToLand, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	auto const pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;

	if (pLinkedTo->GetTechnoType()->DeployToLand)
		pLinkedTo->InAir = false;

	return 0;
}

// DeployToLand units increment WalkingFramesSoFar on every frame, on hover units this causes weird behaviour with move sounds etc.
DEFINE_HOOK(0x4DA9F3, FootClass_AI_DeployToLand, 0x6)
{
	enum { SkipGameCode = 0x4DAA01 };

	GET(FootClass*, pThis, ESI);

	if (pThis->GetTechnoType()->Locomotor == LocomotionClass::CLSIDs::Hover)
		return SkipGameCode;

	return 0;
}

// Allow keeping unit visible while displaying DeployingAnim.
DEFINE_HOOK(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	enum { Continue = 0x73CF62, DoNotDraw = 0x73D43F };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Deploying || pThis->Undeploying)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		if (pTypeExt->DeployingAnim_KeepUnitVisible || (pThis->Deploying && !pThis->DeployAnim))
			return Continue;

		return DoNotDraw;
	}

	return Continue;
}


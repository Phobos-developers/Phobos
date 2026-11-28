#include "Body.h"

#pragma region Drawing

DEFINE_HOOK(0x73C7AC, UnitClass_DrawAsSHP_DrawTurret_TintFix, 0x6)
{
	enum { SkipDrawCode = 0x73CE00 };

	GET(UnitClass*, pThis, EBP);

	const auto pThisType = pThis->Type;
	const VoxelStruct barrelVoxel = pThisType->BarrelVoxel;

	if (barrelVoxel.VXL && barrelVoxel.HVA)
		return 0;

	GET(UnitTypeClass*, pType, ECX);
	GET(SHPStruct*, pShape, EDI);
	GET(const int, bodyFrameIdx, EBX);
	REF_STACK(Point2D, location, STACK_OFFSET(0x128, 0x4));
	REF_STACK(RectangleStruct, bounds, STACK_OFFSET(0x128, 0xC));
	GET_STACK(const int, extraLight, STACK_OFFSET(0x128, 0x1C));

	const bool tooBigToFitUnderBridge = pType->TooBigToFitUnderBridge
		&& reinterpret_cast<bool(__thiscall*)(TechnoClass*)>(0x703B10)(pThis) && !reinterpret_cast<int(__thiscall*)(TechnoClass*)>(0x703E70)(pThis);
	const int zAdjust = tooBigToFitUnderBridge ? -16 : 0;
	const ZGradient zGradient = tooBigToFitUnderBridge ? ZGradient::Ground : pThis->GetZGradient();

	pThis->Draw_A_SHP(pShape, bodyFrameIdx, &location, &bounds, 0, 256, zAdjust, zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);

	const auto pTurretShape = UnitTypeExt::Fetch(pType)->TurretShape;
	const int StartFrame = pTurretShape ? 0 : (pType->WalkFrames * pType->Facings);

	if (pTurretShape)
		pShape = pTurretShape;

	const auto secondaryDir = pThis->SecondaryFacing.Current();
	const int frameIdx = secondaryDir.GetFacing<32>(4) + StartFrame;

	const auto primaryDir = pThis->PrimaryFacing.Current();
	const double bodyRad = primaryDir.GetRadian<32>();
	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(bodyRad));

	TechnoTypeExt::ApplyTurretOffset(pThisType, &mtx);
	const double turretRad = pType->Turret ? secondaryDir.GetRadian<32>() : bodyRad;
	mtx.RotateZ(static_cast<float>(turretRad - bodyRad));

	const auto res = mtx.GetTranslation();
	const auto offset = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };
	Point2D drawPoint = location + TacticalClass::Instance->CoordsToScreen(offset);

	const bool originalDrawShadow = std::exchange(Game::bDrawShadow, false);
	pThis->Draw_A_SHP(pShape, frameIdx, &drawPoint, &bounds, 0, 256, static_cast<DWORD>(-32), zGradient, 0, extraLight, 0, 0, 0, 0, 0, 0);
	Game::bDrawShadow = originalDrawShadow;
	return SkipDrawCode;
}

DEFINE_HOOK(0x73CCF4, UnitClass_DrawSHP_FacingsB_TurretShape, 0xA)
{
	enum { SkipGameCode = 0x73CD06 };

	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, ECX);

	const auto pTurretShape = UnitTypeExt::Fetch(pType)->TurretShape;
	const int StartFrame = pTurretShape ? 0 : (pType->WalkFrames * pType->Facings);
	const int frameIdx = pThis->SecondaryFacing.Current().GetFacing<32>(4) + StartFrame;

	if (pTurretShape)
		R->EDI(pTurretShape);

	R->ECX(pThis);
	R->EAX(frameIdx);
	return 0x73CD06;
}

#pragma endregion

#pragma region DeployFire

DEFINE_HOOK(0x4C77E4, EventClass_Execute_DeployCommand, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = abstract_cast<UnitClass*>(pThis);

	// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit)
	{
		auto const pType = pUnit->Type;

		if (pType->DeployFire && !pType->IsSimpleDeployer)
		{
			int weaponIndex = -1;
			auto const pWeapon = TechnoExt::GetDeployFireWeapon(pThis, pType, weaponIndex);

			if (pWeapon && pWeapon->FireOnce)
			{
				const auto pExt = UnitExt::Fetch(pUnit);

				if (pExt->DeployFireTimer.HasTimeLeft())
					return DoNotExecute;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x73DCEF, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C };

	GET(UnitClass*, pThis, ESI);

	int weaponIndex = -1;
	auto const pWeapon = TechnoExt::GetDeployFireWeapon(pThis, pThis->Type, weaponIndex);

	if (weaponIndex < 0 || !pWeapon)
	{
		pThis->QueueMission(Mission::Guard, true);
		return SkipGameCode;
	}

	const auto pCell = MapClass::Instance.GetCellAt(pThis->GetMapCoords());
	pThis->SetTarget(pCell);

	if (pThis->GetFireError(pCell, weaponIndex, true) == FireError::OK)
	{
		pThis->Fire(pThis->Target, weaponIndex);

		if (pWeapon->FireOnce)
		{
			pThis->SetTarget(nullptr);
			pThis->QueueMission(Mission::Guard, true);
			const auto pExt = UnitExt::Fetch(pThis);
			const auto UnloadControl = &MissionControlClass::Array[(int)Mission::Unload];
			const int delay = static_cast<int>(UnloadControl->Rate * 900) + ScenarioClass::Instance->Random(0, 2);
			pExt->DeployFireTimer.Start(Math::min(pWeapon->ROF, delay));
		}
	}

	return SkipGameCode;
}

#pragma endregion

#pragma region Sinking

DEFINE_HOOK(0x7364DC, UnitClass_Update_SinkSpeed, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(const int, CoordZ, EDX);

	auto const pTypeExt = UnitTypeExt::Fetch(pThis->Type);
	R->EDX(CoordZ - (pTypeExt->SinkSpeed.Get(RulesExt::Global()->SinkSpeed) - 5));
	return 0;
}

DEFINE_HOOK(0x737DE2, UnitClass_ReceiveDamage_Sinkable, 0x6)
{
	enum { GoOtherChecks = 0x737E18, NoSink = 0x737E63 };

	GET(UnitTypeClass*, pType, EAX);

	auto const pTypeExt = UnitTypeExt::Fetch(pType);
	const bool shouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;

	return pTypeExt->Sinkable.Get(RulesExt::Global()->Sinkable.Get(shouldSink)) ? GoOtherChecks : NoSink;
}

DEFINE_HOOK(0x629C67, ParasiteClass_UpdateSquid_SinkableBySquid, 0x9)
{
	enum { ret = 0x629C86 };

	GET(ParasiteClass*, pThis, ESI);
	GET(FootClass*, pVictim, EDI);

	const auto pVictimExt = TechnoExt::Fetch(pVictim);
	const auto pOwner = pThis->Owner;
	const bool isUnit = pVictim->WhatAmI() == AbstractType::Unit;

	if ((isUnit && static_cast<UnitExt*>(pVictimExt)->GetTypeExtData()->Sinkable_SquidGrab.Get(RulesExt::Global()->Sinkable_SquidGrab)) || !isUnit)
	{
		pVictim->IsSinking = true;
		pVictim->Destroyed(pOwner);
		pVictim->Stun();
	}
	else
	{
		auto damage = pVictimExt->TypeExtData->OwnerObject()->Strength;
		pVictim->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, pOwner, true, false, pOwner->Owner);
	}

	return ret;
}

#pragma endregion

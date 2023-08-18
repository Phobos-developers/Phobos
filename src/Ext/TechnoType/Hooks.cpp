#include <AnimClass.h>
#include <UnitClass.h>
#include <AnimClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SpawnManagerClass.h>
#include <TacticalClass.h>
#include <BulletClass.h>

#include "Body.h"
#include <Ext/AnimType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->HealthBar_Hide)
		return 0x6F6AB6;

	return 0;
}

DEFINE_HOOK(0x6F3C56, TechnoClass_Transform_6F3AD0_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0xD8, -0x90));
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3C6D;
}

DEFINE_HOOK(0x6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0xCC, -0x90));
	GET(TechnoTypeClass*, technoType, EBP);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3E85;
}

DEFINE_HOOK(0x73B780, UnitClass_DrawVXL_TurretMultiOffset, 0x0)
{
	GET(TechnoTypeClass*, technoType, EAX);

	auto const pTypeData = TechnoTypeExt::ExtMap.Find(technoType);

	if (pTypeData && *pTypeData->TurretOffset.GetEx() == CoordStruct { 0, 0, 0 })
		return 0x73B78A;

	return 0x73B790;
}

DEFINE_HOOK(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0x1D0, -0x13C));
	GET(TechnoTypeClass*, technoType, EBX);

	double& factor = *reinterpret_cast<double*>(0xB1D008);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, factor);

	return 0x73BA68;
}

DEFINE_HOOK(0x73C890, UnitClass_Draw_1_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x80);
	GET(TechnoTypeClass*, technoType, EAX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1 / 8);

	return 0x73C8B7;
}

DEFINE_HOOK(0x43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x60);
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1 / 8);

	return 0x43E0E8;
}

DEFINE_HOOK(0x73CCE1, UnitClass_DrawSHP_TurretOffest, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	REF_STACK(Point2D, pos, STACK_OFFSET(0x15C, -0xE8));

	auto mtx = pThis->Locomotor->Draw_Matrix(nullptr);
	TechnoTypeExt::ApplyTurretOffset(pThis->Type, &mtx);

	double turretRad = pThis->TurretFacing().GetRadian<32>();
	double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
	float angle = (float)(turretRad - bodyRad);
	mtx.RotateZ(angle);
	auto res = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);
	auto location = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };
	Point2D temp;
	pos += *TacticalClass::Instance()->CoordsToScreen(&temp, &location);

	return 0;
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

DEFINE_HOOK(0x73D223, UnitClass_DrawIt_OreGath, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nFacing, EDI);
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x50, 0x8));
	LEA_STACK(Point2D*, pLocation, STACK_OFFSET(0x50, -0x18));
	GET_STACK(int, nBrightness, STACK_OFFSET(0x50, 0x4));

	auto const pType = pThis->GetTechnoType();
	auto const pData = TechnoTypeExt::ExtMap.Find(pType);

	ConvertClass* pDrawer = FileSystem::ANIM_PAL;
	SHPStruct* pSHP = FileSystem::OREGATH_SHP;
	int idxFrame;

	auto idxTiberium = pThis->GetCell()->GetContainedTiberiumIndex();
	auto idxArray = pData->OreGathering_Tiberiums.size() > 0 ? pData->OreGathering_Tiberiums.IndexOf(idxTiberium) : 0;
	if (idxTiberium != -1 && idxArray != -1)
	{
		auto const pAnimType = pData->OreGathering_Anims.size() > 0 ? pData->OreGathering_Anims[idxArray] : nullptr;
		auto const nFramesPerFacing = pData->OreGathering_FramesPerDir.size() > 0 ? pData->OreGathering_FramesPerDir[idxArray] : 15;
		auto const pAnimExt = AnimTypeExt::ExtMap.Find(pAnimType);
		if (pAnimType)
		{
			pSHP = pAnimType->GetImage();
			if (auto const pPalette = pAnimExt->Palette.GetConvert())
				pDrawer = pPalette;
		}
		idxFrame = nFramesPerFacing * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % nFramesPerFacing;
	}
	else
	{
		idxFrame = 15 * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % 15;
	}

	DSurface::Temp->DrawSHP(
		pDrawer, pSHP, idxFrame, pLocation, pBounds,
		BlitterFlags::Flat | BlitterFlags::Alpha | BlitterFlags::Centered,
		0, pThis->GetZAdjustment() - 2, ZGradient::Ground, nBrightness,
		0, nullptr, 0, 0, 0
	);

	R->EBP(nBrightness);
	R->EBX(pBounds);

	return 0x73D28C;
}

DEFINE_HOOK(0x700C58, TechnoClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		return pExt->NoManualMove ? 0x700C62 : 0;

	return 0;
}

DEFINE_HOOK(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	enum { KeepUnitVisible = 0x73CF62 };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Deploying || pThis->Undeploying)
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->DeployingAnim_KeepUnitVisible)
			return KeepUnitVisible;
	}

	return 0;
}

// Ares hooks in at 739B8A, this goes before it and skips it if needed.
DEFINE_HOOK(0x739B7C, UnitClass_Deploy_DeployDir, 0x6)
{
	enum { SkipAnim = 0x739C70, PlayAnim = 0x739B9E };

	GET(UnitClass*, pThis, ESI);

	if (!pThis->InAir)
	{
		if (pThis->Type->DeployingAnim)
		{
			if (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->DeployingAnim_AllowAnyDirection)
				return PlayAnim;

			return 0;
		}

		pThis->Deployed = true;
	}

	return SkipAnim;
}

DEFINE_HOOK_AGAIN(0x739D8B, UnitClass_DeployUndeploy_DeployAnim, 0x5)
DEFINE_HOOK(0x739BA8, UnitClass_DeployUndeploy_DeployAnim, 0x5)
{
	enum { Deploy = 0x739C20, DeployUseUnitDrawer = 0x739C0A, Undeploy = 0x739E04, UndeployUseUnitDrawer = 0x739DEE };

	GET(UnitClass*, pThis, ESI);

	bool isDeploying = R->Origin() == 0x739BA8;

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (auto const pAnim = GameCreate<AnimClass>(pThis->Type->DeployingAnim,
			pThis->Location, 0, 1, 0x600, 0,
			!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy : false))
		{
			pThis->DeployAnim = pAnim;
			pAnim->SetOwnerObject(pThis);

			if (pExt->DeployingAnim_UseUnitDrawer)
				return isDeploying ? DeployUseUnitDrawer : UndeployUseUnitDrawer;
		}
		else
		{
			pThis->DeployAnim = nullptr;
		}
	}

	return isDeploying ? Deploy : Undeploy;
}

DEFINE_HOOK_AGAIN(0x739E81, UnitClass_DeployUndeploy_DeploySound, 0x6)
DEFINE_HOOK(0x739C86, UnitClass_DeployUndeploy_DeploySound, 0x6)
{
	enum { DeployReturn = 0x739CBF, UndeployReturn = 0x739EB8 };

	GET(UnitClass*, pThis, ESI);

	bool isDeploying = R->Origin() == 0x739C86;
	bool isDoneWithDeployUndeploy = isDeploying ? pThis->Deployed : !pThis->Deployed;

	if (isDoneWithDeployUndeploy)
		return 0; // Only play sound when done with deploying or undeploying.

	return isDeploying ? DeployReturn : UndeployReturn;
}

// Issue #503
// Author : Otamaa
DEFINE_HOOK(0x4AE670, DisplayClass_GetToolTip_EnemyUIName, 0x8)
{
	enum { SetUIName = 0x4AE678 };

	GET(ObjectClass* , pObject, ECX);

	auto pDecidedUIName = pObject->GetUIName();
	auto pFoot = generic_cast<FootClass*>(pObject);
	auto pTechnoType = pObject->GetTechnoType();

	if (pFoot && pTechnoType && !pObject->IsDisguised())
	{
		bool IsAlly = true;
		bool IsCivilian = false;
		bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();

		if (auto pOwnerHouse = pFoot->GetOwningHouse())
		{
			IsAlly = pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer);
			IsCivilian = (pOwnerHouse == HouseClass::FindCivilianSide()) || pOwnerHouse->IsNeutral();
		}

		if (!IsAlly && !IsCivilian && !IsObserver)
		{
			auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (auto pEnemyUIName = pTechnoTypeExt->EnemyUIName.Get().Text)
			{
				pDecidedUIName = pEnemyUIName;
			}
		}
	}

	R->EAX(pDecidedUIName);
	return SetUIName;
}


// Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
// Fixes recursive passenger kills not being accredited
// to proper techno but to their transports
DEFINE_PATCH(0x707CF2, 0x55);

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

DEFINE_HOOK(0x4DB157, FootClass_DrawVoxelShadow_TurretShadow, 0x8)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(Point2D, pos, STACK_OFFSET(0x18, 0x28));
	GET_STACK(Surface*, pSurface, STACK_OFFSET(0x18, 0x24));
	GET_STACK(bool, a9, STACK_OFFSET(0x18, 0x20)); // unknown usage
	GET_STACK(Matrix3D*, pMatrix, STACK_OFFSET(0x18, 0x1C));
	GET_STACK(Point2D*, a4, STACK_OFFSET(0x18, 0x14)); // unknown usage
	GET_STACK(Point2D, a3, STACK_OFFSET(0x18, -0x10)); // unknown usage
	GET_STACK(int*, a5, STACK_OFFSET(0x18, 0x10)); // unknown usage
	GET_STACK(int, angle, STACK_OFFSET(0x18, 0xC));
	GET_STACK(int, idx, STACK_OFFSET(0x18, 0x8));
	GET_STACK(VoxelStruct*, pVXL, STACK_OFFSET(0x18, 0x4));

	auto pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto tur = pType->Gunner || pType->IsChargeTurret
		? &pType->ChargerTurrets[pThis->CurrentTurretNumber]
		: &pType->TurretVoxel;

	if (pTypeExt->TurretShadow.Get(RulesExt::Global()->DrawTurretShadow) && tur->VXL && tur->HVA)
	{
		auto mtx = pThis->Locomotor->Shadow_Matrix(0);
		mtx.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));
		float x = static_cast<float>(pTypeExt->TurretOffset.GetEx()->X / 8);
		float y = static_cast<float>(pTypeExt->TurretOffset.GetEx()->Y / 8);
		float z = -tur->VXL->TailerData->MinBounds.Z;
		mtx.Translate(x, y, z);
		Matrix3D::MatrixMultiply(&mtx, &Matrix3D::VoxelDefaultMatrix, &mtx);

		pThis->DrawVoxelShadow(tur, 0, angle, 0, a4, &a3, &mtx, a9, pSurface, pos);
		auto bar = &pType->BarrelVoxel;

		if (bar->VXL && bar->HVA)
			pThis->DrawVoxelShadow(bar, 0, angle, 0, a4, &a3, &mtx, a9, pSurface, pos);
	}

	if (!pTypeExt->ShadowIndices.size())
	{
		pThis->DrawVoxelShadow(pVXL, idx, angle, a5, a4, &a3, pMatrix, a9, pSurface, pos);
	}
	else
	{
		for (auto index : pTypeExt->ShadowIndices)
		{
			auto hva = pVXL->HVA;
			Matrix3D idxmtx = *pMatrix;
			idxmtx.TranslateZ(-hva->Matrixes[index].GetZVal());
			Matrix3D::MatrixMultiply(&idxmtx, &Matrix3D::VoxelDefaultMatrix, &idxmtx);
			pThis->DrawVoxelShadow(pVXL, index, angle, a5, a4, &a3, pMatrix, a9, pSurface, pos);
		}
	}

	return 0x4DB195;
}

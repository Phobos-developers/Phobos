#include <AnimClass.h>
#include <UnitClass.h>
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
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <JumpjetLocomotionClass.h>
#include <FlyLocomotionClass.h>
#include <RocketLocomotionClass.h>

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->HealthBar_Hide)
		return 0x6F6AB6;

	return 0;
}

DEFINE_HOOK(0x6F3C56, TechnoClass_GetFLH_TurretMultiOffset, 0x0)
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

constexpr reference<double, 0xB1D008> const Pixel_Per_Lepton {};

DEFINE_HOOK(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0x1D0, -0x13C));
	GET(TechnoTypeClass*, technoType, EBX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, Pixel_Per_Lepton);

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

	Matrix3D mtx;
	mtx.MakeIdentity();
	mtx.RotateZ(static_cast<float>(pThis->PrimaryFacing.Current().GetRadian<32>()));
	TechnoTypeExt::ApplyTurretOffset(pThis->Type, &mtx);

	double turretRad = pThis->TurretFacing().GetRadian<32>();
	double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
	float angle = static_cast<float>(turretRad - bodyRad);
	mtx.RotateZ(angle);

	auto res = mtx.GetTranslation();
	auto location = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };

	pos += TacticalClass::CoordsToScreen(location);

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

	GET(ObjectClass*, pObject, ECX);

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

DEFINE_HOOK(0x702672, TechnoClass_ReceiveDamage_RevengeWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));

	if (pSource)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);
		auto const pTypeExt = pExt->TypeExtData;

		if (pTypeExt && pTypeExt->RevengeWeapon.isset() &&
			EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
		{
			WeaponTypeExt::DetonateAt(pTypeExt->RevengeWeapon.Get(), pSource, pThis);
		}

		for (auto& attachEffect : pExt->AttachedEffects)
		{
			if (!attachEffect->IsActive())
				continue;

			auto const pType = attachEffect->GetType();

			if (!pType->RevengeWeapon.isset())
				continue;

			if (EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
				WeaponTypeExt::DetonateAt(pType->RevengeWeapon, pSource, pThis);
		}
	}

	return 0;
}

// 2nd order Pade approximant just in case someone complains about performance
constexpr double Pade2_2(double in)
{
	const double s = in - static_cast<int>(in);
	return GeneralUtils::FastPow(0.36787944117144233, static_cast<int>(in))
		* (12. - 6 * s + s * s) / (12. + 6 * s + s * s);
}

DEFINE_HOOK(0x73C47A, UnitClass_DrawAsVXL_Shadow, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	enum { SkipDrawing = 0x73C5C9 };
	auto const loco = pThis->Locomotor.GetInterfacePtr();
	if (pThis->Type->NoShadow || !loco->Is_To_Have_Shadow())
		return SkipDrawing;

	REF_STACK(Matrix3D, shadow_matrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(VoxelIndexKey, vxl_index_key, STACK_OFFSET(0x1C4, -0x1B0));
	LEA_STACK(RectangleStruct*, bounding, STACK_OFFSET(0x1C4, 0xC));
	LEA_STACK(Point2D*, floor, STACK_OFFSET(0x1C4, -0x1A4));
	GET_STACK(Surface* const, surface, STACK_OFFSET(0x1C4, -0x1A8));

	GET(UnitTypeClass*, pType, EBX);
	// This is not necessarily pThis->Type : UnloadingClass or WaterImage
	// This is the very reason I need to do this here, there's no less hacky way to get this Type from those inner calls

	const auto uTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto jjloco = locomotion_cast<JumpjetLocomotionClass*>(loco);
	const auto height = pThis->GetHeight();
	const double baseScale_log = RulesExt::Global()->AirShadowBaseScale_log; // -ln(baseScale) precomputed

	if (RulesExt::Global()->HeightShadowScaling && height > 0)
	{
		const double minScale = RulesExt::Global()->HeightShadowScaling_MinScale;
		if (jjloco)
		{
			const float cHeight = (float)uTypeExt->ShadowSizeCharacteristicHeight.Get(jjloco->Height);

			if (cHeight > 0)
			{
				shadow_matrix.Scale((float)std::max(Pade2_2(baseScale_log * height / cHeight), minScale));

				if (jjloco->State != JumpjetLocomotionClass::State::Hovering)
					vxl_index_key.Invalidate();
			}
		}
		else
		{
			const float cHeight = (float)uTypeExt->ShadowSizeCharacteristicHeight.Get(RulesClass::Instance->CruiseHeight);

			if (cHeight > 0 && height > 208)
			{
				shadow_matrix.Scale((float)std::max(Pade2_2(baseScale_log * (height - 208) / cHeight), minScale));
				vxl_index_key.Invalidate();
			}
		}
	}
	else if (!RulesExt::Global()->HeightShadowScaling && pThis->Type->ConsideredAircraft)
	{
		shadow_matrix.Scale((float)Pade2_2(baseScale_log));
	}

	// We need to handle Ares turrets/barrels
	struct DummyExtHere
	{
		char _[0xA4];
		std::vector<VoxelStruct> ChargerTurrets;
		std::vector<VoxelStruct> ChargerBarrels;
		char __[0x120];
		UnitTypeClass* WaterImage;
		VoxelStruct NoSpawnAltVXL;
	};

	auto GetMainVoxel = [&]()
	{
		if (pType->NoSpawnAlt && pThis->SpawnManager && pThis->SpawnManager->CountDockedSpawns() == 0)
		{
			if (CAN_USE_ARES && AresHelper::CanUseAres)
			{
				vxl_index_key.Invalidate();// I'd just assume most of the time we have spawn
				return &reinterpret_cast<DummyExtHere*>(pType->align_2FC)->NoSpawnAltVXL;
			}
			return &pType->TurretVoxel;
		}
		return &pType->MainVoxel;
	};

	auto const main_vxl = GetMainVoxel();

	// TODO : adjust shadow point according to height
	// There was a bit deviation that I cannot decipher, might need help with that
	// But it turns out it has basically no visual difference

	auto shadow_point = loco->Shadow_Point();
	auto why = *floor + shadow_point;

	float arf = pThis->AngleRotatedForwards;
	float ars = pThis->AngleRotatedSideways;
	// lazy, don't want to hook inside Shadow_Matrix
	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		// index key should have been already invalid, so it won't hurt to invalidate again
		vxl_index_key.Invalidate();
		shadow_matrix.TranslateX(float(Math::sgn(arf) * pType->VoxelScaleX * (1 - Math::cos(arf))));
		shadow_matrix.TranslateY(float(Math::sgn(-ars) * pType->VoxelScaleY * (1 - Math::cos(ars))));
		shadow_matrix.ScaleX((float)Math::cos(arf));
		shadow_matrix.ScaleY((float)Math::cos(ars));
	}

	auto mtx = Matrix3D::VoxelDefaultMatrix() * shadow_matrix;
	if (height > 0)
		shadow_point.Y += 1;

	if (uTypeExt->ShadowIndices.empty())
	{
		if (pType->ShadowIndex >= 0 && pType->ShadowIndex < main_vxl->HVA->LayerCount)
			pThis->DrawVoxelShadow(
				   main_vxl,
				   pType->ShadowIndex,
				   vxl_index_key,
				   &pType->VoxelShadowCache,
				   bounding,
				   &why,
				   &mtx,
				   true,
				   surface,
				   shadow_point
			);
	}
	else
	{
		for (auto& [index, _] : uTypeExt->ShadowIndices)
			pThis->DrawVoxelShadow(
				   main_vxl,
				   index,
				   vxl_index_key,
				   &pType->VoxelShadowCache,
				   bounding,
				   &why,
				   &mtx,
				   true,
				   surface,
				   shadow_point
			);
	}

	if (!uTypeExt->TurretShadow.Get(RulesExt::Global()->DrawTurretShadow) || main_vxl == &pType->TurretVoxel)
		return SkipDrawing;


	auto GetTurretVoxel = [pType](int idx) ->VoxelStruct*
	{
		if (pType->TurretCount == 0 || pType->IsGattling || idx < 0)
			return &pType->TurretVoxel;

		if (idx < 18)
			return &pType->ChargerTurrets[idx];

		if (CAN_USE_ARES && AresHelper::CanUseAres)
		{
			auto* aresTypeExt = reinterpret_cast<DummyExtHere*>(pType->align_2FC);
			return &aresTypeExt->ChargerTurrets[idx - 18];
		}

		return nullptr;
	};

	auto GetBarrelVoxel = [pType](int idx)->VoxelStruct*
	{
		if (pType->TurretCount == 0 || pType->IsGattling || idx < 0)
			return &pType->BarrelVoxel;

		if (idx < 18)
			return &pType->ChargerBarrels[idx];

		if (CAN_USE_ARES && AresHelper::CanUseAres)
		{
			auto* aresTypeExt = reinterpret_cast<DummyExtHere*>(pType->align_2FC);
			return &aresTypeExt->ChargerBarrels[idx - 18];
		}

		return nullptr;
	};

	Matrix3D rot = Matrix3D::GetIdentity();
	uTypeExt->ApplyTurretOffset(&rot, Pixel_Per_Lepton);
	rot.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));
	auto tur_mtx = mtx * rot; // unfortunately we won't have TurretVoxelScaleX/Y given the amount of work

	auto tur = GetTurretVoxel(pThis->CurrentTurretNumber);

	// sorry but you're fucked
	if (tur && tur->VXL && tur->HVA)
		pThis->DrawVoxelShadow(
			tur,
			0,
			std::bit_cast<VoxelIndexKey>(-1), // no cache, no use for valid key
			nullptr, // no cache atm
			bounding,
			&why,
			&tur_mtx,
			false,
			surface,
			shadow_point
		);

	auto bar = GetBarrelVoxel(pThis->CurrentTurretNumber);

	// and you are utterly fucked
	if (bar && bar->VXL && bar->HVA)
		pThis->DrawVoxelShadow(
			bar,
			0,
			std::bit_cast<VoxelIndexKey>(-1), // no cache, no use
			nullptr,//no cache atm
			bounding,
			&why,
			&tur_mtx,
			false,
			surface,
			shadow_point
		);

	// Add caches in Ext if necessary, remember not to serialize these shit
	// IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*> VoxelTurretShadowCache {};
	// IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*> VoxelBarrelShadowCache {};

	return SkipDrawing;
}

DEFINE_HOOK(0x4147F9, AircraftClass_Draw_Shadow, 0x6)
{
	GET(AircraftClass*, pThis, EBP);
	GET(const int, height, EBX);
	REF_STACK(VoxelIndexKey, key, STACK_OFFSET(0xCC, -0xBC));
	GET_STACK(Point2D, flor, STACK_OFFSET(0xCC, -0xAC));
	GET_STACK(RectangleStruct*, bound, STACK_OFFSET(0xCC, 0x10));
	enum { FinishDrawing = 0x4148A5 };

	const auto loco = pThis->Locomotor.GetInterfacePtr();
	if (pThis->Type->NoShadow || !loco->Is_To_Have_Shadow() || pThis->IsSinking)
		return FinishDrawing;

	auto shadow_mtx = loco->Shadow_Matrix(&key);
	const auto aTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (auto const flyLoco = locomotion_cast<FlyLocomotionClass*>(loco))
	{
		const double baseScale_log = RulesExt::Global()->AirShadowBaseScale_log;

		if (RulesExt::Global()->HeightShadowScaling)
		{
			const double minScale = RulesExt::Global()->HeightShadowScaling_MinScale;
			const float cHeight = (float)aTypeExt->ShadowSizeCharacteristicHeight.Get(pThis->Type->GetFlightLevel());

			if (cHeight > 0)
			{
				shadow_mtx.Scale((float)std::max(Pade2_2(baseScale_log * height / cHeight), minScale));
				if (flyLoco->FlightLevel > 0 || height > 0)
					key.Invalidate();
			}
		}
		else if (pThis->Type->ConsideredAircraft)
		{
			shadow_mtx.Scale((float)Pade2_2(baseScale_log));
		}

		double arf = pThis->AngleRotatedForwards;
		if (flyLoco->CurrentSpeed > pThis->Type->PitchSpeed)
			arf += pThis->Type->PitchAngle;
		double ars = pThis->AngleRotatedSideways;
		if (key.Is_Valid_Key() && (std::abs(arf) > 0.005 || std::abs(ars) > 0.005))
			key.Invalidate();

		shadow_mtx.ScaleY((float)Math::cos(ars));
		shadow_mtx.ScaleX((float)Math::cos(arf));
	}
	else if (height > 0)
	{
		// You must be Rocket, otherwise GO FUCK YOURSELF
		shadow_mtx.ScaleX((float)Math::cos(static_cast<RocketLocomotionClass*>(loco)->CurrentPitch));
		key.Invalidate();
	}

	shadow_mtx = Matrix3D::VoxelDefaultMatrix() * shadow_mtx;

	auto const main_vxl = &pThis->Type->MainVoxel;
	// flor += loco->Shadow_Point(); // no longer needed
	if (aTypeExt->ShadowIndices.empty())
	{
		auto const shadow_index = pThis->Type->ShadowIndex;
		if (shadow_index >= 0 && shadow_index < main_vxl->HVA->LayerCount)
			pThis->DrawVoxelShadow(main_vxl,
				shadow_index,
				key,
				&pThis->Type->VoxelShadowCache,
				bound,
				&flor,
				&shadow_mtx,
				true,
				nullptr,
				{ 0, 0 }
		);
	}
	else
	{
		for (auto& [index, _] : aTypeExt->ShadowIndices)
			pThis->DrawVoxelShadow(main_vxl,
				index,
				key,
				&pThis->Type->VoxelShadowCache,
				bound,
				&flor,
				&shadow_mtx,
				true,
				nullptr,
				{ 0, 0 }
		);
	}

	return FinishDrawing;
}

// Shadow_Point of RocketLoco was forgotten to be set to {0,0}. It was an oversight.
DEFINE_JUMP(VTABLE, 0x7F0B4C, 0x4CF940);
/*
//TO TEST AND EXPLAIN: why resetting height when drawing aircrafts?
DEFINE_JUMP(CALL6, 0x4147D5, 0x5F4300);
DEFINE_JUMP(CALL6, 0x4148AB, 0x5F4300);
DEFINE_JUMP(CALL6, 0x4147F3, 0x5F4300);
*/

DEFINE_HOOK(0x7072A1, suka707280_ChooseTheGoddamnMatrix, 0x7)
{
	GET(FootClass*, pThis, EBX);//Maybe Techno later
	GET(VoxelStruct*, pVXL, EBP);
	GET_STACK(Matrix3D*, pMat, STACK_OFFSET(0xE8, 0xC));
	GET_STACK(int, shadow_index_now, STACK_OFFSET(0xE8, 0x18));// it's used later, otherwise I could have chosen the frame index earlier

	REF_STACK(Matrix3D, matRet, STACK_OFFSET(0xE8, -0x60));
	auto pType = pThis->GetTechnoType();

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	auto hva = pVXL->HVA;

	auto ChooseFrame = [&]()->int //Don't want to use goto
	{
		// Turret or Barrel
		if (pVXL != &pType->MainVoxel)
		{
			// verify just in case:
			auto who_are_you = reinterpret_cast<uintptr_t*>(reinterpret_cast<DWORD>(pVXL) - (offsetof(TechnoTypeClass, MainVoxel)));
			if (who_are_you[0] == UnitTypeClass::AbsVTable)
				pType = reinterpret_cast<TechnoTypeClass*>(who_are_you);//you are someone else
			else
				return pThis->TurretAnimFrame % hva->FrameCount;
			// you might also be SpawnAlt voxel, but I can't know
			// otherwise what would you expect me to do, shift back to ares typeext base and check if ownerobject is technotype?
		}

		// Main body sections
		auto& shadowIndices = pTypeExt->ShadowIndices;
		if (shadowIndices.empty())
		{
			// Only ShadowIndex
			if (pType->ShadowIndex == shadow_index_now)
			{
				int shadow_index_frame = pTypeExt->ShadowIndex_Frame;
				if (shadow_index_frame > -1)
					return shadow_index_frame % hva->FrameCount;
			}
			else
			{
				// WHO THE HELL ARE YOU???
				return 0;
			}
		}
		else
		{
			int idx_of_now = shadowIndices[shadow_index_now];
			if (idx_of_now > -1)
				return idx_of_now % hva->FrameCount;
		}

		return pThis->WalkedFramesSoFar % hva->FrameCount;
	};


	Matrix3D hvamat = hva->Matrixes[shadow_index_now + hva->LayerCount * ChooseFrame()];

	// A nasty temporary backward compatibility option
	if (hva->LayerCount > 1 || pType->Turret)
	// NEEDS IMPROVEMENT : Choose the proper Z offset to shift the sections to the same level
		hvamat.TranslateZ(
			-hvamat.GetZVal()
			- pVXL->VXL->TailerData->Bounds[0].Z
		);

	matRet = *pMat * hvamat;

	// Recover vanilla instructions
	if (pThis->GetTechnoType()->UseBuffer)
		*reinterpret_cast<DWORD*>(0xB43180) = 1;

	REF_STACK(Matrix3D, b, STACK_OFFSET(0xE8, -0x90));
	b.MakeIdentity();// we don't do scaling here anymore

	return 0x707331;
}

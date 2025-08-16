#include <AircraftClass.h>
#include <BounceClass.h>
#include <FlyLocomotionClass.h>
#include <JumpjetLocomotionClass.h>
#include <RocketLocomotionClass.h>
#include <SpawnManagerClass.h>
#include <TacticalClass.h>
#include <TunnelLocomotionClass.h>
#include <UnitClass.h>
#include <Utilities/AresHelper.h>
#include <Utilities/Macro.h>
#include <Ext/Techno/Body.h>

#include "Body.h"


DEFINE_REFERENCE(double, Pixel_Per_Lepton, 0xB1D008)

#pragma region FLH_Turrets

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	TechnoTypeExt::ExtMap.Find(pType)->ApplyTurretOffset(mtx, factor);
}

DEFINE_HOOK(0x6F3E6E, TechnoClass_ActionLines_TurretMultiOffset, 0x0)
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

	if (*pTypeData->TurretOffset.GetEx() == CoordStruct { 0, 0, 0 })
		return 0x73B78A;

	return 0x73B790;
}

struct AresTechnoTypeExt
{
	char _[0xA4];
	std::vector<VoxelStruct> ChargerTurrets;
	std::vector<VoxelStruct> ChargerBarrels;
	char __[0x120];
	UnitTypeClass* WaterImage;
	VoxelStruct NoSpawnAltVXL;
};

DEFINE_HOOK(0x73BA12, UnitClass_DrawAsVXL_RewriteTurretDrawing, 0x6)
{
	enum { SkipGameCode = 0x73BEA4 };

	GET(UnitClass* const, pThis, EBP);
	GET(TechnoTypeClass* const, pDrawType, EBX);
	GET_STACK(const bool, haveTurretCache, STACK_OFFSET(0x1C4, -0x1B3));
	GET_STACK(const bool, haveBar, STACK_OFFSET(0x1C4, -0x1B2));
	GET(const bool, haveBarrelCache, EAX);
	REF_STACK(Matrix3D, drawMatrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(const int, flags, STACK_OFFSET(0x1C4, -0x198));
	GET_STACK(const int, brightness, STACK_OFFSET(0x1C4, 0x1C));
	GET_STACK(const int, hvaFrameIdx, STACK_OFFSET(0x1C4, -0x18C));
	GET_STACK(const int, currentTurretNumber, STACK_OFFSET(0x1C4, -0x1A8));
	LEA_STACK(Point2D* const, center, STACK_OFFSET(0x1C4, -0x194));
	LEA_STACK(RectangleStruct* const, rect, STACK_OFFSET(0x1C4, -0x164));

	// base matrix
	const auto mtx = Matrix3D::VoxelDefaultMatrix * drawMatrix;

	const auto pDrawTypeExt = TechnoTypeExt::ExtMap.Find(pDrawType);
	const bool notChargeTurret = pThis->Type->TurretCount <= 0 || pThis->Type->IsGattling;

	auto getTurretVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
	{
		if (notChargeTurret)
			return &pDrawType->TurretVoxel;

		// Not considering the situation where there is no Ares and the limit is exceeded
		if (currentTurretNumber < 18 || !AresHelper::CanUseAres)
			return &pDrawType->ChargerTurrets[currentTurretNumber];

		auto* aresTypeExt = reinterpret_cast<AresTechnoTypeExt*>(pDrawType->align_2FC);
		return &aresTypeExt->ChargerTurrets[currentTurretNumber - 18];
	};
	const auto pTurretVoxel = getTurretVoxel();

	// When in recoiling or have no cache, need to recalculate drawing matrix
	const bool inRecoil = pDrawType->TurretRecoil && (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive || pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive);
	const bool shouldRedraw = !haveTurretCache || haveBar && !haveBarrelCache || inRecoil;

	// When in recoiling, need to bypass cache and draw without saving
	const auto turKey = inRecoil ? -1 : flags;
	const auto turCache = inRecoil ? nullptr : &pDrawType->VoxelTurretWeaponCache;

	auto getTurretMatrix = [=, &mtx]() -> Matrix3D
	{
		auto mtxTurret = mtx;
		pDrawTypeExt->ApplyTurretOffset(&mtxTurret, Pixel_Per_Lepton);
		mtxTurret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

		if (pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive)
			mtxTurret.TranslateX(-pThis->TurretRecoil.TravelSoFar);

		return mtxTurret;
	};
	auto mtxTurret = shouldRedraw ? getTurretMatrix() : mtx;
	constexpr BlitterFlags blit = BlitterFlags::Alpha | BlitterFlags::Flat;

	// Only when there is a barrel will its calculation and drawing be considered
	if (haveBar)
	{
		auto drawBarrel = [=, &mtxTurret, &mtx]()
		{
			// When in recoiling, need to bypass cache and draw without saving
			const auto brlKey = inRecoil ? -1 : flags;
			const auto brlCache = inRecoil ? nullptr : &pDrawType->VoxelTurretBarrelCache;

			auto getBarrelMatrix = [=, &mtxTurret, &mtx]() -> Matrix3D
			{
				auto mtxBarrel = mtxTurret;
				mtxBarrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
				mtxBarrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));

				if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
					mtxBarrel.TranslateX(-pThis->BarrelRecoil.TravelSoFar);

				mtxBarrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
				return mtxBarrel;
			};
			auto mtxBarrel = shouldRedraw ? getBarrelMatrix() : mtx;

			auto getBarrelVoxel = [pDrawType, notChargeTurret, currentTurretNumber]() -> VoxelStruct*
			{
				if (notChargeTurret)
					return &pDrawType->BarrelVoxel;

				// Not considering the situation where there is no Ares and the limit is exceeded
				if (currentTurretNumber < 18 || !AresHelper::CanUseAres)
					return &pDrawType->ChargerBarrels[currentTurretNumber];

				auto* aresTypeExt = reinterpret_cast<AresTechnoTypeExt*>(pDrawType->align_2FC);
				return &aresTypeExt->ChargerBarrels[currentTurretNumber - 18];
			};
			const auto pBarrelVoxel = getBarrelVoxel();

			// draw barrel
			pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, &mtxBarrel, brightness, blit, 0);
		};

		const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();

		// The orientation of the turret can affect the layer order of the barrel and turret
		if (turretDir != 0 && turretDir != 3)
		{
			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtxTurret, brightness, blit, 0);

			drawBarrel();
		}
		else
		{
			drawBarrel();

			// draw turret
			pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtxTurret, brightness, blit, 0);
		}
	}
	else
	{
		pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtxTurret, brightness, blit, 0);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x73C890, UnitClass_DrawSHP_BarrelMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x80);
	GET(TechnoTypeClass*, technoType, EAX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1 / 8);

	return 0x73C8B7;
}

DEFINE_HOOK(0x43E0C4, BuildingClass_Draw_VXLTurretMultiOffset, 0x0)
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

	Matrix3D mtx = Matrix3D::GetIdentity();
	mtx.RotateZ(static_cast<float>(pThis->PrimaryFacing.Current().GetRadian<32>()));
	TechnoTypeExt::ApplyTurretOffset(pThis->Type, &mtx);

	const double turretRad = pThis->TurretFacing().GetRadian<32>();
	const double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
	const float angle = static_cast<float>(turretRad - bodyRad);
	mtx.RotateZ(angle);

	const auto res = mtx.GetTranslation();
	const auto location = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };

	pos += TacticalClass::CoordsToScreen(location);

	return 0;
}

#pragma endregion

// Hi there copycat

#pragma region draw_matrix

struct JumpjetTiltVoxelIndexKey
{
	unsigned bodyFrame : 5;
	unsigned bodyFace : 5;
	unsigned slopeIndex : 6;
	unsigned isSpawnAlt : 1;
	unsigned forwards : 7;
	unsigned sideways : 7;
	unsigned reserved : 1;
};

struct PhobosVoxelIndexKey
{
	union
	{
		VoxelIndexKey Base;
		union
		{
			JumpjetTiltVoxelIndexKey JumpjetTiltVoxel;
			// add other definitions here as needed
		} CustomIndexKey;
	};

	// add funcs here if needed
	constexpr bool IsCleanKey() const { return Base.Value == 0; }
	constexpr bool IsJumpjetKey() const { return Base.MainVoxel.Reserved != 0; }
};

static_assert(sizeof(PhobosVoxelIndexKey) == sizeof(VoxelIndexKey), "PhobosVoxelIndexKey size mismatch");

DEFINE_HOOK(0x4CF68D, FlyLocomotionClass_DrawMatrix_OnAirport, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	__assume(iloco != nullptr);
	const auto loco = static_cast<FlyLocomotionClass*>(iloco);
	const auto pThis = static_cast<AircraftClass*>(loco->LinkedTo);
	if (pThis->GetHeight() <= 0)
	{
		REF_STACK(Matrix3D, mat, STACK_OFFSET(0x38, -0x30));
		const auto slope_idx = MapClass::Instance.GetCellAt(pThis->Location)->SlopeIndex;
		mat = Matrix3D::VoxelRampMatrix[slope_idx] * mat;
		const float ars = pThis->AngleRotatedSideways;
		const float arf = pThis->AngleRotatedForwards;
		if (std::abs(ars) > 0.005 || std::abs(arf) > 0.005)
		{
			const auto pType = pThis->Type;
			mat.TranslateZ(float(std::abs(Math::sin(ars)) * pType->VoxelScaleX
				+ std::abs(Math::sin(arf)) * pType->VoxelScaleY));
			R->ECX(pThis);
			return 0x4CF6AD;
		}
	}

	return 0x4CF6A0;
}

namespace JumpjetTiltReference
{
	constexpr auto BaseSpeed = 32;
	constexpr auto BaseTilt = Math::HalfPi / 4;
	constexpr auto BaseTurnRaw = 32768;
	constexpr auto MaxTilt = static_cast<float>(Math::HalfPi);
	constexpr auto ForwardBaseTilt = BaseTilt / BaseSpeed;
	constexpr auto SidewaysBaseTilt = BaseTilt / (BaseTurnRaw * BaseSpeed);
}

// Just rewrite this completely to avoid headache
Matrix3D* __stdcall JumpjetLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, PhobosVoxelIndexKey* key)
{
	__assume(iloco != nullptr);
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	auto const linked = pThis->LinkedTo;
	// no more TiltCrashJumpjet, do that above svp
	bool const onGround = pThis->State == JumpjetLocomotionClass::State::Grounded;
	// Man, what can I say, you don't want to stick your rotor into the ground
	auto const slope_idx = MapClass::Instance.GetCellAt(linked->Location)->SlopeIndex;
	*ret = Matrix3D::VoxelRampMatrix[onGround ? slope_idx : 0];
	// Only use LocomotionFacing for general Jumpjet to avoid the problem that ground units being lifted will turn to attacker weirdly.
	auto const curf = linked->IsAttackedByLocomotor ? linked->PrimaryFacing.Current() : pThis->LocomotionFacing.Current();
	ret->RotateZ((float)curf.GetRadian<32>());
	float arf = linked->AngleRotatedForwards;
	float ars = linked->AngleRotatedSideways;
	size_t arfFace = 0;
	size_t arsFace = 0;

	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		if (key)
			key->Base.Invalidate();

		if (onGround)
		{
			const auto pType = linked->GetTechnoType();
			const double scalex = pType->VoxelScaleX;
			const double scaley = pType->VoxelScaleY;
			Matrix3D pre = Matrix3D::GetIdentity();
			pre.TranslateZ(float(std::abs(Math::sin(ars)) * scalex + std::abs(Math::sin(arf)) * scaley));
			ret->TranslateX(float(Math::sgn(arf) * (scaley * (1 - Math::cos(arf)))));
			ret->TranslateY(float(Math::sgn(-ars) * (scalex * (1 - Math::cos(ars)))));
			ret->RotateX(ars);
			ret->RotateY(arf);
			*ret = pre * *ret;
		}
		else
		{
			// No more translation because I don't like it
			ret->RotateX(ars);
			ret->RotateY(arf);
		}
	}
	else
	{
		const auto pTypeExt = TechnoExt::ExtMap.Find(linked)->TypeExtData;

		if (pTypeExt->JumpjetTilt
			&& !onGround
			&& pThis->CurrentSpeed > 0.0
			&& linked->WhatAmI() == AbstractType::Unit
			&& linked->IsAlive
			&& linked->Health > 0
			&& !linked->IsAttackedByLocomotor)
		{
			const float forwardSpeedFactor = static_cast<float>(pThis->CurrentSpeed * pTypeExt->JumpjetTilt_ForwardSpeedFactor);
			const float forwardAccelFactor = static_cast<float>(pThis->Accel * pTypeExt->JumpjetTilt_ForwardAccelFactor);

			arf = Math::clamp(static_cast<float>((forwardAccelFactor + forwardSpeedFactor)
				* JumpjetTiltReference::ForwardBaseTilt), -JumpjetTiltReference::MaxTilt, JumpjetTiltReference::MaxTilt);

			const auto& locoFace = pThis->LocomotionFacing;

			if (locoFace.IsRotating())
			{
				const float sidewaysSpeedFactor = static_cast<float>(pThis->CurrentSpeed * pTypeExt->JumpjetTilt_SidewaysSpeedFactor);
				const float sidewaysRotationFactor = static_cast<float>(static_cast<short>(locoFace.Difference().Raw)
					* pTypeExt->JumpjetTilt_SidewaysRotationFactor);

				ars = Math::clamp(static_cast<float>(sidewaysSpeedFactor * sidewaysRotationFactor
					* JumpjetTiltReference::SidewaysBaseTilt), -JumpjetTiltReference::MaxTilt, JumpjetTiltReference::MaxTilt);

				const auto arsDir = DirStruct(ars);

				// When changing the radian to DirStruct, it will rotate 90 degrees.
				// To ensure that 0 is still 0, it needs to be rotated back
				arsFace = arsDir.GetFacing<128>(96);

				if (arsFace)
					ret->RotateX(static_cast<float>(arsDir.GetRadian<128>()));
			}

			const auto arfDir = DirStruct(arf);

			// Similarly, turn it back
			arfFace = arfDir.GetFacing<128>(96);

			if (arfFace)
				ret->RotateY(static_cast<float>(arfDir.GetRadian<128>()));
		}
	}

	if (key && key->Base.Is_Valid_Key())
	{
		// It is currently unclear whether the passed key only has two situations:
		// all 0s and all 1s, so I use the safest approach for now
		if (key->IsCleanKey() && (arfFace || arsFace))
		{
			key->CustomIndexKey.JumpjetTiltVoxel.forwards = arfFace;
			key->CustomIndexKey.JumpjetTiltVoxel.sideways = arsFace;

			if (onGround)
				key->CustomIndexKey.JumpjetTiltVoxel.slopeIndex = slope_idx;

			key->CustomIndexKey.JumpjetTiltVoxel.bodyFace = curf.GetFacing<32>();

			// Outside the function, there is another step to add a frame number to the key for drawing
			key->Base.Value >>= 5;
		}
		else // Keep the original code
		{
			if (onGround)
				key->Base.Value = slope_idx + (key->Base.Value << 6);

			key->Base.Value <<= 5;
			key->Base.Value |= curf.GetFacing<32>();
		}
	}

	return ret;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECD8C, JumpjetLocomotionClass_Draw_Matrix);

DEFINE_HOOK(0x73B748, UnitClass_DrawVXL_ResetKeyForTurretUse, 0x7)
{
	REF_STACK(PhobosVoxelIndexKey, key, STACK_OFFSET(0x1C4, -0x1B0));

	// Main body drawing completed, then enable accurate drawing of turrets and barrels
	if (key.Base.Is_Valid_Key() && key.IsJumpjetKey())
		key.Base.Invalidate();

	return 0;
}

// Visual bugfix : Teleport loco vxls could not tilt
Matrix3D* __stdcall TeleportLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* pIndex)
{
	__assume(iloco != nullptr);
	auto const pThis = static_cast<LocomotionClass*>(iloco);
	auto const linked = pThis->LinkedTo;
	auto const slope_idx = MapClass::Instance.GetCellAt(linked->Location)->SlopeIndex;

	if (pIndex && pIndex->Is_Valid_Key())
		*(int*)(pIndex) = slope_idx + (*(int*)(pIndex) << 6);

	*ret = Matrix3D::VoxelRampMatrix[slope_idx] * pThis->LocomotionClass::Draw_Matrix(pIndex);

	const float arf = linked->AngleRotatedForwards;
	const float ars = linked->AngleRotatedSideways;

	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		if (pIndex)
			pIndex->Invalidate();

		const auto pType = linked->GetTechnoType();
		const double scalex = pType->VoxelScaleX;
		const double scaley = pType->VoxelScaleY;

		Matrix3D pre = Matrix3D::GetIdentity();
		pre.TranslateZ(float(std::abs(Math::sin(ars)) * scalex + std::abs(Math::sin(arf)) * scaley));
		ret->TranslateX(float(Math::sgn(arf) * (scaley * (1 - Math::cos(arf)))));
		ret->TranslateY(float(Math::sgn(-ars) * (scalex * (1 - Math::cos(ars)))));
		ret->RotateX(ars);
		ret->RotateY(arf);

		*ret = pre * *ret;
	}
	return ret;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5024, TeleportLocomotionClass_Draw_Matrix);

// Visual bugfix: Tunnel loco could not tilt when being flipped
DEFINE_HOOK(0x729B5D, TunnelLocomotionClass_DrawMatrix_Tilt, 0x8)
{
	GET(ILocomotion*, iloco, ESI);
	GET_BASE(VoxelIndexKey*, pIndex, 0x10);
	GET_BASE(Matrix3D*, ret, 0xC);
	R->EAX(TeleportLocomotionClass_Draw_Matrix(iloco, ret, pIndex));
	return 0x729C09;
}

#pragma endregion


#pragma region shadow_matrix
// just in case any retard complains about performance
constexpr double Pade2_2(double in)
{
	const double s = in - static_cast<int>(in);
	return GeneralUtils::FastPow(0.36787944117144233, static_cast<int>(in))
		* (12. - 6 * s + s * s) / (12. + 6 * s + s * s);
}

Matrix3D* __fastcall sub7559B0(Matrix3D* ret, int idx)
{
	*ret = Matrix3D::VoxelRampMatrix[idx] * Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };
	return ret;
}
DEFINE_FUNCTION_JUMP(CALL, 0x55A814, sub7559B0);

Matrix3D* __stdcall TunnelLocomotionClass_ShadowMatrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* key)
{
	__assume(iloco != nullptr);
	const auto tLoco = static_cast<TunnelLocomotionClass*>(iloco);
	*ret = tLoco->LocomotionClass::Shadow_Matrix(key);
	if (tLoco->State != TunnelLocomotionClass::State::Idle)
	{
		double theta = 0.;
		switch (tLoco->State)
		{
		case TunnelLocomotionClass::State::DiggingIn:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (const auto total = tLoco->DigTimer.Rate)
				theta *= 1.0 - double(tLoco->DigTimer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DugIn:
			theta = Math::HalfPi;
			break;
		case TunnelLocomotionClass::State::PreDigOut:
			theta = -Math::HalfPi;
			break;
		case TunnelLocomotionClass::State::DiggingOut:
			if (key)key->Invalidate();
			theta = -Math::HalfPi;
			if (const auto total = tLoco->DigTimer.Rate)
				theta *= double(tLoco->DigTimer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DugOut:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (const auto total = tLoco->DigTimer.Rate)
				theta *= double(tLoco->DigTimer.GetTimeLeft()) / double(total);
			break;
		default:break;
		}
		ret->RotateY((float)theta);// I know it's ugly
	}
	return ret;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5A4C, TunnelLocomotionClass_ShadowMatrix);

DEFINE_HOOK(0x73C47A, UnitClass_DrawAsVXL_Shadow, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	enum { SkipDrawing = 0x73C5C9 };
	auto const loco = pThis->Locomotor.GetInterfacePtr();
	if (pThis->CloakState != CloakState::Uncloaked || pThis->Type->NoShadow || !loco->Is_To_Have_Shadow())
		return SkipDrawing;

	REF_STACK(Matrix3D, shadowMatrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(VoxelIndexKey, vxlIndexKey, STACK_OFFSET(0x1C4, -0x1B0));
	LEA_STACK(RectangleStruct*, bnd, STACK_OFFSET(0x1C4, 0xC));
	LEA_STACK(Point2D*, pt, STACK_OFFSET(0x1C4, -0x1A4));
	GET_STACK(Surface* const, surface, STACK_OFFSET(0x1C4, -0x1A8));

	GET(UnitTypeClass*, pDrawType, EBX);
	// This is not necessarily pThis->Type : UnloadingClass or WaterImage
	// This is the very reason I need to do this here, there's no less hacky way to get this Type from those inner calls

	const auto pDrawTypeExt = TechnoTypeExt::ExtMap.Find(pDrawType);
	const auto jjloco = locomotion_cast<JumpjetLocomotionClass*>(loco);
	const auto height = pThis->GetHeight();
	const double baseScale_log = RulesExt::Global()->AirShadowBaseScale_log;

	if (RulesExt::Global()->HeightShadowScaling && height > 0)
	{
		const double minScale = RulesExt::Global()->HeightShadowScaling_MinScale;
		if (jjloco)
		{
			const float cHeight = (float)pDrawTypeExt->ShadowSizeCharacteristicHeight.Get(jjloco->Height);

			if (cHeight > 0)
			{
				shadowMatrix.Scale((float)std::max(Pade2_2(baseScale_log * height / cHeight), minScale));

				if (jjloco->State != JumpjetLocomotionClass::State::Hovering)
					vxlIndexKey.Invalidate();
			}
		}
		else
		{
			const float cHeight = (float)pDrawTypeExt->ShadowSizeCharacteristicHeight.Get(RulesClass::Instance->CruiseHeight);

			if (cHeight > 0 && height > 208)
			{
				shadowMatrix.Scale((float)std::max(Pade2_2(baseScale_log * (height - 208) / cHeight), minScale));
				vxlIndexKey.Invalidate();
			}
		}
	}
	else if (!RulesExt::Global()->HeightShadowScaling && pThis->Type->ConsideredAircraft)
	{
		shadowMatrix.Scale((float)Pade2_2(baseScale_log));
	}

	auto GetMainVoxel = [&]()
		{
			if (pDrawType->NoSpawnAlt && pThis->SpawnManager && pThis->SpawnManager->CountDockedSpawns() == 0)
			{
				if (AresHelper::CanUseAres)
				{
					vxlIndexKey.Invalidate();// I'd just assume most of the time we have spawn
					return &reinterpret_cast<AresTechnoTypeExt*>(pDrawType->align_2FC)->NoSpawnAltVXL;
				}
				return &pDrawType->TurretVoxel;
			}
			return &pDrawType->MainVoxel;
		};

	auto const main_vxl = GetMainVoxel();

	auto shadowPoint = loco->Shadow_Point();
	auto why = *pt + shadowPoint;

	float arf = pThis->AngleRotatedForwards;
	float ars = pThis->AngleRotatedSideways;
	// lazy, don't want to hook inside Shadow_Matrix
	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		// index key should have been already invalid, so it won't hurt to invalidate again
		vxlIndexKey.Invalidate();
		shadowMatrix.TranslateX(float(Math::sgn(arf) * pDrawType->VoxelScaleX * (1 - Math::cos(arf))));
		shadowMatrix.TranslateY(float(Math::sgn(-ars) * pDrawType->VoxelScaleY * (1 - Math::cos(ars))));
		shadowMatrix.RotateY(arf);
		shadowMatrix.RotateX(ars);
	}
	else if (jjloco
		&& pDrawTypeExt->JumpjetTilt
		&& jjloco->State != JumpjetLocomotionClass::State::Grounded
		&& jjloco->CurrentSpeed > 0.0
		&& pThis->IsAlive
		&& pThis->Health > 0
		&& !pThis->IsAttackedByLocomotor)
	{
		const float forwardSpeedFactor = static_cast<float>(jjloco->CurrentSpeed * pDrawTypeExt->JumpjetTilt_ForwardSpeedFactor);
		const float forwardAccelFactor = static_cast<float>(jjloco->Accel * pDrawTypeExt->JumpjetTilt_ForwardAccelFactor);

		arf = Math::clamp(static_cast<float>((forwardAccelFactor + forwardSpeedFactor)
			* JumpjetTiltReference::ForwardBaseTilt), -JumpjetTiltReference::MaxTilt, JumpjetTiltReference::MaxTilt);

		const auto& locoFace = jjloco->LocomotionFacing;

		if (locoFace.IsRotating())
		{
			const float sidewaysSpeedFactor = static_cast<float>(jjloco->CurrentSpeed * pDrawTypeExt->JumpjetTilt_SidewaysSpeedFactor);
			const float sidewaysRotationFactor = static_cast<float>(static_cast<short>(locoFace.Difference().Raw)
				* pDrawTypeExt->JumpjetTilt_SidewaysRotationFactor);

			ars = Math::clamp(static_cast<float>(sidewaysSpeedFactor * sidewaysRotationFactor
				* JumpjetTiltReference::SidewaysBaseTilt), -JumpjetTiltReference::MaxTilt, JumpjetTiltReference::MaxTilt);
		}

		if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
		{
			vxlIndexKey.Invalidate();
			shadowMatrix.RotateX(ars);
			shadowMatrix.RotateY(arf);
		}
	}

	auto mtx = Matrix3D::VoxelDefaultMatrix * shadowMatrix;

	if (height > 0)
		shadowPoint.Y += 1;

	if (!pDrawType->UseTurretShadow)
	{
		if (pDrawTypeExt->ShadowIndices.empty())
		{
			if (pDrawType->ShadowIndex >= 0 && pDrawType->ShadowIndex < main_vxl->HVA->LayerCount)
			{
				pThis->DrawVoxelShadow(
					main_vxl,
					pDrawType->ShadowIndex,
					vxlIndexKey,
					&pDrawType->VoxelShadowCache,
					bnd,
					&why,
					&mtx,
					true,
					surface,
					shadowPoint
				);
			}
		}
		else
		{
			for (auto& [index, _] : pDrawTypeExt->ShadowIndices)
			{
				pThis->DrawVoxelShadow(
					main_vxl,
					index,
					index == pDrawType->ShadowIndex ? vxlIndexKey : VoxelIndexKey(-1),
					&pDrawType->VoxelShadowCache,
					bnd,
					&why,
					&mtx,
					index == pDrawType->ShadowIndex,
					surface,
					shadowPoint
				);
			}
		}
	}

	if (main_vxl == &pDrawType->TurretVoxel || (!pDrawType->UseTurretShadow && !pDrawTypeExt->TurretShadow.Get(RulesExt::Global()->DrawTurretShadow)))
		return SkipDrawing;

	auto GetTurretVoxel = [pDrawType](int idx) ->VoxelStruct*
	{
		if (pDrawType->TurretCount == 0 || pDrawType->IsGattling || idx < 0)
			return &pDrawType->TurretVoxel;

		if (idx < 18)
			return &pDrawType->ChargerTurrets[idx];

		if (AresHelper::CanUseAres)
		{
			auto* aresTypeExt = reinterpret_cast<AresTechnoTypeExt*>(pDrawType->align_2FC);
			return &aresTypeExt->ChargerTurrets[idx - 18];
		}

		return nullptr;
	};

	auto GetBarrelVoxel = [pDrawType](int idx)->VoxelStruct*
	{
		if (pDrawType->TurretCount == 0 || pDrawType->IsGattling || idx < 0)
			return &pDrawType->BarrelVoxel;

		if (idx < 18)
			return &pDrawType->ChargerBarrels[idx];

		if (AresHelper::CanUseAres)
		{
			auto* aresTypeExt = reinterpret_cast<AresTechnoTypeExt*>(pDrawType->align_2FC);
			return &aresTypeExt->ChargerBarrels[idx - 18];
		}

		return nullptr;
	};

	pDrawTypeExt->ApplyTurretOffset(&mtx, Pixel_Per_Lepton);
	mtx.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

	const bool inRecoil = pDrawType->TurretRecoil && pThis->TurretRecoil.State != RecoilData::RecoilState::Inactive;
	if (inRecoil)
		mtx.TranslateX(-pThis->TurretRecoil.TravelSoFar);

	const auto tur = GetTurretVoxel(pThis->CurrentTurretNumber);
	if (!(tur && tur->VXL && tur->HVA))
		return SkipDrawing;

	const auto bar = GetBarrelVoxel(pThis->CurrentTurretNumber);
	const auto haveBar = bar && bar->VXL && bar->HVA && !bar->VXL->Initialized;
	if (vxlIndexKey.Is_Valid_Key())
		vxlIndexKey.MinorVoxel.TurretFacing = pThis->SecondaryFacing.Current().GetFacing<32>();

	auto* cache = &pDrawType->VoxelShadowCache;
	if (!pDrawType->UseTurretShadow)
	{
		if (haveBar)
		{
			cache = nullptr;
		}
		else
		{
			cache = tur != &pDrawType->TurretVoxel
				? nullptr // man what can I say, you are fucked, for now
				: reinterpret_cast<decltype(cache)>(&pDrawType->VoxelTurretBarrelCache); // excuse me
		}
	}

	pThis->DrawVoxelShadow(
		tur,
		0,
		(inRecoil ? VoxelIndexKey(-1) : vxlIndexKey),
		(inRecoil ? nullptr : cache),
		bnd,
		&why,
		&mtx,
		(!inRecoil && cache != nullptr),
		surface,
		shadowPoint
	);

	if (haveBar)// you are utterly fucked, for now
	{
		if (pDrawType->TurretRecoil && pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
			mtx.TranslateX(-pThis->BarrelRecoil.TravelSoFar);

		mtx.ScaleX(static_cast<float>(Math::cos(-pThis->BarrelFacing.Current().GetRadian<32>())));

		pThis->DrawVoxelShadow(
			bar,
			0,
			VoxelIndexKey(-1),
			nullptr,
			bnd,
			&why,
			&mtx,
			false,
			surface,
			shadowPoint
		);
	}

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

	AircraftTypeClass* pAircraftType = pThis->Type;
	const auto loco = pThis->Locomotor.GetInterfacePtr();

	if (pAircraftType->NoShadow || pThis->CloakState != CloakState::Uncloaked || pThis->IsSinking || !loco->Is_To_Have_Shadow())
		return FinishDrawing;

	pAircraftType = TechnoExt::GetAircraftTypeExtra(pThis);

	auto shadow_mtx = loco->Shadow_Matrix(&key);
	const auto aTypeExt = TechnoTypeExt::ExtMap.Find(pAircraftType);

	if (auto const flyLoco = locomotion_cast<FlyLocomotionClass*>(loco))
	{
		const double baseScale_log = RulesExt::Global()->AirShadowBaseScale_log;

		if (RulesExt::Global()->HeightShadowScaling)
		{
			const double minScale = RulesExt::Global()->HeightShadowScaling_MinScale;
			const float cHeight = (float)aTypeExt->ShadowSizeCharacteristicHeight.Get(pAircraftType->GetFlightLevel());

			if (cHeight > 0)
			{
				shadow_mtx.Scale((float)std::max(Pade2_2(baseScale_log * height / cHeight), minScale));
				if (flyLoco->FlightLevel > 0 || height > 0)
					key.Invalidate();
			}
		}
		else if (pAircraftType->ConsideredAircraft)
		{
			shadow_mtx.Scale((float)Pade2_2(baseScale_log));
		}

		double arf = pThis->AngleRotatedForwards;
		if (flyLoco->CurrentSpeed > pAircraftType->PitchSpeed)
			arf += pAircraftType->PitchAngle;
		float ars = pThis->AngleRotatedSideways;
		if (key.Is_Valid_Key() && (std::abs(arf) > 0.005 || std::abs(ars) > 0.005))
			key.Invalidate();

		shadow_mtx.RotateX(ars);
		shadow_mtx.RotateY((float)arf);
	}
	else if (height > 0)
	{
		// You must be Rocket, otherwise GO FUCK YOURSELF
		shadow_mtx.RotateY(static_cast<RocketLocomotionClass*>(loco)->CurrentPitch);
		key.Invalidate();
	}

	shadow_mtx = Matrix3D::VoxelDefaultMatrix * shadow_mtx;

	auto const main_vxl = &pAircraftType->MainVoxel;
	auto shadow_cache = &pAircraftType->VoxelShadowCache;
	// flor += loco->Shadow_Point(); // no longer needed
	if (aTypeExt->ShadowIndices.empty())
	{
		const int shadow_index = pAircraftType->ShadowIndex;
		if (shadow_index >= 0 && shadow_index < main_vxl->HVA->LayerCount)
			pThis->DrawVoxelShadow(main_vxl,
				shadow_index,
				key,
				shadow_cache,
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
		const int shadow_index = pAircraftType->ShadowIndex;
		for (auto& [index, _] : aTypeExt->ShadowIndices)
			pThis->DrawVoxelShadow(main_vxl,
				index,
				index == shadow_index ? key : std::bit_cast<VoxelIndexKey>(-1),
				shadow_cache,
				bound,
				&flor,
				&shadow_mtx,
				index == shadow_index,
				nullptr,
				{ 0, 0 }
			);
	}

	return FinishDrawing;
}

DEFINE_JUMP(VTABLE, 0x7F0B4C, 0x4CF940);// Shadow_Point of RocketLoco was forgotten to be set to {0,0}. It was an oversight.
DEFINE_JUMP(LJMP, 0x706BDD, 0x706C01); // I checked it a priori

/*
//TO TEST AND EXPLAIN: why resetting height when drawing aircrafts?
DEFINE_JUMP(CALL6, 0x4147D5, 0x5F4300);
DEFINE_JUMP(CALL6, 0x4148AB, 0x5F4300);
DEFINE_JUMP(CALL6, 0x4147F3, 0x5F4300);
*/

DEFINE_HOOK(0x7072A1, cyka707280_WhichMatrix, 0x6)
{
	GET(FootClass*, pThis, EBX);//Maybe Techno later, for GTGCAN
	GET(VoxelStruct*, pVXL, EBP);
	GET_STACK(Matrix3D*, pMat, STACK_OFFSET(0xE8, 0xC));
	GET_STACK(const int, shadow_index_now, STACK_OFFSET(0xE8, 0x18));// it's used later, otherwise I could have chosen the frame index earlier

	REF_STACK(Matrix3D, matRet, STACK_OFFSET(0xE8, -0x60));
	auto pType = pThis->GetTechnoType();

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	const auto hva = pVXL->HVA;

	auto ChooseFrame = [&]()->int //Don't want to use goto
		{
			// Turret or Barrel
			if (pVXL != &pType->MainVoxel)
			{
				// verify just in case:
				const auto who_are_you = reinterpret_cast<uintptr_t*>(reinterpret_cast<DWORD>(pVXL) - (offsetof(TechnoTypeClass, MainVoxel)));
				if (who_are_you[0] == UnitTypeClass::AbsVTable)
					pType = reinterpret_cast<TechnoTypeClass*>(who_are_you);//you are someone else
				else
				{
					// guess what, someone actually has a multisection nospawnalt
					if (!(AresHelper::CanUseAres && pVXL == &reinterpret_cast<AresTechnoTypeExt*>(pType->align_2FC)->NoSpawnAltVXL))
						return pThis->TurretAnimFrame % hva->FrameCount;
				}
				// you might also be WaterImage or sth else, but I don't want to care anymore, go fuck yourself
			}

			// Main body sections
			auto& shadowIndices = pTypeExt->ShadowIndices;
			if (shadowIndices.empty())
			{
				// Only ShadowIndex
				if (pType->ShadowIndex == shadow_index_now)
				{
					const int shadow_index_frame = pTypeExt->ShadowIndex_Frame;
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
				const int idx_of_now = shadowIndices[shadow_index_now];
				if (idx_of_now > -1)
					return idx_of_now % hva->FrameCount;
			}

			return pThis->WalkedFramesSoFar % hva->FrameCount;
		};

	matRet = *pMat * hva->Matrixes[shadow_index_now + hva->LayerCount * ChooseFrame()] * Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };

	double l2 = 0;
	auto& arr = matRet.row;
	for (int i = 0; i < 3; i++)	for (int j = 0; j < 3; j++)	l2 += arr[i][j] * arr[i][j];
	if (l2 < 0.03) R->Stack(STACK_OFFSET(0xE8, 0x20), true);

	// Recover vanilla instructions
	if (pType->UseBuffer)
		*reinterpret_cast<DWORD*>(0xB43180) = 1;

	REF_STACK(Matrix3D, b, STACK_OFFSET(0xE8, -0x90));
	b.MakeIdentity(); // we don't do scaling here anymore

	return 0x707331;
}

Matrix3D* __fastcall BounceClass_ShadowMatrix(BounceClass* self, void*, Matrix3D* ret)
{
	Matrix3D::FromQuaternion(ret, &self->CurrentAngle);
	*ret = Matrix3D { 1, 0, 0 , 0, 0, 0.25f, -0.4330127f , 0,0, -0.4330127f, 0.75f , 0 }**ret * Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };
	return ret;
}
DEFINE_FUNCTION_JUMP(CALL, 0x749CAC, BounceClass_ShadowMatrix);
#pragma endregion

#pragma region voxel_ramp_matrix

// I don't know how can WW miscalculated
// In fact, there should be three different degrees of tilt angles
// - EBX -> atan((2*104)/(256√2)) should only be used on the steepest slopes (13-16)
// - EBP -> atan(104/256) should be used on the most common slopes (1-4)
// - A smaller radian atan(104/(256√2)) should be use to other slopes (5-12)
// But this position is too far ahead, I can't find a good way to solve it perfectly
// Using hooks and filling in floating-point numbers will cause the register to reset to zero
// So I have to do it this way for now, make changes based on the existing data
// Thanks to NetsuNegi for providing a simpler patch method to replace the hook method
DEFINE_PATCH(0x75546D, 0x55) // push ebp
DEFINE_PATCH(0x755484, 0x55) // push ebp
DEFINE_PATCH(0x7554A1, 0x55) // push ebp
DEFINE_PATCH(0x7554BE, 0x55) // push ebp
DEFINE_PATCH(0x755656, 0x55) // push ebp
DEFINE_PATCH(0x755677, 0x55) // push ebp
DEFINE_PATCH(0x755698, 0x55) // push ebp
DEFINE_PATCH(0x7556B9, 0x55) // push ebp
// Although it is not the perfectest
// It can still solve the most common situations on slopes - CrimRecya

#pragma endregion

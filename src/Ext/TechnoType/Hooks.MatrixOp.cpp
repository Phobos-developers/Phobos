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

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor, int turIdx)
{
	TechnoTypeExt::ExtMap.Find(pType)->ApplyTurretOffset(mtx, factor, turIdx);
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
	enum { CleanFlag = 0x73B78A, SkipFlag = 0x73B790 };

	GET(TechnoTypeClass* const, pDrawType, EBX);

	auto const pDrawTypeExt = TechnoTypeExt::ExtMap.Find(pDrawType);

	return (*pDrawTypeExt->TurretOffset.GetEx() == CoordStruct::Empty
		&& pDrawTypeExt->ExtraTurretCount <= 0
		&& pDrawTypeExt->ExtraBarrelCount <= 0)
		? CleanFlag : SkipFlag;
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

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
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
	const auto pBarrelVoxel = haveBar ? getBarrelVoxel() : nullptr;

	constexpr BlitterFlags blit = BlitterFlags::Alpha | BlitterFlags::Flat;
	// When in recoiling or have no cache, need to recalculate drawing matrix
	const bool shouldRedraw = !haveTurretCache || haveBar && !haveBarrelCache;

	// The orientation of the turret can affect the layer order of the barrel and turret
	const auto turretDir = pThis->SecondaryFacing.Current().GetFacing<4>();
	const bool barrelOverTechno = pDrawTypeExt->BarrelOverTurret.Get(turretDir != 0 && turretDir != 3);

	auto drawTurret = [=, &mtx](int turIdx)
		{
			const auto pTurData = pDrawType->TurretRecoil ? ((turIdx < 0) ? &pThis->TurretRecoil : &pExt->ExtraTurretRecoil[turIdx]) : nullptr;
			const bool turretInRecoil = pTurData && pTurData->State != RecoilData::RecoilState::Inactive;

			// When in recoiling or is not main turret, need to bypass cache and draw without saving
			const bool turShouldRedraw = turretInRecoil || turIdx >= 0;
			const auto turKey = turShouldRedraw ? -1 : flags;
			const auto turCache = turShouldRedraw ? nullptr : &pDrawType->VoxelTurretWeaponCache;

			auto shouldCalculateMatrix = [=]()
				{
					if (!haveBar)
						return false;

					if (pThis->BarrelRecoil.State != RecoilData::RecoilState::Inactive)
						return true;

					return pDrawTypeExt->ExtraBarrelCount.Get() > 0;
				};
			auto getTurretMatrix = [=, &mtx]() -> Matrix3D
				{
					auto mtx_turret = mtx;
					pDrawTypeExt->ApplyTurretOffset(&mtx_turret, Pixel_Per_Lepton, turIdx);
					mtx_turret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

					if (turretInRecoil)
						mtx_turret.TranslateX(-pTurData->TravelSoFar);

					return mtx_turret;
				};
			auto mtx_turret = (shouldRedraw || turShouldRedraw || shouldCalculateMatrix()) ? getTurretMatrix() : mtx;

			auto drawBarrel = [=, &mtx_turret, &mtx](int brlIdx)
				{
					const auto idx = brlIdx + ((turIdx + 1) * (pDrawTypeExt->ExtraBarrelCount.Get() + 1));
					const auto pBrlData = pDrawType->TurretRecoil ? ((idx < 0) ? &pThis->BarrelRecoil : &pExt->ExtraBarrelRecoil[idx]) : nullptr;
					const bool barrelInRecoil = pBrlData && pBrlData->State != RecoilData::RecoilState::Inactive;

					// When in recoiling or is not main barrel, need to bypass cache and draw without saving
					const bool brlShouldRedraw = turretInRecoil || barrelInRecoil || idx >= 0;
					const auto brlKey = brlShouldRedraw ? -1 : flags;
					const auto brlCache = brlShouldRedraw ? nullptr : &pDrawType->VoxelTurretBarrelCache;

					auto getBarrelMatrix = [=, &mtx_turret, &mtx]() -> Matrix3D
						{
							auto mtx_barrel = mtx_turret;
							mtx_barrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
							mtx_barrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));
							const auto offset = ((brlIdx < 0) ? pDrawTypeExt->BarrelOffset.Get() : pDrawTypeExt->ExtraBarrelOffsets[brlIdx]);
							mtx_barrel.TranslateY(static_cast<float>(Pixel_Per_Lepton * offset));

							if (barrelInRecoil)
								mtx_barrel.TranslateX(-pBrlData->TravelSoFar);

							mtx_barrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
							return mtx_barrel;
						};
					auto mtx_barrel = (shouldRedraw || brlShouldRedraw) ? getBarrelMatrix() : mtx;

					// draw barrel
					pThis->Draw_A_VXL(pBarrelVoxel, hvaFrameIdx, brlKey, brlCache, rect, center, &mtx_barrel, brightness, blit, 0);
				};

			auto drawBarrels = [&drawBarrel, pDrawTypeExt, turretDir]()
				{
					const auto exBrlCount = pDrawTypeExt->ExtraBarrelCount.Get();

					if (exBrlCount > 0)
					{
						std::vector<int> barrels;
						barrels.emplace_back(-1);

						for (int i = 0; i < exBrlCount; ++i)
							barrels.emplace_back(i);

						const auto barrelsSize = barrels.size();
						const bool faceRight = turretDir == 0 || turretDir == 1;
						std::sort(&barrels[0], &barrels[barrelsSize], [pDrawTypeExt, faceRight](const auto& idxA, const auto& idxB)
							{
										const auto offsetA = idxA < 0 ? pDrawTypeExt->BarrelOffset.Get() : pDrawTypeExt->ExtraBarrelOffsets[idxA];
										const auto offsetB = idxB < 0 ? pDrawTypeExt->BarrelOffset.Get() : pDrawTypeExt->ExtraBarrelOffsets[idxB];

										return faceRight ? (offsetA > offsetB) : (offsetA <= offsetB);
							});

						for (const auto& i : barrels)
							drawBarrel(i);
					}
					else
					{
						drawBarrel(-1);
					}
				};

			if (barrelOverTechno)
			{
				// draw turret
				pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, blit, 0);

				if (haveBar)
					drawBarrels();
			}
			else
			{
				if (haveBar)
					drawBarrels();

				// draw turret
				pThis->Draw_A_VXL(pTurretVoxel, hvaFrameIdx, turKey, turCache, rect, center, &mtx_turret, brightness, blit, 0);
			}
		};

	auto drawTurrets = [&drawTurret, pThis, pDrawTypeExt]()
		{
			const auto exTurCount = pDrawTypeExt->ExtraTurretCount.Get();

			if (exTurCount > 0)
			{
				std::vector<int> turrets;
				turrets.emplace_back(-1);

				for (int i = 0; i < exTurCount; ++i)
					turrets.emplace_back(i);

				const auto turretsSize = turrets.size();
				std::sort(&turrets[0], &turrets[turretsSize], [pThis, pDrawTypeExt](const auto& idxA, const auto& idxB)
					{
							const auto pOffsetA = idxA < 0 ? static_cast<CoordStruct*>(pDrawTypeExt->TurretOffset.GetEx()) : &pDrawTypeExt->ExtraTurretOffsets[idxA];
							const auto pOffsetB = idxB < 0 ? static_cast<CoordStruct*>(pDrawTypeExt->TurretOffset.GetEx()) : &pDrawTypeExt->ExtraTurretOffsets[idxB];

							if (pOffsetA->Z < pOffsetB->Z)
								return true;

							if (pOffsetA->Z > pOffsetB->Z)
								return false;

							const auto pointA = TacticalClass::Instance->CoordsToClient(TechnoExt::GetFLHAbsoluteCoords(pThis, *pOffsetA)).first;
							const auto pointB = TacticalClass::Instance->CoordsToClient(TechnoExt::GetFLHAbsoluteCoords(pThis, *pOffsetB)).first;

							if (pointA.Y < pointB.Y)
								return true;

							if (pointA.Y > pointB.Y)
								return false;

							return pointA.X <= pointB.X;
					});

				for (const auto& i : turrets)
					drawTurret(i);
			}
			else
			{
				drawTurret(-1);
			}
		};
	drawTurrets();

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
static Matrix3D* __stdcall JumpjetLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, PhobosVoxelIndexKey* key)
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
static Matrix3D* __stdcall TeleportLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* pIndex)
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

static Matrix3D* __fastcall sub7559B0(Matrix3D* ret, int idx)
{
	*ret = Matrix3D::VoxelRampMatrix[idx] * Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };
	return ret;
}
DEFINE_FUNCTION_JUMP(CALL, 0x55A814, sub7559B0);

static Matrix3D* __stdcall TunnelLocomotionClass_ShadowMatrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* key)
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
	enum { SkipDrawing = 0x73C5C9 };

	GET(UnitClass* const, pThis, EBP);

	auto const loco = pThis->Locomotor.GetInterfacePtr();

	if (pThis->CloakState != CloakState::Uncloaked || pThis->Type->NoShadow || !loco->Is_To_Have_Shadow())
		return SkipDrawing;

	REF_STACK(Matrix3D, shadowMatrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(VoxelIndexKey, vxlIndexKey, STACK_OFFSET(0x1C4, -0x1B0));
	LEA_STACK(RectangleStruct* const, bnd, STACK_OFFSET(0x1C4, 0xC));
	LEA_STACK(Point2D* const, pt, STACK_OFFSET(0x1C4, -0x1A4));
	GET_STACK(Surface* const, surface, STACK_OFFSET(0x1C4, -0x1A8));

	GET(TechnoTypeClass* const, pDrawType, EBX);
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
	auto shadowCenter = *pt + shadowPoint;

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

	const bool notUseTurretShadow = pDrawType->WhatAmI() != AbstractType::UnitType || !static_cast<UnitTypeClass*>(pDrawType)->UseTurretShadow;

	if (notUseTurretShadow)
	{
		if (pDrawTypeExt->ShadowIndices.empty())
		{
			if (pDrawType->ShadowIndex >= 0 && pDrawType->ShadowIndex < main_vxl->HVA->LayerCount)
			{
				pThis->DrawVoxelShadow(main_vxl, pDrawType->ShadowIndex, vxlIndexKey,
					&pDrawType->VoxelShadowCache, bnd, &shadowCenter, &mtx, true, surface, shadowPoint);
			}
		}
		else
		{
			for (auto& [index, _] : pDrawTypeExt->ShadowIndices)
			{
				pThis->DrawVoxelShadow(main_vxl, index, index == pDrawType->ShadowIndex ? vxlIndexKey : VoxelIndexKey(-1),
					&pDrawType->VoxelShadowCache, bnd, &shadowCenter, &mtx, index == pDrawType->ShadowIndex, surface, shadowPoint);
			}
		}
	}

	if (main_vxl == &pDrawType->TurretVoxel || (notUseTurretShadow && !pDrawTypeExt->TurretShadow.Get(RulesExt::Global()->DrawTurretShadow)))
		return SkipDrawing;

	auto getTurretVoxel = [pDrawType](int idx) ->VoxelStruct*
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
	const auto pTurretVoxel = getTurretVoxel(pThis->CurrentTurretNumber);

	if (!(pTurretVoxel && pTurretVoxel->VXL && pTurretVoxel->HVA))
		return SkipDrawing;

	if (vxlIndexKey.Is_Valid_Key())
		vxlIndexKey.MinorVoxel.TurretFacing = pThis->SecondaryFacing.Current().GetFacing<32>();

	auto getBarrelVoxel = [pDrawType](int idx)->VoxelStruct*
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
	const auto pBarrelVoxel = getBarrelVoxel(pThis->CurrentTurretNumber);

	const auto haveBar = pBarrelVoxel && pBarrelVoxel->VXL && pBarrelVoxel->HVA && !pBarrelVoxel->VXL->Initialized;
	auto pCache = &pDrawType->VoxelShadowCache;
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	// Not available under multiple turrets/barrels due to different base positions
	if (notUseTurretShadow)
		pCache = (haveBar || pTurretVoxel != &pDrawType->TurretVoxel) ? nullptr : reinterpret_cast<decltype(pCache)>(&pDrawType->VoxelTurretBarrelCache);

	auto drawTurretShadow = [&](int turIdx)
		{
			auto mtx_turret = mtx;
			pDrawTypeExt->ApplyTurretOffset(&mtx_turret, Pixel_Per_Lepton, turIdx);
			mtx_turret.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

			const auto pTurData = pDrawType->TurretRecoil ? ((turIdx >= 0) ? &pExt->ExtraTurretRecoil[turIdx] : &pThis->TurretRecoil) : nullptr;
			const auto turretInRecoil = pTurData && pTurData->State != RecoilData::RecoilState::Inactive;
			const auto shouldRedraw = turretInRecoil || turIdx >= 0;

			if (turretInRecoil)
				mtx_turret.TranslateX(-pTurData->TravelSoFar);

			pThis->DrawVoxelShadow(pTurretVoxel, 0, (shouldRedraw ? VoxelIndexKey(-1) : vxlIndexKey), (shouldRedraw ? nullptr : pCache),
				bnd, &shadowCenter, &mtx_turret, (!shouldRedraw && pCache != nullptr), surface, shadowPoint);

			if (!haveBar)
				return;

			auto drawBarrelShadow = [=, &mtx_turret, &mtx, &shadowCenter](int brlIdx)
				{
					const auto idx = brlIdx + ((turIdx + 1) * (pDrawTypeExt->ExtraBarrelCount + 1));
					const auto pBrlData = pDrawType->TurretRecoil ? ((idx >= 0) ? &pExt->ExtraBarrelRecoil[idx] : &pThis->BarrelRecoil) : nullptr;
					const auto barrelInRecoil = pBrlData && pBrlData->State != RecoilData::RecoilState::Inactive;

					auto mtx_barrel = mtx_turret;
					mtx_barrel.Translate(-mtx.Row[0].W, -mtx.Row[1].W, -mtx.Row[2].W);
					mtx_barrel.RotateY(static_cast<float>(-pThis->BarrelFacing.Current().GetRadian<32>()));
					const auto offset = ((brlIdx >= 0) ? pDrawTypeExt->ExtraBarrelOffsets[brlIdx] : pDrawTypeExt->BarrelOffset.Get());
					mtx_barrel.TranslateY(static_cast<float>(Pixel_Per_Lepton * offset));

					if (barrelInRecoil)
						mtx_barrel.TranslateX(-pBrlData->TravelSoFar);

					mtx_barrel.Translate(mtx.Row[0].W, mtx.Row[1].W, mtx.Row[2].W);
					pThis->DrawVoxelShadow(pBarrelVoxel, 0, VoxelIndexKey(-1), nullptr, bnd, &shadowCenter, &mtx_barrel, false, surface, shadowPoint);
				};

			auto drawBarrelsShadow = [&drawBarrelShadow, pDrawTypeExt]()
				{
					drawBarrelShadow(-1);

					const auto exBrlCount = pDrawTypeExt->ExtraBarrelCount.Get();

					if (exBrlCount > 0)
					{
						for (int i = 0; i < exBrlCount; ++i)
							drawBarrelShadow(i);
					}
				};
			drawBarrelsShadow();
		};

	auto drawTurretsShadow = [&drawTurretShadow, pDrawTypeExt]()
		{
			drawTurretShadow(-1);

			const auto exTurCount = pDrawTypeExt->ExtraTurretCount.Get();

			if (exTurCount > 0)
			{
				for (int i = 0; i < exTurCount; ++i)
					drawTurretShadow(i);
			}
		};
	drawTurretsShadow();

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

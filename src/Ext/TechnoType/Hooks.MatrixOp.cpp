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

#include "Body.h"


constexpr reference<double, 0xB1D008> const Pixel_Per_Lepton {};

#pragma region FLH_Turrets

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	TechnoTypeExt::ExtMap.Find(pType)->ApplyTurretOffset(mtx, factor);
}

DEFINE_HOOK(0x6F3C56, TechnoClass_GetFLH_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0xD8, -0x90));
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3C6D;
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


DEFINE_HOOK(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0x1D0, -0x13C));
	GET(TechnoTypeClass*, technoType, EBX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, Pixel_Per_Lepton);

	return 0x73BA68;
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

	double turretRad = pThis->TurretFacing().GetRadian<32>();
	double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
	float angle = static_cast<float>(turretRad - bodyRad);
	mtx.RotateZ(angle);

	auto res = mtx.GetTranslation();
	auto location = CoordStruct { static_cast<int>(res.X), static_cast<int>(-res.Y), static_cast<int>(res.Z) };

	pos += TacticalClass::CoordsToScreen(location);

	return 0;
}

#pragma endregion

// Hi there copycat

#pragma region draw_matrix

DEFINE_HOOK(0x4CF68D, FlyLocomotionClass_DrawMatrix_OnAirport, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	__assume(iloco != nullptr);
	auto loco = static_cast<FlyLocomotionClass*>(iloco);
	auto pThis = static_cast<AircraftClass*>(loco->LinkedTo);
	if (pThis->GetHeight() <= 0)
	{
		REF_STACK(Matrix3D, mat, STACK_OFFSET(0x38, -0x30));
		auto slope_idx = MapClass::Instance->GetCellAt(pThis->Location)->SlopeIndex;
		mat = Matrix3D::VoxelRampMatrix[slope_idx] * mat;
		float ars = pThis->AngleRotatedSideways;
		float arf = pThis->AngleRotatedForwards;
		if (std::abs(ars) > 0.005 || std::abs(arf) > 0.005)
		{
			mat.TranslateZ(float(std::abs(Math::sin(ars)) * pThis->Type->VoxelScaleX
				+ std::abs(Math::sin(arf)) * pThis->Type->VoxelScaleY));
			R->ECX(pThis);
			return 0x4CF6AD;
		}
	}

	return 0x4CF6A0;
}

// Just rewrite this completely to avoid headache
Matrix3D* __stdcall JumpjetLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, int* pIndex)
{
	__assume(iloco != nullptr);
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iloco);
	auto linked = pThis->LinkedTo;
	// no more TiltCrashJumpjet, do that above svp
	bool const onGround = pThis->State == JumpjetLocomotionClass::State::Grounded;
	// Man, what can I say, you don't want to stick your rotor into the ground
	auto slope_idx = MapClass::Instance->GetCellAt(linked->Location)->SlopeIndex;
	*ret = Matrix3D::VoxelRampMatrix[onGround ? slope_idx : 0];
	auto curf = pThis->LocomotionFacing.Current();
	ret->RotateZ((float)curf.GetRadian<32>());
	float arf = linked->AngleRotatedForwards;
	float ars = linked->AngleRotatedSideways;

	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		if (pIndex) *pIndex = -1;

		if (onGround)
		{
			double scalex = linked->GetTechnoType()->VoxelScaleX;
			double scaley = linked->GetTechnoType()->VoxelScaleY;
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

	if (pIndex && *pIndex != -1)
	{
		if (onGround) *pIndex = slope_idx + (*pIndex << 6);
		*pIndex *= 32;
		*pIndex |= curf.GetFacing<32>();
	}

	return ret;
}
DEFINE_JUMP(VTABLE, 0x7ECD8C, GET_OFFSET(JumpjetLocomotionClass_Draw_Matrix));


// Visual bugfix : Teleport loco vxls could not tilt
Matrix3D* __stdcall TeleportLocomotionClass_Draw_Matrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* pIndex)
{
	__assume(iloco != nullptr);
	auto const pThis = static_cast<LocomotionClass*>(iloco);
	auto linked = pThis->LinkedTo;
	auto slope_idx = MapClass::Instance->GetCellAt(linked->Location)->SlopeIndex;

	if (pIndex && pIndex->Is_Valid_Key())
		*(int*)(pIndex) = slope_idx + (*(int*)(pIndex) << 6);

	*ret = Matrix3D::VoxelRampMatrix[slope_idx] * pThis->LocomotionClass::Draw_Matrix(pIndex);

	float arf = linked->AngleRotatedForwards;
	float ars = linked->AngleRotatedSideways;

	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		if (pIndex)
			pIndex->Invalidate();

		double scalex = linked->GetTechnoType()->VoxelScaleX;
		double scaley = linked->GetTechnoType()->VoxelScaleY;

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

DEFINE_JUMP(VTABLE, 0x7F5024, GET_OFFSET(TeleportLocomotionClass_Draw_Matrix));

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
DEFINE_JUMP(CALL, 0x55A814, GET_OFFSET(sub7559B0));

Matrix3D* __stdcall TunnelLocomotionClass_ShadowMatrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* key)
{
	__assume(iloco != nullptr);
	auto tLoco = static_cast<TunnelLocomotionClass*>(iloco);
	*ret = tLoco->LocomotionClass::Shadow_Matrix(key);
	if (tLoco->State != TunnelLocomotionClass::State::Idle)
	{
		double theta = 0.;
		switch (tLoco->State)
		{
		case TunnelLocomotionClass::State::DiggingIn:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (auto total = tLoco->DigTimer.Rate)
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
			if (auto total = tLoco->DigTimer.Rate)
				theta *= double(tLoco->DigTimer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DugOut:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (auto total = tLoco->DigTimer.Rate)
				theta *= double(tLoco->DigTimer.GetTimeLeft()) / double(total);
			break;
		default:break;
		}
		ret->RotateY((float)theta);// I know it's ugly
	}
	return ret;
}
DEFINE_JUMP(VTABLE, 0x7F5A4C, GET_OFFSET(TunnelLocomotionClass_ShadowMatrix));

struct DummyTypeExtHere
{
	char _[0xA4];
	std::vector<VoxelStruct> ChargerTurrets;
	std::vector<VoxelStruct> ChargerBarrels;
	char __[0x120];
	UnitTypeClass* WaterImage;
	VoxelStruct NoSpawnAltVXL;
};

DEFINE_HOOK(0x73C47A, UnitClass_DrawAsVXL_Shadow, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	enum { SkipDrawing = 0x73C5C9 };
	auto const loco = pThis->Locomotor.GetInterfacePtr();
	if (pThis->CloakState != CloakState::Uncloaked || pThis->Type->NoShadow || !loco->Is_To_Have_Shadow())
		return SkipDrawing;

	REF_STACK(Matrix3D, shadow_matrix, STACK_OFFSET(0x1C4, -0x130));
	GET_STACK(VoxelIndexKey, vxl_index_key, STACK_OFFSET(0x1C4, -0x1B0));
	LEA_STACK(RectangleStruct*, bnd, STACK_OFFSET(0x1C4, 0xC));
	LEA_STACK(Point2D*, pt, STACK_OFFSET(0x1C4, -0x1A4));
	GET_STACK(Surface* const, surface, STACK_OFFSET(0x1C4, -0x1A8));

	GET(UnitTypeClass*, pType, EBX);
	// This is not necessarily pThis->Type : UnloadingClass or WaterImage
	// This is the very reason I need to do this here, there's no less hacky way to get this Type from those inner calls

	const auto uTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto jjloco = locomotion_cast<JumpjetLocomotionClass*>(loco);
	const auto height = pThis->GetHeight();
	const double baseScale_log = RulesExt::Global()->AirShadowBaseScale_log;

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

	auto GetMainVoxel = [&]()
		{
			if (pType->NoSpawnAlt && pThis->SpawnManager && pThis->SpawnManager->CountDockedSpawns() == 0)
			{
				if (AresHelper::CanUseAres)
				{
					vxl_index_key.Invalidate();// I'd just assume most of the time we have spawn
					return &reinterpret_cast<DummyTypeExtHere*>(pType->align_2FC)->NoSpawnAltVXL;
				}
				return &pType->TurretVoxel;
			}
			return &pType->MainVoxel;
		};

	auto const main_vxl = GetMainVoxel();


	auto shadow_point = loco->Shadow_Point();
	auto why = *pt + shadow_point;

	float arf = pThis->AngleRotatedForwards;
	float ars = pThis->AngleRotatedSideways;
	// lazy, don't want to hook inside Shadow_Matrix
	if (std::abs(ars) >= 0.005 || std::abs(arf) >= 0.005)
	{
		// index key should have been already invalid, so it won't hurt to invalidate again
		vxl_index_key.Invalidate();
		shadow_matrix.TranslateX(float(Math::sgn(arf) * pType->VoxelScaleX * (1 - Math::cos(arf))));
		shadow_matrix.TranslateY(float(Math::sgn(-ars) * pType->VoxelScaleY * (1 - Math::cos(ars))));
		shadow_matrix.RotateY(arf);
		shadow_matrix.RotateX(ars);
	}

	auto mtx = Matrix3D::VoxelDefaultMatrix() * shadow_matrix;

	if (height > 0)
		shadow_point.Y += 1;

	if (!pType->UseTurretShadow)
	if (uTypeExt->ShadowIndices.empty())
	{
		if (pType->ShadowIndex >= 0 && pType->ShadowIndex < main_vxl->HVA->LayerCount)
			pThis->DrawVoxelShadow(
				main_vxl,
				pType->ShadowIndex,
				vxl_index_key,
				&pType->VoxelShadowCache,
				bnd,
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
				index == pType->ShadowIndex ? vxl_index_key : VoxelIndexKey(-1),
				&pType->VoxelShadowCache,
				bnd,
				&why,
				&mtx,
				index == pType->ShadowIndex,
				surface,
				shadow_point
			);
	}

	if (main_vxl == &pType->TurretVoxel || (!pType->UseTurretShadow && !uTypeExt->TurretShadow.Get(RulesExt::Global()->DrawTurretShadow)))
		return SkipDrawing;

	auto GetTurretVoxel = [pType](int idx) ->VoxelStruct*
		{
			if (pType->TurretCount == 0 || pType->IsGattling || idx < 0)
				return &pType->TurretVoxel;

			if (idx < 18)
				return &pType->ChargerTurrets[idx];

			if (AresHelper::CanUseAres)
			{
				auto* aresTypeExt = reinterpret_cast<DummyTypeExtHere*>(pType->align_2FC);
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

			if (AresHelper::CanUseAres)
			{
				auto* aresTypeExt = reinterpret_cast<DummyTypeExtHere*>(pType->align_2FC);
				return &aresTypeExt->ChargerBarrels[idx - 18];
			}

			return nullptr;
		};

	uTypeExt->ApplyTurretOffset(&mtx, Pixel_Per_Lepton);
	mtx.RotateZ(static_cast<float>(pThis->SecondaryFacing.Current().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>()));

	auto tur = GetTurretVoxel(pThis->CurrentTurretNumber);
	if (!(tur && tur->VXL && tur->HVA))
		return SkipDrawing;

	auto bar = GetBarrelVoxel(pThis->CurrentTurretNumber);
	auto haveBar = bar && bar->VXL && bar->HVA && !bar->VXL->Initialized;
	if (vxl_index_key.Is_Valid_Key())
		vxl_index_key.TurretWeapon.Facing = pThis->SecondaryFacing.Current().GetFacing<32>();

	auto* cache = &pType->VoxelShadowCache;
	if (!pType->UseTurretShadow)
	{
		if (haveBar)
			cache = nullptr;
		else
			cache = tur != &pType->TurretVoxel ?
			nullptr // man what can I say, you are fucked, for now
			: reinterpret_cast<decltype(cache)>(&pType->VoxelTurretBarrelCache) // excuse me
			;
	}

	pThis->DrawVoxelShadow(
		tur,
		0,
		vxl_index_key,
		cache,
		bnd,
		&why,
		&mtx,
		cache != nullptr,
		surface,
		shadow_point
	);

	if (haveBar)// you are utterly fucked, for now
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
			shadow_point
		);

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
	if (pThis->Type->NoShadow || pThis->CloakState != CloakState::Uncloaked || pThis->IsSinking || !loco->Is_To_Have_Shadow())
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
		float ars = pThis->AngleRotatedSideways;
		if (key.Is_Valid_Key() && (std::abs(arf) > 0.005 || std::abs(ars) > 0.005))
			key.Invalidate();

		shadow_mtx.RotateX((float)ars);
		shadow_mtx.RotateY((float)arf);
	}
	else if (height > 0)
	{
		// You must be Rocket, otherwise GO FUCK YOURSELF
		shadow_mtx.RotateY(static_cast<RocketLocomotionClass*>(loco)->CurrentPitch);
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
				index == pThis->Type->ShadowIndex ? key : std::bit_cast<VoxelIndexKey>(-1),
				&pThis->Type->VoxelShadowCache,
				bound,
				&flor,
				&shadow_mtx,
				index == pThis->Type->ShadowIndex,
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
				{
					// guess what, someone actually has a multisection nospawnalt
					if (!(AresHelper::CanUseAres && pVXL == &reinterpret_cast<DummyTypeExtHere*>(pType->align_2FC)->NoSpawnAltVXL))
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

	matRet = *pMat * hva->Matrixes[shadow_index_now + hva->LayerCount * ChooseFrame()] * Matrix3D { 1,0,0,0,0,1,0,0,0,0,0,0 };

	double l2 = 0;
	auto& arr = matRet.row;
	for (int i = 0; i < 3; i++)	for (int j = 0; j < 3; j++)	l2 += arr[i][j] * arr[i][j];
	if (l2 < 0.03) R->Stack(STACK_OFFSET(0xE8, 0x20), true);

	// Recover vanilla instructions
	if (pThis->GetTechnoType()->UseBuffer)
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
DEFINE_JUMP(CALL, 0x749CAC, GET_OFFSET(BounceClass_ShadowMatrix));
#pragma endregion

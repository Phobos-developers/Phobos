#include "Body.h"
#include <LocomotionClass.h>
#include <TeleportLocomotionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <TacticalClass.h>

#define GET_LOCO(reg_Loco) \
	GET(ILocomotion *, Loco, reg_Loco); \
	__assume(Loco!=nullptr);\
	TeleportLocomotionClass *pLocomotor = static_cast<TeleportLocomotionClass*>(Loco); \
	FootClass* pLinked = pLocomotor->LinkedTo;\
	TechnoTypeClass const*pType = pLinked->GetTechnoType(); \
	TechnoTypeExt::ExtData const*pExt = TechnoTypeExt::ExtMap.Find(pType);

DEFINE_HOOK(0x7193F6, TeleportLocomotionClass_ILocomotion_Process_WarpoutAnim, 0x6)
{
	GET_LOCO(ESI);

	if (auto pWarpOut = pExt->WarpOut.Get(RulesClass::Instance->WarpOut))
		GameCreate<AnimClass>(pWarpOut, pLinked->Location)->Owner = pLinked->Owner;

	if (pExt->WarpOutWeapon)
		WeaponTypeExt::DetonateAt(pExt->WarpOutWeapon, pLinked, pLinked);

	const int distance = (int)Math::sqrt(pLinked->Location.DistanceFromSquared(pLocomotor->LastCoords));
	TechnoExt::ExtMap.Find(pLinked)->LastWarpDistance = distance;

	if (auto pImage = pType->AlphaImage)
	{
		auto [xy, _] = TacticalClass::Instance->CoordsToClient(pLinked->Location);
		RectangleStruct Dirty = { xy.X - (pImage->Width / 2) , xy.Y - (pImage->Height / 2),
		  pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}

	int duree = pExt->ChronoMinimumDelay.Get(RulesClass::Instance->ChronoMinimumDelay);

	if (distance >= pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum)
		&& pExt->ChronoTrigger.Get(RulesClass::Instance->ChronoTrigger))
	{
		int factor = std::max(pExt->ChronoDistanceFactor.Get(RulesClass::Instance->ChronoDistanceFactor), 1);
		duree = std::max(distance / factor, duree);

	}
	pLocomotor->Timer.Start(duree);

	pLinked->WarpingOut = true;

	if (auto pUnit = specific_cast<UnitClass*>(pLinked))
	{
		if (pUnit->Type->Harvester || pUnit->Type->Weeder)
		{
			pLocomotor->Timer.Start(0);
			pLinked->WarpingOut = false;
		}
	}

	return 0x7195BC;
}

DEFINE_HOOK(0x719742, TeleportLocomotionClass_ILocomotion_Process_WarpInAnim, 0x5)
{
	GET_LOCO(ESI);

	if (auto pWarpIn = pExt->WarpIn.Get(RulesClass::Instance->WarpIn))
		GameCreate<AnimClass>(pWarpIn, pLinked->Location)->Owner
		= pLinked->Owner;

	auto const lastWarpDistance = TechnoExt::ExtMap.Find(pLinked)->LastWarpDistance;
	bool isInMinRange = lastWarpDistance < pExt->ChronoRangeMinimum.Get(RulesClass::Instance->ChronoRangeMinimum);

	if (auto const weaponType = isInMinRange ? pExt->WarpInMinRangeWeapon.Get(pExt->WarpInWeapon) : pExt->WarpInWeapon)
	{
		int damage = pExt->WarpInWeapon_UseDistanceAsDamage ? lastWarpDistance / Unsorted::LeptonsPerCell : weaponType->Damage;
		WeaponTypeExt::DetonateAt(weaponType, pLinked, pLinked, damage);
	}

	return 0x719796;
}

DEFINE_HOOK(0x719827, TeleportLocomotionClass_ILocomotion_Process_WarpAway, 0x5)
{
	GET_LOCO(ESI);

	if (auto pWarpAway = pExt->WarpAway.Get(RulesClass::Instance->WarpOut))
		GameCreate<AnimClass>(pWarpAway, pLinked->Location)->Owner = pLinked->Owner;

	return 0x719878;
}

DEFINE_HOOK(0x719973, TeleportLocomotionClass_ILocomotion_Process_ChronoDelay, 0x5)
{
	GET_LOCO(ESI);

	pLinked->ChronoLockRemaining = pExt->ChronoDelay.Get(RulesClass::Instance->ChronoDelay);
	R->EAX(0);

	return 0x719989;
}

#undef GET_LOCO

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
// DEFINE_JUMP(VTABLE, 0x7F5028, 0x5142A0);//TeleportLocomotionClass_Shadow_Matrix : just use hover's to save my time

// Visual bugfix: Tunnel loco could not tilt when being flipped
DEFINE_HOOK(0x729B5D, TunnelLocomotionClass_DrawMatrix_Tilt, 0x8)
{
	GET(ILocomotion*, iloco, ESI);
	GET_BASE(VoxelIndexKey*, pIndex, 0x10);
	GET_BASE(Matrix3D*, ret, 0xC);
	R->EAX(TeleportLocomotionClass_Draw_Matrix(iloco, ret, pIndex));
	return 0x729C09;
}

// DEFINE_JUMP(VTABLE, 0x7F5A4C, 0x5142A0);//TunnelLocomotionClass_Shadow_Matrix : just use hover's to save my time
// Since I've already invalidated the key for tilted vxls when reimplementing the shadow drawing code, this is no longer necessary

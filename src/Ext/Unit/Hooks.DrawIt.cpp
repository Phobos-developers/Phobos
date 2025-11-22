#include <Helpers/Macro.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

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

	const auto pTurretShape = TechnoTypeExt::ExtMap.Find(pType)->TurretShape;
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

	const auto pTurretShape = TechnoTypeExt::ExtMap.Find(pType)->TurretShape;
	const int StartFrame = pTurretShape ? 0 : (pType->WalkFrames * pType->Facings);
	const int frameIdx = pThis->SecondaryFacing.Current().GetFacing<32>(4) + StartFrame;

	if (pTurretShape)
		R->EDI(pTurretShape);

	R->ECX(pThis);
	R->EAX(frameIdx);
	return 0x73CD06;
}

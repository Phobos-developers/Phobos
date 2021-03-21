#include <UnitClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SpawnManagerClass.h>
#include <BulletClass.h>

#include "Body.h"
#include "../BulletType/Body.h"
#include "../Techno/Body.h"

DEFINE_HOOK(6F64A9, HealthBar_Hide, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->HealthBar_Hide) {
		return 0x6F6AB6;
	}
	return 0;
}

DEFINE_HOOK(739956, UnitClass_Deploy_Transfer, 6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	// Vehicle-to-building deployer targeting
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pStructure->GetTechnoType());
	if (pTypeData && pTypeData->Deployed_RememberTarget)
	{ // && pUnit->Target > 0)
		pStructure->Target = pUnit->Target;
	}

	return 0;
}

DEFINE_HOOK(6F9E50, TechnoClass_Update, 5)
{
	GET(TechnoClass*, pThis, ECX);

	// MindControlRangeLimit
	TechnoTypeExt::ApplyMindControlRangeLimit(pThis);
	// BuildingDeployerTargeting
	TechnoTypeExt::ApplyBuildingDeployerTargeting(pThis);
	// Interceptor
	TechnoTypeExt::ApplyInterceptor(pThis);
	// Powered.KillSpawns
	TechnoTypeExt::ApplyPowered_KillSpawns(pThis);
	// Spawner.LimitRange & Spawner.ExtraLimitRange
	TechnoTypeExt::ApplySpawn_LimitRange(pThis);

	return 0;
}

DEFINE_HOOK(6F3C56, TechnoClass_Transform_6F3AD0_TurretMultiOffset, 0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xD8, 0x90));
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3C6D;
}

DEFINE_HOOK(6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0xCC, 0x90));
	GET(TechnoTypeClass*, technoType, EBP);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3E85;
}

DEFINE_HOOK(73B780, UnitClass_DrawVXL_TurretMultiOffset, 0)
{
	GET(TechnoTypeClass*, technoType, EAX);

	auto const pTypeData = TechnoTypeExt::ExtMap.Find(technoType);

	if (pTypeData && *pTypeData->TurretOffset.GetEx() == CoordStruct{ 0, 0, 0 }) {
		return 0x73B78A;
	}

	return 0x73B790;
}

DEFINE_HOOK(73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFS(0x1D0, 0x13C));
	GET(TechnoTypeClass*, technoType, EBX);

	double& factor = *reinterpret_cast<double*>(0xB1D008);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, factor);

	return 0x73BA68;
}

DEFINE_HOOK(73C890, UnitClass_Draw_1_TurretMultiOffset, 0)
{
	LEA_STACK(Matrix3D*, mtx, 0x80);
	GET(TechnoTypeClass*, technoType, EAX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1/8);

	return 0x73C8B7;
}

DEFINE_HOOK(43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0)
{
	LEA_STACK(Matrix3D*, mtx, 0x60);
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1 / 8);

	return 0x43E0E8;
}
#include <UnitClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SpawnManagerClass.h>
#include <BulletClass.h>

#include "Body.h"
#include "../BulletType/Body.h"
#include "../Techno/Body.h"
#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(6F64A9, TechnoClass_DrawHealthBar_Hide, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->HealthBar_Hide) {
		return 0x6F6AB6;
	}
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

DEFINE_HOOK(6B7282, SpawnManagerClass_AI_PromoteSpawns, 5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	//auto Owner = pThis->Owner;

	for (auto i : pThis->SpawnedNodes)
	{
		auto spawn = i->Unit;
		if (spawn)
		{
			if (pTypeExt->Promote_IncludeSpawns)
			{
				if (spawn->Veterancy.Veterancy < pThis->Owner->Veterancy.Veterancy)
					spawn->Veterancy.Add(pThis->Owner->Veterancy.Veterancy - spawn->Veterancy.Veterancy);
			}

			//auto lastMission = Owner->GetCurrentMission();
			/**/
			if (spawn->Target)
			{
				auto what = spawn->Target->WhatAmI();
				if (what == AbstractType::Unit
					|| what == AbstractType::Infantry
					|| what == AbstractType::Building
					|| what == AbstractType::Aircraft
					)
				{
					if (auto tech = static_cast<TechnoClass*>(spawn->Target))
					{
						auto WeaponType = spawn->Veterancy.IsElite() ?
							spawn->GetTechnoType()->EliteWeapon[0].WeaponType :
							spawn->GetTechnoType()->Weapon[0].WeaponType;

						auto damage = WeaponType->Damage *
							GeneralUtils::GetWarheadVersusArmor(WeaponType->Warhead, static_cast<int>(tech->GetTechnoType()->Armor));

						if (static_cast<int>(damage) < 0)
						{
							if (tech->GetHealthPercentage() == RulesClass::Instance->unknown_double_16F8)//condition green
							{
								/*
								if (Owner->Type()->AttackFriendlies)
								{
									Owner->Target = nullptr;

									//reset mission after it nulled target
									Owner->QueueMission(lastMission, 1);
									Owner->NextMission();
								}*/

								pThis->ResetTarget();
							}
						}
					}
				}
			}
		}
	}

	return 0;
}
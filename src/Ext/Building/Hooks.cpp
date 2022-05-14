#include "Body.h"

#include <BulletClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BitFont.h>
#include <Misc/FlyingStrings.h>

DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	if (auto pStructureExt = BuildingExt::ExtMap.Find(pStructure))
		pStructureExt->DeployedTechno = true;

	return 0;
}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x0)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = BuildingExt::ExtMap.Find(pThis);
	if (pExt && pExt->DeployedTechno && pThis->LastTarget)
	{
		pThis->Target = pThis->LastTarget;
		pThis->QueueMission(Mission::Attack, false);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
	}

	return 0x449AE8;
}

DEFINE_HOOK(0x4401BB, Factory_AI_PickWithFreeDocks, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (Phobos::Config::AllowParallelAIQueues)
		return 0;

	if (!pBuilding)
		return 0;

	HouseClass* pOwner = pBuilding->Owner;

	if (!pOwner)
		return 0;

	if (pOwner->Type->MultiplayPassive
		|| pOwner->IsPlayer()
		|| pOwner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExt::HasFreeDocks(pBuilding))
		{
			auto BuildingExt = BuildingExt::ExtMap.Find(pBuilding);
			if (!BuildingExt)
				return 0;

			BuildingExt::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFS(0xF0, 0xA4));

	pBullet->SetWeaponType(pWeapon);

	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int iPos = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pTypeExt->RandomProduct[iPos]);
		UnitClass* pNewUnit = static_cast<UnitClass*>(pType->CreateObject(pUnit->GetOwningHouse()));
		pNewUnit->Limbo();
		pNewUnit->Unlimbo(pUnit->Location, Direction::SouthEast);
		pUnit->Limbo();
		pUnit->UnInit();
		R->EDI(pNewUnit);
		pUnit = pNewUnit;
	}

	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	GET(InfantryClass*, pInf, EDI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int iPos = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pTypeExt->RandomProduct[iPos]);
		InfantryClass* pNewInf = static_cast<InfantryClass*>(pType->CreateObject(pHouse));
		pInf->Limbo();
		pInf->UnInit();
		R->EDI(pNewInf);
		pInf = pNewInf;
  }
  
  return 0;
}

DEFINE_HOOK(0x43FE73, BuildingClass_AI_FlyingStrings, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (auto const pExt = BuildingExt::ExtMap.Find(pThis))
	{
		if (Unsorted::CurrentFrame % 15 == 0 && pExt->AccumulatedGrindingRefund)
		{
			auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

			int refundAmount = pExt->AccumulatedGrindingRefund;
			bool isPositive = refundAmount > 0;
			auto color = isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 };
			wchar_t moneyStr[0x20];
			swprintf_s(moneyStr, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(refundAmount));

			auto coords = CoordStruct::Empty;
			coords = *pThis->GetCenterCoord(&coords);

			int width = 0, height = 0;
			BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);

			Point2D pixelOffset = Point2D::Empty;
			pixelOffset += pTypeExt->Grinding_DisplayRefund_Offset;
			pixelOffset.X -= width / 2;

			FlyingStrings::Add(moneyStr, coords, color, pixelOffset);

			pExt->AccumulatedGrindingRefund = 0;
		}
	}

	return 0;
}
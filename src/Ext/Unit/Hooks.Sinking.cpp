#include <AircraftClass.h>

#include <ScenarioClass.h>
#include <TunnelLocomotionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x7364DC, UnitClass_Update_SinkSpeed, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(int, CoordZ, EDX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->EDX(CoordZ - (pTypeExt->SinkSpeed - 5));
	return 0;
}

DEFINE_HOOK(0x737DE2, UnitClass_ReceiveDamage_Sinkable, 0x6)
{
	enum { GoOtherChecks = 0x737E18, NoSink = 0x737E63 };

	GET(UnitTypeClass*, pType, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	bool ShouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;

	return pTypeExt->Sinkable.Get(ShouldSink) ? GoOtherChecks : NoSink;
}

DEFINE_HOOK(0x629C7C, ParasiteClass_UpdateSquid_SinkableBySquid, 0xA)
{
	enum { ret = 0x629C86 };

	GET(FootClass*, pVictim, EDI);

	if (pVictim->WhatAmI() != AbstractType::Unit)
	{
		pVictim->Stun();
		return ret;
	}

	auto pVictimType = pVictim->GetTechnoType();
	auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(pVictimType);

	if (pVictimTypeExt->SinkableBySquid)
	{
		pVictim->Stun();
	}
	else
	{
		pVictim->IsSinking = false;
		pVictim->Health = 0;
		pVictim->IsAlive = false;
		auto location = pVictim->Location;
		auto location2D = Point2D({ pVictim->Location.X,pVictim->Location.Y });

		if (pVictim->GetHeight() <= 10
		  && pVictim->IsABomb
		  && (MapClass::Instance.GetTargetCell(location2D)->LandType == LandType::Water))
		{
			GameCreate<AnimClass>(RulesClass::Instance->Wake, location, 0, 1, 1536, 0, 0);
			GameCreate<AnimClass>(RulesClass::Instance->SplashList.Items[RulesClass::Instance->SplashList.Count - 1], location, 0, 1, 1536, 0, 0);
		}
		else
		{
			((UnitClass*)pVictim)->Explode();
		}
	}

	return ret;
}

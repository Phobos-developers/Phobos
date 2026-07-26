
#include <Ext/Unit/Body.h>

DEFINE_HOOK(0x7364DC, UnitClass_Update_SinkSpeed, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(const int, CoordZ, EDX);

	auto const pTypeExt = UnitTypeExt::Fetch(pThis->Type);
	R->EDX(CoordZ - (pTypeExt->SinkSpeed.Get(RulesExt::Global()->SinkSpeed) - 5));
	return 0;
}

DEFINE_HOOK(0x737DE2, UnitClass_ReceiveDamage_Sinkable, 0x6)
{
	enum { GoOtherChecks = 0x737E18, NoSink = 0x737E63 };

	GET(UnitTypeClass*, pType, EAX);

	auto const pTypeExt = UnitTypeExt::Fetch(pType);
	const bool shouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;

	return pTypeExt->Sinkable.Get(RulesExt::Global()->Sinkable.Get(shouldSink)) ? GoOtherChecks : NoSink;
}

DEFINE_HOOK(0x629C67, ParasiteClass_UpdateSquid_SinkableBySquid, 0x9)
{
	enum { ret = 0x629C86 };

	GET(ParasiteClass*, pThis, ESI);
	GET(FootClass*, pVictim, EDI);

	const auto pVictimExt = TechnoExt::Fetch(pVictim);
	const auto pOwner = pThis->Owner;
	const bool isUnit = pVictim->WhatAmI() == AbstractType::Unit;

	if ((isUnit && static_cast<UnitExt*>(pVictimExt)->GetTypeExtData()->Sinkable_SquidGrab.Get(RulesExt::Global()->Sinkable_SquidGrab)) || !isUnit)
	{
		pVictim->IsSinking = true;
		pVictim->Destroyed(pOwner);
		pVictim->Stun();
	}
	else
	{
		auto damage = pVictimExt->TypeExtData->OwnerObject()->Strength;
		pVictim->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, pOwner, true, false, pOwner->Owner);
	}

	return ret;
}

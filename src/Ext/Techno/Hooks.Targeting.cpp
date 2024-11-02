#include "Body.h"

#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <TerrainClass.h>

#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x6FA697, TechnoClass_Update_DontScanIfUnarmed, 0x6)
{
	enum { SkipTargeting = 0x6FA6F5, DoTargeting = 0 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->IsArmed())
		return DoTargeting;
	else
		return SkipTargeting;
}

DEFINE_HOOK(0x709866, TechnoClass_TargetAndEstimateDamage_ScanDelayGuardArea, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pOwner = pThis->Owner;
	auto const pRulesExt = RulesExt::Global();
	auto const pRules = RulesClass::Instance();
	int delay = 1;

	if (pOwner->IsHumanPlayer || pOwner->IsControlledByHuman())
	{
		delay = pTypeExt->PlayerGuardAreaTargetingDelay.Get(pRulesExt->PlayerGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay));
	}
	else
	{
		delay = pTypeExt->AIGuardAreaTargetingDelay.Get(pRulesExt->AIGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay));
	}

	R->ECX(delay);
	return 0;
}

DEFINE_HOOK(0x70989C, TechnoClass_TargetAndEstimateDamage_ScanDelayNormal, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pOwner = pThis->Owner;
	auto const pRulesExt = RulesExt::Global();
	auto const pRules = RulesClass::Instance();
	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);

	if (pOwner->IsHumanPlayer || pOwner->IsControlledByHuman())
	{
		delay += pTypeExt->PlayerNormalTargetingDelay.Get(pRulesExt->PlayerNormalTargetingDelay.Get(pRules->NormalTargetingDelay));
	}
	else
	{
		delay += pTypeExt->AINormalTargetingDelay.Get(pRulesExt->AINormalTargetingDelay.Get(pRules->NormalTargetingDelay));
	}

	R->ECX(delay);

	return 0;
}

DEFINE_HOOK(0x6FA67D, TechnoClass_Update_DistributeTargetingFrame, 0xA)
{
	enum { Targeting = 0x6FA687, SkipTargeting = 0x6FA6F5 };
	GET(TechnoClass* const, pThis, ESI);

	auto const pRulesExt = RulesExt::Global();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	bool allowed = !pThis->Owner->IsControlledByHuman() ? true : !pRulesExt->DistributeTargetingFrame_AIOnly;

	if (allowed && pTypeExt && pTypeExt->DistributeTargetingFrame.Get(pRulesExt->DistributeTargetingFrame))
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt && Unsorted::CurrentFrame % 16 != pExt->MyTargetingFrame)
		{
			return SkipTargeting;
		}
	}

	R->EAX(pThis->vt_entry_4C4());
	return Targeting;
}

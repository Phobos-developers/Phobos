#include "Body.h"

#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <TerrainClass.h>

#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>


DEFINE_HOOK(0x4DF410, FootClass_UpdateAttackMove_TargetAcquired, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt)
	{
		bool DefaultValue = RulesExt::Global()->AttackMove_StopWhenTargetAcquired_UseOpportunityFireAsDefault ? !pType->OpportunityFire : false;

		if (pTypeExt->AttackMove_StopWhenTargetAcquired.Get(DefaultValue))
		{
			pThis->StopMoving();
			pThis->AbortMotion();
		}
	}

	return 0;
}


#include <Utilities/Macro.h>
#include <ScenarioClass.h>
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/ParticleSystemType/Body.h>

DEFINE_HOOK(0x51A3A2, InfantryClass_PerCellProcess_CyborgLegsCheck, 0x5)
{
	GET(InfantryClass* const, pThis, ESI);
	GET(TechnoClass* const, pTransport, EDI);

	// Note: "SequenceAnim" and "Crawling" values will suffer a reset before entering into transports.
	// The Cyborg legs state will be saved for the unlimbo case
	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->IsLeglessCyborg = true;

		if (auto const pTransportTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			if (pTransportTypeExt->Transporter_FixCyborgLegs)
				pExt->IsLeglessCyborg = false;
		}
	}

	return 0;
}

DEFINE_HOOK(0x518047, InfantryClass_TakeDamage_CyborgLegsCheck, 0x5)
{
	GET(InfantryClass*, pInf, ESI);
	GET(DamageState, eDamageState, EAX);

	if (pInf->Type->Cyborg
		&& eDamageState != DamageState::PostMortem
		&& pInf->Crawling == true)
	{
		if (auto pExt = TechnoExt::ExtMap.Find(pInf))
			pExt->IsLeglessCyborg = true;
	}

	return 0;
}

#include <Utilities/Macro.h>
#include <ScenarioClass.h>
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/ParticleSystemType/Body.h>

DEFINE_HOOK(0x51A3A2, InfantryClass_PerCellProcess_CyborgLegsCheck, 0x5)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(TechnoClass*, pTransport, EDI);

	if (pTechno->WhatAmI() == AbstractType::Infantry)
	{
		auto const pInf = static_cast<InfantryClass*>(pTechno);

		// "SequenceAnim" and "Crawling" values will suffer a reset before entering into transports
		// The Cyborg legs state will be saved for the unlimbo case
		if (pInf->Type->Cyborg
			&& pInf->Crawling
			&& (pInf->SequenceAnim == Sequence::Prone || pInf->SequenceAnim == Sequence::Crawl))
		{
			auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
			pTechnoExt->IsLeggedCyborg = true;

			if (auto const pTransportTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
			{
				if (pTransportTypeExt->Transporter_FixCyborgLegs)
					pTechnoExt->IsLeggedCyborg = false;
			}
		}
	}

	return 0;
}

#include <Utilities/Macro.h>
#include <ScenarioClass.h>
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/ParticleSystemType/Body.h>

DEFINE_HOOK(0x51DF42, InfantryClass_Limbo_Cyborg, 0x7)
{
	enum { SkipReset = 0x51DF53 };

	GET(InfantryClass*, pThis, ESI);

	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		if (pThis->Transporter)
		{
			// Note: When infantry enters into a transport this Limbo will be executed 2 times, in the second run of this hook infaatry will contain information of the transport unit (Transporter variable)
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Transporter->GetTechnoType());

			if (pTypeExt->Transporter_FixCyborgLegs)
				return 0;
		}

		return SkipReset;
	}

	return 0;
}

// When infantry enters into structures (not executed in Ares Tunnel logic)
DEFINE_HOOK(0x52291A, InfantryClass_InfantryEnteredThing_Cyborg, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	REF_STACK(TechnoClass*, pBuilding, 0x1C);

	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType());

		if (pTypeExt->Transporter_FixCyborgLegs)
			pThis->Crawling = false;
	}

	return 0;
}

// When infantry enters into a structure with "tunnel logic" created by Ares
DEFINE_HOOK(0x51A27F, InfantryClass_PerCellProcess_AresTunnel_Cyborg, 0xA)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->Type);

		if (pTypeExt->Transporter_FixCyborgLegs)
			pThis->Crawling = false;
	}

	return 0;
}

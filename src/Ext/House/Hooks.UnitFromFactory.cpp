#include <FactoryClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>
#include <Ext/Techno/Body.h>
#include "Body.h"

DEFINE_HOOK(0x4FB63A, HouseClass_UnitFromFactory_DisablingEVAUnitReady, 0xF)
{
	if (!RulesExt::Global()->IsVoiceCreatedGlobal.Get())
		VoxClass::Play(GameStrings::EVA_UnitReady);

	return 0x4FB649;
}

DEFINE_HOOK(0x4FB64B, HouseClass_UnitFromFactory_VoiceCreated, 0x5)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(TechnoClass* const, pThisTechno, ESI);
	GET(FactoryClass* const, pThisFactory, EBX);

	auto const pTechnoExt = TechnoExt::ExtMap.Find(pThisTechno);
	auto const pTechnoTypeExt = pTechnoExt->TypeExtData;

	auto const pHouseExt = HouseExt::ExtMap.Find(pThisFactory->Owner);

	if (pThisTechno->Owner->IsControlledByCurrentPlayer() && pTechnoTypeExt->VoiceCreated.isset())
	{
		if (RulesExt::Global()->IsVoiceCreatedGlobal.Get())
			pThisTechno->QueueVoice(pTechnoTypeExt->VoiceCreated);
		else
			VocClass::PlayAt(pTechnoTypeExt->VoiceCreated, pThisTechno->Location);
	}

	pThisFactory->CompletedProduction();

	auto const pBuildingExt = TechnoExt::ExtMap.Find(pBuilding);

	static std::map<EventActorType, AbstractClass*> participants;
	participants[EventActorType::Me] = pBuilding;
	participants[EventActorType::They] = pThisTechno;
	pBuildingExt->InvokeEvent(EventTypeClass::WhenProduce, &participants);

	participants[EventActorType::Me] = pThisTechno;
	participants[EventActorType::They] = pBuilding;
	pTechnoExt->InvokeEvent(EventTypeClass::WhenProduced, &participants);

	participants[EventActorType::Me] = pBuilding;
	participants[EventActorType::They] = pThisTechno;
	pHouseExt->InvokeEvent(EventTypeClass::WhenProduce, &participants);

	return 0x4FB650;
}

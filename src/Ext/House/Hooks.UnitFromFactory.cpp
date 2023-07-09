#include <FactoryClass.h>

#include <Ext/TechnoType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x4FB64B, HouseClass_UnitFromFactory_VoiceCreated, 0x5)
{
	GET(TechnoClass* const, pThisTechno, ESI);
	GET(FactoryClass* const, pThisFactory, EBX);

	auto const pThisTechnoType = TechnoTypeExt::ExtMap.Find(pThisTechno->GetTechnoType());
	if(pThisTechno->Owner->IsControlledByCurrentPlayer() && pThisTechnoType->VoiceCreated.isset())
		VocClass::PlayAt(pThisTechnoType->VoiceCreated, pThisTechno->Location);

	pThisFactory->CompletedProduction();
	return 0x4FB650;
}

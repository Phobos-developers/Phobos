#include <HouseClass.h>
#include <Utilities/Macro.h>

#include "Ext/House/Body.h"
#include "Ext/Techno/Body.h"
#include "New/Entity/StorageClassExt.h"

DEFINE_FUNCTION_JUMP(LJMP, 0x6C9600, StorageClassExt::GetTotalValue)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C9650, StorageClassExt::GetTotalAmount)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C9680, StorageClassExt::GetAmount)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C9690, StorageClassExt::IncreaseAmount)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C96B0, StorageClassExt::DecreaseAmount)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C9740, StorageClassExt::operator+=)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C97E0, StorageClassExt::operator-=)
DEFINE_FUNCTION_JUMP(LJMP, 0x6C9820, StorageClassExt::FirstUsedSlot)

// Initialize our new storage class inside the owner objects
// This patch is placed after SwizzleManager.Reset() happens
// It is important to do it now because to do this we need the
// owner object - extension pairs to be restored,
// which is not the case while they are being loaded
DEFINE_HOOK(0x6851DF, PostGameLoad, 0x5)
{
	for (auto pTechno : TechnoClass::Array)
	{
		auto pExtension = TechnoExt::ExtMap.Find(pTechno);
		new (&pTechno->Tiberium) StorageClassExt(pExtension->Tiberium);
	}
	for (auto pHouse : HouseClass::Array)
	{
		auto pExtension = HouseExt::ExtMap.Find(pHouse);
		new (&pHouse->OwnedTiberium) StorageClassExt(pExtension->Tiberium);
		new (&pHouse->OwnedWeed) StorageClassExt(pExtension->Weed);
	}

	return 0x660C50;
}

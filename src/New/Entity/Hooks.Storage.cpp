#include <Utilities/Macro.h>
#include "New/Entity/PhobosStorageClass.h"

DEFINE_JUMP(LJMP, 0x6C9600, GET_PTR(&PhobosStorageClass::GetTotalValue))
DEFINE_JUMP(LJMP, 0x6C9650, GET_PTR(&PhobosStorageClass::GetTotalAmount))
DEFINE_JUMP(LJMP, 0x6C9680, GET_PTR(&PhobosStorageClass::GetAmount))
DEFINE_JUMP(LJMP, 0x6C9690, GET_PTR(&PhobosStorageClass::IncreaseAmount))
DEFINE_JUMP(LJMP, 0x6C96B0, GET_PTR(&PhobosStorageClass::DecreaseAmount))
DEFINE_JUMP(LJMP, 0x6C9740, GET_PTR(&PhobosStorageClass::operator+=))
DEFINE_JUMP(LJMP, 0x6C97E0, GET_PTR(&PhobosStorageClass::operator-=))
DEFINE_JUMP(LJMP, 0x6C9820, GET_PTR(&PhobosStorageClass::FirstUsedSlot))

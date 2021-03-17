#include "Swizzle.h"

#include "../Phobos.h"

#include <SwizzleManagerClass.h>

PhobosSwizzle PhobosSwizzle::Instance;

HRESULT PhobosSwizzle::RegisterForChange(void** p) {
	SwizzleManagerClass::Instance.Swizzle(p);
}

HRESULT PhobosSwizzle::RegisterChange(void* was, void* is) {
	SwizzleManagerClass::Instance.Here_I_Am((long)was, is);
}
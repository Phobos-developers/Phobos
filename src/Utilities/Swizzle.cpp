#include "Swizzle.h"

#include <Phobos.h>

#include <SwizzleManagerClass.h>

PhobosSwizzle PhobosSwizzle::Instance;

HRESULT PhobosSwizzle::RegisterForChange(void** p)
{
	return SwizzleManagerClass::Instance().Swizzle(p);
}

HRESULT PhobosSwizzle::RegisterChange(void* was, void* is)
{
	return SwizzleManagerClass::Instance().Here_I_Am((long)was, is);
}

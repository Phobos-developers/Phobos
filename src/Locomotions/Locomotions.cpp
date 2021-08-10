#include "Locomotions.h"

#include "DummyLocomotor.h"

#include <Helpers/Macro.h>

auto Locomotors = LocomotorCollection<

>();

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
{
	*ppvOut = nullptr;
	
	return Locomotors.GetClassObject(rclsid, riid, ppvOut);
}

DEFINE_HOOK(0x6BD42B, WinMain_CoRegisterClassObjects_Locomotions, 0x6)
{
	GET_STACK(DWORD*, lpdwRegister, STACK_OFFS(0xE3C, 0xA34));
	GET(IUnknown*, pUkn, EAX);

	Locomotors.CoRegisterClassObject(pUkn, lpdwRegister);

	return 0;
}

//
// PhobosLocomotorFactory
//
PhobosLocomotorFactory::PhobosLocomotorFactory()
{
	RefCount = 0L;
}

PhobosLocomotorFactory::~PhobosLocomotorFactory()
{
	
}

STDMETHODIMP PhobosLocomotorFactory::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
	if (!ppv)	return E_POINTER;
	*ppv = nullptr;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
	{
		*ppv = (LPCLASSFACTORY)this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) PhobosLocomotorFactory::AddRef()
{
	return InterlockedIncrement(&RefCount);
}

STDMETHODIMP_(ULONG) PhobosLocomotorFactory::Release()
{
	if (InterlockedDecrement(&RefCount))
		return RefCount;

	GameDelete(this);

	return 0L;
}

STDMETHODIMP PhobosLocomotorFactory::LockServer(BOOL fLock)
{
	RefCount = fLock ? RefCount + 1 : RefCount - 1;
	return NOERROR;
}
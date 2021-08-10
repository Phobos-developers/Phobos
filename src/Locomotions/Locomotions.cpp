#include "Locomotions.h"

#include "DummyLocomotor.h"

namespace Locomotions
{
	UINT g_cRefThisDll = 0;
}

template <typename...>
struct DummyTypes { };

struct GetObjectHelper
{
	template <typename T>
	static HRESULT Process(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
	{
		return getObject<T>(rclsid, riid, ppvOut);
	}

private:
	template <typename T>
	static HRESULT getObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
	{
		*ppvOut = nullptr;

		if (IsEqualIID(rclsid, T::Locomotion::ClassID))
		{
			T* pcf = GameCreate<T>();
			return pcf->QueryInterface(riid, ppvOut);
		}

		return CLASS_E_CLASSNOTAVAILABLE;
	}
};

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... Ts>
struct LocomotorCollection
{
public:
	__forceinline HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
	{
		return process<GetObjectHelper>(DummyTypes<Ts...>(), rclsid, riid, ppvOut);
	}

private:
	// T: the method dispatcher class to call with each type
	// TArgs: the arguments to call the method dispatcher's Process() method
	// TType and TTypes: setup for recursion. TType is the first type, the one
	// to handle now. TTypes is the tail that is recursively went into.

	// this is the base case, no more types, nothing to call
	template <typename T, typename... TArgs>
	HRESULT process(DummyTypes<>, TArgs... args)
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// this is the recursion part: invoke T:Process() for first type, then
	// recurse with the remaining types
	template <typename T, typename... TArgs, typename TType, typename... TTypes>
	__forceinline HRESULT process(DummyTypes<TType, TTypes...>, TArgs... args)
	{
		HRESULT hr = T::Process<TType>(args...);
		if (hr != CLASS_E_CLASSNOTAVAILABLE)
			return hr;

		return process<T>(DummyTypes<TTypes...>(), args...);
	}
};

auto Locomotors = LocomotorCollection<
	DummyLocomotorFactory
>();

STDAPI DllCanUnloadNow(VOID)
{
	return (Locomotions::g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
{
	*ppvOut = nullptr;
	
	return Locomotors.GetClassObject(rclsid, riid, ppvOut);
}

/*
const LPSTR c_lpName = "Phobos Locomotion";
const LPSTR c_lpTModel = "Apartment";

STDAPI DllRegisterServer(VOID)
{
	HKEY hKey;
	CHAR szTemp[MAX_PATH];

	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "CLSID\\{0C2F47D2-34F5-445a-A38A-D66C70329646}", 0, NULL, 0,
		KEY_SET_VALUE, NULL, &hKey, NULL) != ERROR_SUCCESS)
	{
		return SELFREG_E_CLASS;
	}
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)c_lpName, strlen(c_lpName) + 1) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return SELFREG_E_CLASS;
	}
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "CLSID\\{0C2F47D2-34F5-445a-A38A-D66C70329646}\\InprocServer32", 0, NULL, 0,
		KEY_SET_VALUE, NULL, &hKey, NULL) != ERROR_SUCCESS)
	{
		return SELFREG_E_CLASS;
	}
	if (RegSetValueEx(hKey, "ThreadingModel", 0, REG_SZ, (LPBYTE)c_lpTModel, strlen(c_lpTModel) + 1) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return SELFREG_E_CLASS;
	}
	if (GetModuleFileName(hInstance, szTemp, MAX_PATH) == 0)
	{
		RegCloseKey(hKey);
		return SELFREG_E_CLASS;
	}
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szTemp, strlen(szTemp) + 1) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return SELFREG_E_CLASS;
	}
	RegCloseKey(hKey);
	return S_OK;
}

STDAPI DllUnregisterServer(VOID)
{
	if (SHDeleteKey(HKEY_CLASSES_ROOT, "CLSID\\{0C2F47D2-34F5-445a-A38A-D66C70329646}") != ERROR_SUCCESS)
	{
		return SELFREG_E_CLASS;
	}
	return S_OK;
}
*/

//
// PhobosLocomotorFactory
//
PhobosLocomotorFactory::PhobosLocomotorFactory()
{
	m_cRef = 0L;
	++Locomotions::g_cRefThisDll;
}

PhobosLocomotorFactory::~PhobosLocomotorFactory()
{
	--Locomotions::g_cRefThisDll;
}

STDMETHODIMP PhobosLocomotorFactory::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
	*ppv = nullptr;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
	{
		*ppv = (LPCLASSFACTORY)this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) PhobosLocomotorFactory::AddRef()
{
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) PhobosLocomotorFactory::Release()
{
	if (--m_cRef)
		return m_cRef;

	GameDelete(this);

	return 0L;
}

STDMETHODIMP PhobosLocomotorFactory::LockServer(BOOL fLock)
{
	return NOERROR;
}
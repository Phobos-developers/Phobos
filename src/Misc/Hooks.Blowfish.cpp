#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <GameStrings.h>

HRESULT __stdcall Blowfish_Loader(
	REFCLSID  rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD     dwClsContext,
	REFIID    riid,
	LPVOID* ppv
)
{
	typedef HRESULT(__stdcall* pDllGetClassObject)(const IID&, const IID&, IClassFactory**);

	auto result = REGDB_E_KEYMISSING;

	// First, let's try to run the vanilla function
	result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (SUCCEEDED(result))
		return result;

	HMODULE hDll = LoadLibrary(GameStrings::BLOWFISH_DLL);
	if (hDll)
	{
		auto GetClassObject = (pDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
		if (GetClassObject)
		{

			IClassFactory* pIFactory;
			result = GetClassObject(rclsid, IID_IClassFactory, &pIFactory);

			if (SUCCEEDED(result))
			{
				result = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
				pIFactory->Release();
			}
		}
	}

	if (!SUCCEEDED(result))
	{
		FreeLibrary(hDll);

		Debug::FatalErrorAndExit("File Blowfish.dll was not found\n");
	}

	return result;
}

DEFINE_JUMP(CALL6, 0x6BEDDD, GET_OFFSET(Blowfish_Loader))
DEFINE_JUMP(CALL6, 0x437F6E, GET_OFFSET(Blowfish_Loader))

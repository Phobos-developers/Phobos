#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

HRESULT __stdcall Blowfish_Loader(
	REFCLSID  rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD	 dwClsContext,
	REFIID	riid,
	LPVOID* ppv)
{
	typedef HRESULT(__stdcall *pDllGetClassObject)(const IID&, const IID&, IClassFactory**);

	auto result = REGDB_E_KEYMISSING;

	// First, let's try to run the vanilla function
	result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (SUCCEEDED(result))
		return result;

	HMODULE hDll = LoadLibrary("Blowfish.dll");
	if (hDll) {
		auto GetClassObject = (pDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
		if (GetClassObject) {

			IClassFactory* pIFactory;
			result = GetClassObject(rclsid, IID_IClassFactory, &pIFactory);

			if (SUCCEEDED(result)) {
				result = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
				pIFactory->Release();
			}
		}
	}

	if (!SUCCEEDED(result)) {
		FreeLibrary(hDll);

		char* Message = "File Blowfish.dll was not found\n";
		MessageBox(0, Message, "Fatal error ", MB_ICONERROR);
		Debug::FatalErrorAndExit(Message);
	}

	return result;
}

DEFINE_NAKED_LJMP(0x6BEDDD, _Blowfish_Loader_Init) {
	CALL(Blowfish_Loader);
	JMP(0x6BEDE3);
}

DEFINE_NAKED_LJMP(0x437F6E, _Blowfish_Loader_Create) {
	CALL(Blowfish_Loader);
	JMP(0x437F74);
}

#include <Utilities/Debug.h>
#include <YRCom.h>

// Registers a manually created factory for a class.
template<typename T>
void RegisterFactoryForClass(IClassFactory* pFactory)
{
	DWORD dwRegister = 0;
	HRESULT hr = CoRegisterClassObject(__uuidof(T), pFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister);

	if (FAILED(hr))
		Debug::Log("CoRegisterClassObject for %s class factory failed with error code %d.\n", typeid(T).name(), GetLastError());
	else
		Debug::Log("Class factory for %s registered.\n", typeid(T).name());

	Game::COMClasses->AddItem((ULONG)dwRegister);
}

// Registers an automatically created factory for a class.
template<typename T>
void RegisterFactoryForClass()
{
	RegisterFactoryForClass<T>(GameCreate<TClassFactory<T>>());
}

#include "Dummy.h"

#include <Utilities/Debug.h>

const char* DummyCommandClass::GetName() const
{
	return "Phobos Dummy";
}

const wchar_t* DummyCommandClass::GetUIName() const
{
	return L"Phobos Dummy";
}

const wchar_t* DummyCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT
}

const wchar_t* DummyCommandClass::GetUIDescription() const
{
	return L"Dummy";
}

void DummyCommandClass::Execute(WWKey eInput) const
{
	Debug::Log("[Phobos] Dummy command runs.\n");
	MessageListClass::Instance.PrintMessage(L"[Phobos] Dummy command runs.");
}

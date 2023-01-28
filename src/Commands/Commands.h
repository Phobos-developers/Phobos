#pragma once

#include <CommandClass.h>
#include <StringTable.h>
#include <MessageListClass.h>
#include <Phobos.h>

#include <Utilities/Debug.h>

class PhobosCommandClass : public CommandClass
{
protected:
	bool CheckDebugDeactivated() const
	{
		if (!Phobos::Config::DevelopmentCommands)
		{
			if (const wchar_t* text = StringTable::LoadString("TXT_COMMAND_DISABLED"))
			{
				wchar_t msg[0x100] = L"\0";
				wsprintfW(msg, text, this->GetUIName());
				MessageListClass::Instance->PrintMessage(msg);
			}
			return true;
		}
		return false;
	}
};

template <typename T>
void MakeCommand()
{
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};

#define CATEGORY_TEAM StringTable::LoadString("TXT_TEAM")
#define CATEGORY_INTERFACE StringTable::LoadString("TXT_INTERFACE")
#define CATEGORY_TAUNT StringTable::LoadString("TXT_TAUNT")
#define CATEGORY_SELECTION StringTable::LoadString("TXT_SELECTION")
#define CATEGORY_CONTROL StringTable::LoadString("TXT_CONTROL")
#define CATEGORY_DEBUG L"Debug"
#define CATEGORY_GUIDEBUG StringTable::LoadString("GUI:Debug")
#define CATEGORY_DEVELOPMENT StringTable::LoadString("TXT_DEVELOPMENT")

#pragma once

#include <YRPP.h>
#include <CommandClass.h>

#include "../Phobos.h"

class PhobosCommandClass : public CommandClass
{
protected:
	bool CheckDebugDeactivated() const {
		if (!Phobos::Config::DevelopmentCommands) {
			if (const wchar_t* text = StringTable::LoadString("TXT_COMMAND_DISABLED")) {
				wchar_t msg[0x100] = L"\0";
				wsprintfW(msg, text, this->GetUIName());
				MessageListClass::Instance->PrintMessage(msg);
			}
			return true;
		}
		return false;
	}
};

// will the templates ever stop? :D
template <typename T>
void MakeCommand() {
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};


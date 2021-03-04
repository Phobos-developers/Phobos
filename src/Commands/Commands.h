#pragma once

#include <YRPP.h>
#include <CommandClass.h>

class PhobosCommandClass : public CommandClass
{

};

// will the templates ever stop? :D
template <typename T>
void MakeCommand() {
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};


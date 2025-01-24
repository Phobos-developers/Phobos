#include "PlayerEmblemTypeClass.h"

template<>

const char* Enumerable<PlayerEmblemTypeClass>::GetMainSection()
{
	return "PlayerEmblemTypes";
}

bool PlayerEmblemTypeClass::AlterBuildOptions() const
{
	return !this->BuildOptions_Allow.empty()
		|| !this->BuildOptions_Disallow.empty();
}

void PlayerEmblemTypeClass::InvokeEventHandlers(EventTypeClass* pEventType, HouseClass* pHouse) const
{
	if (this->EventHandlersMap.contains(pEventType))
	{
		std::map<EventActorType, AbstractClass*> participants = {
			{ EventActorType::Me, pHouse },
		};
		for (auto pEventHandlerTypeClass : this->EventHandlersMap.get_or_default(pEventType))
		{
			pEventHandlerTypeClass->HandleEvent(&participants);
		}
	}
}

void PlayerEmblemTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	// Event Handler
	EventHandlerTypeClass::LoadTypeMapFromINI(exINI, pSection, "Trigger", &this->EventHandlersMap);

	// Build Options
	this->BuildOptions_Allow.Read(exINI, pSection, "BuildOptions.Allow");
	this->BuildOptions_Disallow.Read(exINI, pSection, "BuildOptions.Disallow");
}

template <typename T>
void PlayerEmblemTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->BuildOptions_Allow)
		.Process(this->BuildOptions_Disallow)
		.Process(this->EventHandlersMap)
		.Success();
}

void PlayerEmblemTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void PlayerEmblemTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

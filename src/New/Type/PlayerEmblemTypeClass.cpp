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

bool PlayerEmblemTypeClass::AutoCreateAttachEffects() const
{
	return !this->AttachEffect_AttachTypes.empty()
		&& !this->AttachEffect_TechnoTypes.empty();
}

void PlayerEmblemTypeClass::InvokeEventHandlers(EventTypeClass* pEventType, HouseClass* pHouse) const
{
	static std::map<EventActorType, AbstractClass*> participants;
	participants[EventActorType::Me] = pHouse;
	EventHandlerTypeClass::InvokeEventStatic(pEventType, &participants, &this->EventHandlersMap);
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

	// Attach Effect
	this->AttachEffect_AttachTypes.Read(exINI, pSection, "AttachEffect.AttachTypes");
	this->AttachEffect_TechnoTypes.Read(exINI, pSection, "AttachEffect.TechnoTypes");
}

template <typename T>
void PlayerEmblemTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->BuildOptions_Allow)
		.Process(this->BuildOptions_Disallow)
		.Process(this->EventHandlersMap)
		.Process(this->AttachEffect_AttachTypes)
		.Process(this->AttachEffect_TechnoTypes)
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

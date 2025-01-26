#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerCompClass.h"

class HandlerCompClass;

class EventHandlerTypeClass final : public Enumerable<EventHandlerTypeClass>
{
public:
	Valueable<bool> loaded;
	std::vector<std::unique_ptr<HandlerCompClass>> HandlerComps;
	Nullable<EventHandlerTypeClass*> Next;

	EventHandlerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventHandlerTypeClass>(pTitle)
		, loaded { false }
		, HandlerComps {}
		, Next {}
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromINI(INI_EX& exINI);
	void LoadForActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const char* actorName);
	void LoadForExtendedActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const EventExtendedActorType extendedActorType, const char* actorName, const char* extendedActorName);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventHandlerTypeClass*>* vec);
	static void LoadTypeMapFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, PhobosMap<EventTypeClass*, std::vector<EventHandlerTypeClass*>>* map);

	void HandleEvent(std::map<EventActorType, AbstractClass*>* pParticipants);

	static void InvokeEventStatic(EventTypeClass* pEventTypeClass,
		std::map<EventActorType, AbstractClass*>* pParticipants,
		const PhobosMap<EventTypeClass*, std::vector<EventHandlerTypeClass*>>* map);
private:
	template <typename T>
	void Serialize(T& Stm);
	void LoadFromINIPrivate(INI_EX& exINI, const char* pSection);

	bool CheckFilters(std::map<EventActorType, AbstractClass*>* pParticipants, EventActorType actorType) const;
	void ExecuteEffects(std::map<EventActorType, AbstractClass*>* pParticipants, EventActorType actorType) const;
};

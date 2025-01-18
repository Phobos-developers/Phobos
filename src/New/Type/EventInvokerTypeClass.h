#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerCompClass.h"

class EventHandlerTypeClass;

class HandlerCompClass;

class EventInvokerTypeClass final : public Enumerable<EventInvokerTypeClass>
{
public:
	Valueable<bool> loaded;
	std::vector<std::unique_ptr<HandlerCompClass>> HandlerComps;
	ValueableVector<EventTypeClass*> EventTypes;
	ValueableVector<EventHandlerTypeClass*> ExtraEventHandlers;
	Valueable<bool> PassDown_Passengers;
	Valueable<bool> PassDown_MindControlled;

	EventInvokerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventInvokerTypeClass>(pTitle)
		, loaded { false }
		, HandlerComps {}
		, EventTypes {}
		, ExtraEventHandlers {}
		, PassDown_Passengers { false }
		, PassDown_MindControlled { false }
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromINI(INI_EX& exINI);
	void LoadForActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const char* actorName);
	void LoadForExtendedActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const EventExtendedActorType extendedActorType, const char* actorName, const char* extendedActorName);
	void TryExecute(HouseClass* pHouse, std::map<EventActorType, AbstractClass*>* pParticipants);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventInvokerTypeClass*>* vec);
private:
	template <typename T>
	void Serialize(T& Stm);
	void LoadFromINIPrivate(INI_EX& exINI, const char* pSection);
	bool CheckInvokerFilters(HouseClass* pHouse, AbstractClass* pInvoker) const;
	bool CheckTargetFilters(HouseClass* pHouse, AbstractClass* pTarget) const;
	void TryExecuteOnTarget(HouseClass* pHouse, std::map<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pTarget);
	void TryPassDown(HouseClass* pHouse, std::map<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pRoot);
};

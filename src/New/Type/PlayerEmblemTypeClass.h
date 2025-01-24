#pragma once
#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include "EventTypeClass.h"
#include "EventHandlerTypeClass.h"

class PlayerEmblemTypeClass final : public Enumerable<PlayerEmblemTypeClass>
{
public:
	PhobosMap<EventTypeClass*, std::vector<EventHandlerTypeClass*>> EventHandlersMap;

	ValueableVector<TechnoTypeClass*> BuildOptions_Allow;
	ValueableVector<TechnoTypeClass*> BuildOptions_Disallow;

	PlayerEmblemTypeClass(const char* pTitle = NONE_STR) : Enumerable<PlayerEmblemTypeClass>(pTitle)
		, EventHandlersMap {}
		, BuildOptions_Allow {}
		, BuildOptions_Disallow {}
	{}

	bool AlterBuildOptions() const;
	void InvokeEventHandlers(EventTypeClass* pEventType, HouseClass* pHouse) const;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

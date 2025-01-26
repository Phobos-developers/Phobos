#pragma once
#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class EventTypeClass final : public Enumerable<EventTypeClass>
{
public:

	EventTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventTypeClass>(pTitle)
	{ }

	static void AddDefaults();

#pragma region TechnoEvents
	static EventTypeClass* WhenCreated;
	static EventTypeClass* WhenCaptured;
	static EventTypeClass* WhenPromoted;
	static EventTypeClass* WhenDemoted;
	static EventTypeClass* WhenProduce;
	static EventTypeClass* WhenProduced;
	static EventTypeClass* WhenGrind;
	static EventTypeClass* WhenGrinded;
	static EventTypeClass* WhenKill;
	static EventTypeClass* WhenKilled;
	static EventTypeClass* WhenCrush;
	static EventTypeClass* WhenCrushed;
	static EventTypeClass* WhenInfiltrate;
	static EventTypeClass* WhenInfiltrated;
	static EventTypeClass* WhenLoad;
	static EventTypeClass* WhenUnload;
	static EventTypeClass* WhenBoard;
	static EventTypeClass* WhenUnboard;
	static EventTypeClass* WhenUpgraded;
#pragma endregion

#pragma region AttachedEffectEvents
	static EventTypeClass* WhenAttach;
	static EventTypeClass* WhenDetach;
	static EventTypeClass* WhenExpired;
	static EventTypeClass* WhenRemoved;
	static EventTypeClass* WhenObjectDied;
	static EventTypeClass* WhenDiscarded;
#pragma endregion

#pragma region SuperWeaponEvents
	static EventTypeClass* WhenLaunch;
	static EventTypeClass* WhenImpact;
#pragma endregion

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventTypeClass*>* vec);

private:
	template <typename T>
	void Serialize(T& Stm);
};

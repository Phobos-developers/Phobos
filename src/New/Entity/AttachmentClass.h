#pragma once

#include <algorithm>

#include <GeneralStructures.h>

#include <New/Type/AttachmentTypeClass.h>
#include <Ext/TechnoType/Body.h>

class TechnoClass;
class HouseClass;

class AttachmentClass
{
public:
	static std::vector<AttachmentClass*> Array;

	TechnoTypeExt::ExtData::AttachmentDataEntry* Data;
	TechnoClass* Parent;
	TechnoClass* Child;
	HouseClass* ChildOriginalOwner;
	CDTimerClass RespawnTimer;
	AbstractClass* LastValidParentTarget;
	AbstractClass* LastValidParentDestination;
	Mission LastValidParentMission;

	AttachmentClass(TechnoTypeExt::ExtData::AttachmentDataEntry* data,
		TechnoClass* pParent, TechnoClass* pChild = nullptr) :
		Data { data },
		Parent { pParent },
		Child { pChild },
		ChildOriginalOwner { pChild ? pChild->Owner : nullptr },
		RespawnTimer { },
		LastValidParentTarget { nullptr },
		LastValidParentDestination { nullptr },
		LastValidParentMission { Mission::None }
	{
		Array.push_back(this);
	}

	AttachmentClass() :
		Data { },
		Parent { },
		Child { },
		ChildOriginalOwner { nullptr },
		RespawnTimer { },
		LastValidParentTarget { nullptr },
		LastValidParentDestination { nullptr },
		LastValidParentMission { Mission::None }
	{
		Array.push_back(this);
	}

	~AttachmentClass();

	AttachmentTypeClass* GetType();
	TechnoTypeClass* GetChildType();
	CoordStruct GetChildLocation();

	void OnCreated();
	void CreateChild();
	void AI();
	void Destroy(TechnoClass* pSource);
	void ChildDestroyed();

	void Unlimbo();
	void Limbo();

	bool AttachChild(TechnoClass* pChild);
	bool DetachChild();

	// Core link/unlink without side effects (locomotor changes, missions, owner resets)
	void AttachChildCore(TechnoClass* pChild);
	void DetachChildCore();

	void InvalidatePointer(void* ptr);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

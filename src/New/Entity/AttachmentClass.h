#pragma once

#include <algorithm>

#include <GeneralStructures.h>

#include <New/Type/AttachmentTypeClass.h>
#include <Ext/TechnoType/Body.h>

class TechnoClass;

class AttachmentClass
{
public:
	static std::vector<AttachmentClass*> Array;

	TechnoTypeExt::ExtData::AttachmentDataEntry* Data;
	TechnoClass* Parent;
	TechnoClass* Child;

	// volatile, don't serialize
	// if you ever change the tree structure, you need to call CacheTreeData()
	struct Cache {
		TechnoClass* TopLevelParent;

		int LastUpdateFrame;
		Matrix3D ChildTransform;
	} Cache;

	AttachmentClass(TechnoTypeExt::ExtData::AttachmentDataEntry* data,
		TechnoClass* pParent, TechnoClass* pChild = nullptr) :
		Data(data),
		Parent(pParent),
		Child(pChild)
	{
		this->InitCacheData();
		Array.push_back(this);
	}

	AttachmentClass() :
		Data(),
		Parent(),
		Child()
	{
		Array.push_back(this);
	}

	~AttachmentClass();

	void InitCacheData();
	Matrix3D GetUpdatedTransform(int* pKey = nullptr);

	AttachmentTypeClass* GetType();
	TechnoTypeClass* GetChildType();
	CoordStruct GetChildLocation();

	void Initialize();
	void CreateChild();
	void AI();
	void Destroy(TechnoClass* pSource);
	void ChildDestroyed();

	void Unlimbo();
	void Limbo();

	bool AttachChild(TechnoClass* pChild);
	bool DetachChild(bool force = false);

	void InvalidatePointer(void* ptr);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

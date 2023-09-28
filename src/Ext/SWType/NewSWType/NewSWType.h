#pragma once
#include <Ext/SWType/Body.h>
#include <SuperClass.h>
#include <HouseClass.h>

class NewSWType
{
public:

	static void Init();
	static void Clear();
	static int GetNewSWTypeIdx(const char* TypeID);
	static NewSWType* GetNthItem(int idx);

	virtual ~NewSWType() = default;

	virtual int GetTypeIndex() final;

	// selectable override

	virtual void Initialize(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW) { }
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) { }

	// must be override

	virtual const char* GetTypeID() = 0;
	virtual bool Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer) = 0;

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

protected:
	virtual void SetTypeIndex(int idx) final;

private:
	static std::vector<std::unique_ptr<NewSWType>> Array;
	static void Register(std::unique_ptr<NewSWType> pType);

	int TypeIndex = -1;
};

#pragma once
#include "NewSWType.h"

#include <vector>

class EMPulseExtra : public NewSWType
{
public:
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) override;
	virtual const char* GetTypeID() override;
	virtual bool Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer) override;

	static void FireEMPulse(TechnoClass* pFirer, SuperClass* pSW, const CellStruct& cell);

	static void ProcessEMPulseCannon(const std::vector<TechnoClass*>& technos, SuperClass* pSW, const CellStruct& cell);
};

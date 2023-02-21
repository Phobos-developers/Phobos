#pragma once

#include <Utilities/GeneralUtils.h>

#include <TechnoClass.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Utilities/Enum.h>

struct TransferUnit
{
	TransferUnit(TechnoClass* pTechno, TransferResource attr);
	TransferUnit(HouseClass* pHouse);
	TransferUnit(): TransferUnit(nullptr, TransferResource::Health) { }

	TechnoClass* Techno;
	HouseClass* House;

	double Value;
	double Modifier;

	double Current;
	double Total;
	TransferResource Resource;
};

class TransferClass
{
public:
	TransferClass() = delete;
	TransferClass(TechnoClass* pSTechno, HouseClass* pHouse, WarheadTypeClass* pWarhead, TechnoClass* pTTechno, TransferTypeClass* pTType, CoordStruct coords);

	static int ChangeHealth(TechnoClass* pTechno, int value, TechnoClass* pSource, HouseClass* pHouse, WarheadTypeClass* pWarhead, bool killable);
	static int ChangeExperience(TechnoClass* pTechno, int value, bool demotable);
	static int ChangeMoney(HouseClass* pHouse, int value);
	static int ChangeAmmo(TechnoClass* pTechno, int value);
	static int ChangeGatlingRate(TechnoClass* pTechno, int value, int changeLimit, bool canCycle, bool change = true);

	int AlterResource(TransferUnit* pValues);

	bool PerformTransfer();
private:
	TechnoClass* SourceTechno;
	HouseClass* SourceHouse;
	WarheadTypeClass* SourceWarhead;
	TechnoClass* BulletTargetTechno;
	CoordStruct DetonationCoords;
	TransferTypeClass* Type;

	std::vector<TransferUnit> Senders;
	std::vector<TransferUnit> Receivers;

	std::string FailureMessage;

	bool IsCellSpread = false;
	bool IsSenderTarget = false;
	bool IsReceiverTarget = false;
	bool IsSenderExtra = false;
	bool IsReceiverExtra = false;

	bool DetermineSides();
	bool ApplyModifiers();
	bool RegisterValues();
	bool ValidateLimits();
	bool EnforceChanges();
};

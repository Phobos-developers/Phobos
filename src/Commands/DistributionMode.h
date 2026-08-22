#pragma once

#include "Commands.h"
#include "AdvancedCommandBarButtons.h"

class ObjectClass;
class TechnoClass;
class TechnoTypeClass;

struct DistributionTargetInfo
{
	CoordStruct Center;
	bool TargetIsNeutral;
	TechnoTypeClass* pType;
	AbstractType WhatAmI;
	Action Action;
};

class SwitchNoMoveCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeSpreadCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeFilterCommandClass : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

class DistributionModeHoldDownCommandClass : public CommandClass
{
public:
	static bool Enabled;
	static bool OnMessageShowed;
	static bool OffMessageShowed;
	static int ShowTime;

	static bool IsDragDistributing;
	static CoordStruct DragStartCenter;
	static DistributionTargetInfo DragInfo;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual bool PreventCombinationOverride(WWKey eInput) const override;
	virtual bool ExtraTriggerCondition(WWKey eInput) const override;
	virtual void Execute(WWKey eInput) const override;

	static void DistributionModeOn();
	static void DistributionModeOff();
	static void DistributionSpreadModeExpand();
	static void DistributionSpreadModeReduce();

	static void __fastcall ClickedWaypoint(ObjectClass* pSelect, int idxPath, signed char idxWP);
	static void __fastcall ClickedTargetAction(ObjectClass* pSelect, Action action, ObjectClass* pTarget);
	static void __fastcall ClickedCellAction(ObjectClass* pSelect, Action action, CellStruct* pCell, CellStruct* pSecondCell);
	static void __fastcall AreaGuardAction(TechnoClass* pTechno);

	static void ProcessWaypointCommand(int idxPath, unsigned char idxWP);
	static bool IsDistributionModeEligible(unsigned int range, int count, Action action, TechnoClass* pTechno);
	static DistributionTargetInfo CollectTargetInfo(TechnoClass* pTechno, Action action);
	static std::vector<std::pair<TechnoClass*, int>> CollectAndSortTargets(CoordStruct center, double range);
	static void ProcessDistributionMode(const DistributionTargetInfo& info, ObjectClass* pTarget, int filterMode, bool noMove);
	static void ProcessNormalTargetClick(ObjectClass* pTarget, Action action, bool noMove);
	static void ProcessCellClick(CellStruct* pCell, Action action);
};

class DistributionModeHoldDownButtonClass : public AdvancedCommandBarButton
{
	virtual const char* GetName() const override;
	virtual const char* GetTipName() const override;
	virtual bool CanHoldDown() const override;
	virtual void Execute(bool isOn) const override;
};

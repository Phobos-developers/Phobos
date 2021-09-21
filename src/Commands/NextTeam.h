#pragma once
#include <TeamClass.h>
#include <HouseClass.h>

#include "Commands.h"
#include <Utilities/GeneralUtils.h>

class NextTeamCommandClass : public PhobosCommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override
	{
		return "Next Team";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_TEAM", L"Next Team");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return StringTable::LoadString("TXT_SELECTION");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_TEAM_DESC", L"Select the next team belonging to player.");
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		MapClass::Instance->SetTogglePowerMode(0);
		MapClass::Instance->SetWaypointMode(0, false);
		MapClass::Instance->SetRepairMode(0);
		MapClass::Instance->SetSellMode(0);

		if (!TeamClass::Array->Count)
			return;

		TeamClass* pTeam = nullptr;
		for (auto& pObject : ObjectClass::CurrentObjects())
		{
			if (auto const pFoot = abstract_cast<FootClass*>(pObject))
			{
				if (pFoot->Team && pFoot->Team->Owner->IsPlayerControl())
				{
					pTeam = pFoot->Team;
					break;
				}
			}
		}

		int nIndex = TeamClass::Array->FindItemIndex(pTeam);
		if (++nIndex == TeamClass::Array->Count)
			nIndex = 0;

		MapClass::UnselectAll();
		pTeam = TeamClass::Array->GetItem(nIndex);
		for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
		{
			if (!pFoot->IsSelected && pFoot->IsSelectable())
			{
				pFoot->Select();
				Unsorted::MoveFeedback = false;
			}
		}
		Unsorted::MoveFeedback = true;
	}
};

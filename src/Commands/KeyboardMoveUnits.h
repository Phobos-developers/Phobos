#pragma once

#include "Commands.h"

#include <FootClass.h>
#include <Unsorted.h>

#include <Utilities/GeneralUtils.h>


template<size_t Dir>
class KeyboardMoveUnits : public CommandClass
{
public:
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;

	virtual bool ExtraTriggerCondition(WWKey) const override;
	static CellStruct CalcDirection();

	static inline bool KeyDown = false;
};

namespace MoveUnits
{
	inline void MoveSelectedUnits(const CellStruct& direction)
	{
		for (const auto pObject : ObjectClass::CurrentObjects)
		{
			const auto pFoot = abstract_cast<FootClass*, true>(pObject);

			if (!pFoot || !pFoot->IsAlive || pFoot->InLimbo)
				continue;

			const auto pOwner = pFoot->Owner;

			if (!pOwner || !pOwner->IsControlledByCurrentPlayer())
				continue;

			const auto pTargetCell = MapClass::Instance.TryGetCellAt(pFoot->GetMapCoords() + direction);

			if (pTargetCell && pFoot->Destination != pTargetCell)
			{
				if (direction != CellStruct::Empty)
				{
					pFoot->SetDestination(pTargetCell, true);
					pFoot->QueueMission(Mission::Move, false);
				}
				else
				{
					if(pFoot->GetCurrentMission() == Mission::Move)
						pFoot->SetDestination(nullptr, true);
				}
			}
		}
	}
	
	inline void KeyboardMoveUpdate()
	{
		CellStruct finalDirection = KeyboardMoveUnits<0>::CalcDirection() + KeyboardMoveUnits<1>::CalcDirection() + KeyboardMoveUnits<2>::CalcDirection() + KeyboardMoveUnits<3>::CalcDirection();
		const bool keyDown = KeyboardMoveUnits<0>::KeyDown || KeyboardMoveUnits<1>::KeyDown || KeyboardMoveUnits<2>::KeyDown || KeyboardMoveUnits<3>::KeyDown;
		if(Phobos::Config::ToggleKeyboardMove)
		{
			if(keyDown)
				MoveUnits::MoveSelectedUnits(finalDirection);
			else
				MoveUnits::MoveSelectedUnits(CellStruct::Empty);
		}
	}
}

template<size_t Dir>
bool KeyboardMoveUnits<Dir>::ExtraTriggerCondition(WWKey) const
{
	return true;
}

template<size_t Dir>
const char* KeyboardMoveUnits<Dir>::GetName() const
{
	_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Keyboard Move Units %d", Dir);
	return Phobos::readBuffer;
}

template<size_t Dir>
const wchar_t* KeyboardMoveUnits<Dir>::GetUIName() const
{
	const wchar_t* csfString = StringTable::TryFetchString("TXT_KEYBOARD_MOVE_UNITS_XX", L"Keyboard Move Units %d");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Dir);
	return Phobos::wideBuffer;
}

template<size_t Dir>
const wchar_t* KeyboardMoveUnits<Dir>::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

template<size_t Dir>
const wchar_t* KeyboardMoveUnits<Dir>::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_KEYBOARD_MOVE_UNITS_DESC", L"Allows moving units with keyboard keys.");
}

template<size_t Dir>
void KeyboardMoveUnits<Dir>::Execute(WWKey eInput) const
{
	KeyDown = !(eInput & WWKey::Release);
}

template<size_t Dir>
CellStruct KeyboardMoveUnits<Dir>::CalcDirection()
{
	CellStruct direction = {0, 0};

	if (!Game::IsFocused || !ScenarioClass::Instance || ScenarioClass::Instance->UserInputLocked)
	{
		KeyDown = false;
		return direction;
	}

	if(KeyDown)
	{
		switch (Dir)
		{
		case 0:
			direction = {-2, -2}; // up
			break;
		case 1:
			direction = {2, 2}; // down
			break;
		case 2:
			direction = {-2, 2}; // left
			break;
		case 3:
			direction = {2, -2}; // right
			break;
		default:
			break;
		}
	}

	return direction;
}
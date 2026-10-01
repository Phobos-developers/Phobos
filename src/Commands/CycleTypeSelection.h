#pragma once

#include "Commands.h"

// Cycles through the object types present in the current selection, selecting every object
// of one type at a time and wrapping around at the end of the type list.
// "Same type" is decided by the game's own type selection definition - vanilla's Type ID
// comparison, extended by Ares' GroupAs / Phobos' selection group IDs - mirrored by this
// command's own predicate rather than delegated, because the cycle must stay confined to
// the objects the cycle was started with.
// The order the types are cycled to is customizable through TypeCyclePriority.
class CycleTypeSelectionCommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

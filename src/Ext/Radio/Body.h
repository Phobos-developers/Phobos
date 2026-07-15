#pragma once

#include <Ext/Mission/Body.h>
#include <RadioClass.h>

// Empty intermediate base mirroring RadioClass in the extension hierarchy.
class RadioClassExtension : public MissionClassExtension
{
public:
	explicit RadioClassExtension(RadioClass* const OwnerObject) : MissionClassExtension(OwnerObject)
	{ }

	RadioClass* OwnerObject() const
	{
		return static_cast<RadioClass*>(this->GetAttachedObject());
	}
};

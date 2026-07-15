#pragma once

#include <Ext/Object/Body.h>
#include <MissionClass.h>

// Empty intermediate base mirroring MissionClass in the extension hierarchy.
class MissionClassExtension : public ObjectClassExtension
{
public:
	explicit MissionClassExtension(MissionClass* const OwnerObject) : ObjectClassExtension(OwnerObject)
	{ }

	MissionClass* OwnerObject() const
	{
		return static_cast<MissionClass*>(this->GetAttachedObject());
	}
};

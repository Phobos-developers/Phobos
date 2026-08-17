#pragma once

#include <Ext/Object/Body.h>
#include <MissionClass.h>

// Empty intermediate base mirroring MissionClass in the extension hierarchy.
class MissionExt : public ObjectExt
{
public:
	explicit MissionExt(MissionClass* const OwnerObject) : ObjectExt(OwnerObject)
	{ }

	MissionClass* OwnerObject() const
	{
		return static_cast<MissionClass*>(this->GetAttachedObject());
	}

	static MissionExt* Fetch(const MissionClass* pThis)
	{
		return AbstractExt::Fetch<MissionExt>(pThis);
	}

	static MissionExt* TryFetch(const MissionClass* pThis)
	{
		return AbstractExt::TryFetch<MissionExt>(pThis);
	}
};

#pragma once

#include <Ext/Mission/Body.h>
#include <RadioClass.h>

// Empty intermediate base mirroring RadioClass in the extension hierarchy.
class RadioExt : public MissionExt
{
public:
	explicit RadioExt(RadioClass* const OwnerObject) : MissionExt(OwnerObject)
	{ }

	RadioClass* OwnerObject() const
	{
		return static_cast<RadioClass*>(this->GetAttachedObject());
	}

	static RadioExt* Fetch(const RadioClass* pThis)
	{
		return static_cast<RadioExt*>(AbstractExt::Fetch(pThis));
	}

	static RadioExt* TryFetch(const RadioClass* pThis)
	{
		return pThis ? Fetch(pThis) : nullptr;
	}
};

#pragma once

#include <ObjectClass.h>
#include <Utilities/Container.h>

// Empty intermediate base mirroring ObjectClass in the extension hierarchy.
// It carries no data of its own; it only exists so the chain of extensions
// matches the game's class hierarchy (AbstractClass -> ObjectClass -> ...).
class ObjectExt : public AbstractExt
{
public:
	explicit ObjectExt(ObjectClass* const OwnerObject) : AbstractExt(OwnerObject)
	{ }

	ObjectClass* OwnerObject() const
	{
		return static_cast<ObjectClass*>(this->GetAttachedObject());
	}

	static ObjectExt* Fetch(const ObjectClass* pThis)
	{
		return AbstractExt::Fetch<ObjectExt>(pThis);
	}

	static ObjectExt* TryFetch(const ObjectClass* pThis)
	{
		return AbstractExt::TryFetch<ObjectExt>(pThis);
	}
};

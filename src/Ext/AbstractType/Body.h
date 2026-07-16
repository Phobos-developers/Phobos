#pragma once

#include <AbstractTypeClass.h>
#include <Utilities/Container.h>

// Empty intermediate base mirroring AbstractTypeClass in the extension hierarchy.
class AbstractTypeExt : public AbstractExt
{
public:
	explicit AbstractTypeExt(AbstractTypeClass* const OwnerObject) : AbstractExt(OwnerObject)
	{ }

	AbstractTypeClass* OwnerObject() const
	{
		return static_cast<AbstractTypeClass*>(this->GetAttachedObject());
	}

	static AbstractTypeExt* Fetch(const AbstractTypeClass* pThis)
	{
		return static_cast<AbstractTypeExt*>(AbstractExt::Fetch(pThis));
	}

	static AbstractTypeExt* TryFetch(const AbstractTypeClass* pThis)
	{
		return pThis ? Fetch(pThis) : nullptr;
	}
};

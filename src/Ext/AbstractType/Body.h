#pragma once

#include <AbstractTypeClass.h>
#include <Utilities/Container.h>

// Empty intermediate base mirroring AbstractTypeClass in the extension hierarchy.
class AbstractTypeClassExtension : public AbstractExt
{
public:
	explicit AbstractTypeClassExtension(AbstractTypeClass* const OwnerObject) : AbstractExt(OwnerObject)
	{ }

	AbstractTypeClass* OwnerObject() const
	{
		return static_cast<AbstractTypeClass*>(this->GetAttachedObject());
	}
};

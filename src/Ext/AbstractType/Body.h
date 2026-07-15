#pragma once

#include <AbstractTypeClass.h>
#include <Utilities/Container.h>

// Empty intermediate base mirroring AbstractTypeClass in the extension hierarchy.
class AbstractTypeClassExtension : public AbstractClassExtension
{
public:
	explicit AbstractTypeClassExtension(AbstractTypeClass* const OwnerObject) : AbstractClassExtension(OwnerObject)
	{ }

	AbstractTypeClass* OwnerObject() const
	{
		return static_cast<AbstractTypeClass*>(this->GetAttachedObject());
	}
};

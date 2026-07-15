#pragma once

#include <Ext/AbstractType/Body.h>
#include <ObjectTypeClass.h>

// Empty intermediate base mirroring ObjectTypeClass in the extension hierarchy.
class ObjectTypeClassExtension : public AbstractTypeClassExtension
{
public:
	explicit ObjectTypeClassExtension(ObjectTypeClass* const OwnerObject) : AbstractTypeClassExtension(OwnerObject)
	{ }

	ObjectTypeClass* OwnerObject() const
	{
		return static_cast<ObjectTypeClass*>(this->GetAttachedObject());
	}
};

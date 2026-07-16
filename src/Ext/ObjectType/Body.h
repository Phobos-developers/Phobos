#pragma once

#include <Ext/AbstractType/Body.h>
#include <ObjectTypeClass.h>

// Empty intermediate base mirroring ObjectTypeClass in the extension hierarchy.
class ObjectTypeExt : public AbstractTypeExt
{
public:
	explicit ObjectTypeExt(ObjectTypeClass* const OwnerObject) : AbstractTypeExt(OwnerObject)
	{ }

	ObjectTypeClass* OwnerObject() const
	{
		return static_cast<ObjectTypeClass*>(this->GetAttachedObject());
	}

	static ObjectTypeExt* Fetch(const ObjectTypeClass* pThis)
	{
		return static_cast<ObjectTypeExt*>(AbstractExt::Fetch(pThis));
	}

	static ObjectTypeExt* TryFetch(const ObjectTypeClass* pThis)
	{
		return pThis ? Fetch(pThis) : nullptr;
	}
};

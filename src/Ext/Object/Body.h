#pragma once

#include <ObjectClass.h>
#include <Utilities/Container.h>

// Empty intermediate base mirroring ObjectClass in the extension hierarchy.
// It carries no data of its own; it only exists so the chain of extensions
// matches the game's class hierarchy (AbstractClass -> ObjectClass -> ...).
class ObjectClassExtension : public AbstractClassExtension
{
public:
	explicit ObjectClassExtension(ObjectClass* const OwnerObject) : AbstractClassExtension(OwnerObject)
	{ }

	ObjectClass* OwnerObject() const
	{
		return static_cast<ObjectClass*>(this->GetAttachedObject());
	}
};

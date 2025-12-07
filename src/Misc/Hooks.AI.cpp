#include <New/Entity/AttachmentClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x55B6B3, LogicClass_AI_After, 0x5)
{
	for (auto const& attachment : AttachmentClass::Array)
	{
		auto pType = attachment->GetType();
		if (pType && pType->InheritStateEffects && !pType->PassSelection && attachment->Child)
		{
			int childFlashRemaining = attachment->Child->Flashing.DurationRemaining;

			attachment->AI();

			if (childFlashRemaining > 0)
			{
				attachment->Child->Flash(childFlashRemaining);
			}
		}
		else
		{
			attachment->AI();
		}
	}

	return 0;
}

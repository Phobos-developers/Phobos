#include <New/Entity/AttachmentClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x55B6B3, LogicClass_AI_After, 0x5)
{
	for (auto const& attachment : AttachmentClass::Array)
		attachment->AI();

	return 0;
}
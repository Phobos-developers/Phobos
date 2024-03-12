#include <Phobos.h>
#include <Utilities/Macro.h>
#include <ColorScheme.h>

DEFINE_HOOK(0x69A317, GetLinkedColor_SkirmishUnlimitedColors, 0x0)
{
	GET_STACK(int, index, 0x4);

	if (index == -2)
		index = ColorScheme::FindIndex("LightGrey", 53);
	else
		index = index * 2 + 1;

	R->EAX(index);

	return 0x69A325;
}

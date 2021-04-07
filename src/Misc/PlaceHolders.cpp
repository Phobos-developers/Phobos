#include <Phobos.h>

DEFINE_HOOK(70ABDD, TechnoClass_Draw_Stuff_When_Selected_PowerDisplay, 8)
{
	LEA_STACK(wchar_t*, pString, STACK_OFFS(0x15C, 0x100));
	Debug::Log(__FUNCTION__ " string = %ls\n", pString);
	R->EDX(pString);
	R->ECX(R->lea_Stack<int>(STACK_OFFS(0x15C, 0x120)));

	return 0x70ABE5;
}
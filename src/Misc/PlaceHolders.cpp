#include <Phobos.h>

// #issue 149 : replace power used by power remain
// Failed cause Ares overtaken 70AA60 = TechnoClass_DrawExtraInfo, 6

//DEFINE_HOOK(0x70ABDD, TechnoClass_Draw_Stuff_When_Selected_PowerDisplay, 0x8)
//{
//	LEA_STACK(wchar_t*, pString, STACK_OFFSET(0x15C, -0x100));
//	Debug::Log(__FUNCTION__ " string = %ls\n", pString);
//	R->EDX(pString);
//	R->ECX(R->lea_Stack<int>(STACK_OFFSET(0x15C, -0x120)));
//
//	return 0x70ABE5;
//}

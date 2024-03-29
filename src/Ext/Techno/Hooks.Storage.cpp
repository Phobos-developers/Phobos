#include "Body.h"
#include "New/Entity/ExtendedStorageClass.h"

DEFINE_HOOK(0x6C9600, StorageClass_Get_Total_Value, 0x5)
{
	enum { Exit = 0x6C964E };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	int value = (*pStorageClass)->Get_Total_Value();
	R->EAX(value);

	return Exit;
}

DEFINE_HOOK(0x6C9653, StorageClass_Get_Total_Amount, 0x5)
{
	enum { Exit = 0x6C967B };

	GET(ExtendedStorageClass**, pStorageClass, ECX);

	float value = (*pStorageClass)->Get_Total_Amount();
	__asm fld value

	return Exit;
}

DEFINE_HOOK(0x6C9680, StorageClass_Get_Amount, 0x5)
{
	enum { Exit = 0x6C9687 };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET_STACK(int, tiberium, 4);

	float value = (*pStorageClass)->Get_Amount(tiberium);
	__asm fld value

	return Exit;
}

DEFINE_HOOK(0x6C9694, StorageClass_Increase_Amount, 0x5)
{
	enum { Exit = 0x6C969E };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET(int, tiberium, EAX);
	GET_STACK(float, amount, 4);

	float value = (*pStorageClass)->Increase_Amount(amount, tiberium);
	__asm fld value

	return Exit;
}

DEFINE_HOOK(0x6C96B4, StorageClass_Decrease_Amount, 0x5)
{
	enum { Exit = 0x6C96DC };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET(int, tiberium, EDX);
	GET_STACK(float, amount, 4);

	float value = (*pStorageClass)->Decrease_Amount(amount, tiberium);
	__asm fld value

	return Exit;
}

DEFINE_HOOK(0x6C96E3, StorageClass_Operator_Plus, 0x5)
{
	enum { Exit = 0x6C973A };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET_STACK(ExtendedStorageClass**, that, STACK_OFFSET(0x10, 0x8));
	GET_STACK(ExtendedStorageClass**, retstr, STACK_OFFSET(0x10, 0x4));

	auto newStorageClass = **pStorageClass + **that;
	R->EAX(newStorageClass);
	*retstr = newStorageClass;

	return Exit;
}

DEFINE_HOOK(0x6C9745, StorageClass_Operator_PlusEquals, 0x5)
{
	enum { Exit = 0x6C9775 };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET(ExtendedStorageClass**, that, EDX);
	GET_STACK(ExtendedStorageClass**, retstr, STACK_OFFSET(0x4, 0x4));

	**pStorageClass += **that;
	R->EAX(*pStorageClass);
	*retstr = *pStorageClass;

	return Exit;
}

DEFINE_HOOK(0x6C9783, StorageClass_Operator_Minus, 0x5)
{
	enum { Exit = 0x6C97D8 };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET_STACK(ExtendedStorageClass**, that, STACK_OFFSET(0x10, 0x8));
	GET_STACK(ExtendedStorageClass**, retstr, STACK_OFFSET(0x10, 0x4));

	auto newStorageClass = **pStorageClass - **that;
	R->EAX(newStorageClass);
	*retstr = newStorageClass;

	return Exit;
}

DEFINE_HOOK(0x6C97E5, StorageClass_Operator_MinusEquals, 0x5)
{
	enum { Exit = 0x6C9815 };

	GET(ExtendedStorageClass**, pStorageClass, ECX);
	GET(ExtendedStorageClass**, that, EDX);
	GET_STACK(ExtendedStorageClass**, retstr, STACK_OFFSET(0x4, 0x4));

	**pStorageClass -= **that;
	R->EAX(*pStorageClass);
	*retstr = *pStorageClass;

	return Exit;
}

DEFINE_HOOK(0x6C9820, StorageClass_First_Used_Slot, 0x5)
{
	enum { Exit = 0x6C983D };

	GET(ExtendedStorageClass**, pStorageClass, ECX);

	int result = (*pStorageClass)->First_Used_Slot();
	R->EAX(result);

	return Exit;
}

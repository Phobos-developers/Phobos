#include "Body.h"
#include "New/Entity/ExtendedStorageClass.h"

namespace StorageClass_Wrappers
{
	int __fastcall Get_Total_Value(ExtendedStorageClass** pStorageClass)
	{
		return (*pStorageClass)->Get_Total_Value();
	}

	double __fastcall Get_Total_Amount(ExtendedStorageClass** pStorageClass)
	{
		return (*pStorageClass)->Get_Total_Amount();
	}

	double __fastcall Get_Amount(ExtendedStorageClass** pStorageClass, void*, int index)
	{
		return (*pStorageClass)->Get_Amount(index);
	}

	float __fastcall Increase_Amount(ExtendedStorageClass** pStorageClass, void*, float amount, int index)
	{
		return (*pStorageClass)->Increase_Amount(amount, index);
	}

	float __fastcall Decrease_Amount(ExtendedStorageClass** pStorageClass, void*, float amount, int index)
	{
		return (*pStorageClass)->Decrease_Amount(amount, index);
	}

	ExtendedStorageClass** __fastcall Operator_Plus(ExtendedStorageClass** pStorageClass, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** that)
	{
		**retstr = **pStorageClass + **that;
		return retstr;
	}

	ExtendedStorageClass** __fastcall Operator_PlusEquals(ExtendedStorageClass** pStorageClass, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** that)
	{
		**pStorageClass += **that;
		*retstr = *pStorageClass;
		return pStorageClass;
	}

	ExtendedStorageClass** __fastcall Operator_Minus(ExtendedStorageClass** pStorageClass, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** that)
	{
		**retstr = **pStorageClass - **that;
		return retstr;
	}

	ExtendedStorageClass** __fastcall Operator_MinusEquals(ExtendedStorageClass** pStorageClass, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** that)
	{
		**pStorageClass -= **that;
		*retstr = *pStorageClass;
		return pStorageClass;
	}

	int __fastcall First_Used_Slot(ExtendedStorageClass** pStorageClass)
	{
		return (*pStorageClass)->First_Used_Slot();
	}
}

DEFINE_JUMP(LJMP, 0x6C9600, GET_OFFSET(StorageClass_Wrappers::Get_Total_Value))
DEFINE_JUMP(LJMP, 0x6C9650, GET_OFFSET(StorageClass_Wrappers::Get_Total_Amount))
DEFINE_JUMP(LJMP, 0x6C9680, GET_OFFSET(StorageClass_Wrappers::Get_Amount))
DEFINE_JUMP(LJMP, 0x6C9690, GET_OFFSET(StorageClass_Wrappers::Increase_Amount))
DEFINE_JUMP(LJMP, 0x6C96B0, GET_OFFSET(StorageClass_Wrappers::Decrease_Amount))
DEFINE_JUMP(LJMP, 0x6C96E0, GET_OFFSET(StorageClass_Wrappers::Operator_Plus))
DEFINE_JUMP(LJMP, 0x6C9740, GET_OFFSET(StorageClass_Wrappers::Operator_PlusEquals))
DEFINE_JUMP(LJMP, 0x6C9780, GET_OFFSET(StorageClass_Wrappers::Operator_Minus))
DEFINE_JUMP(LJMP, 0x6C97E0, GET_OFFSET(StorageClass_Wrappers::Operator_MinusEquals))
DEFINE_JUMP(LJMP, 0x6C9820, GET_OFFSET(StorageClass_Wrappers::First_Used_Slot))

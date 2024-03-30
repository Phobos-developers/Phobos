#include <Utilities/Macro.h>
#include "New/Entity/ExtendedStorageClass.h"

namespace StorageClass_Wrappers
{
	int __fastcall GetTotalValue(ExtendedStorageClass** ppStorage)
	{
		return (*ppStorage)->GetTotalValue();
	}

	double __fastcall GetTotalAmount(ExtendedStorageClass** ppStorage)
	{
		return (*ppStorage)->GetTotalAmount();
	}

	double __fastcall GetAmount(ExtendedStorageClass** ppStorage, void*, int index)
	{
		return (*ppStorage)->GetAmount(index);
	}

	float __fastcall IncreaseAmount(ExtendedStorageClass** ppStorage, void*, float amount, int index)
	{
		return (*ppStorage)->IncreaseAmount(amount, index);
	}

	float __fastcall DecreaseAmount(ExtendedStorageClass** ppStorage, void*, float amount, int index)
	{
		return (*ppStorage)->DecreaseAmount(amount, index);
	}

	ExtendedStorageClass** __fastcall Operator_Plus(ExtendedStorageClass** ppThis, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** ppThat)
	{
		**retstr = **ppThis + **ppThat;
		return retstr;
	}

	ExtendedStorageClass** __fastcall Operator_PlusEquals(ExtendedStorageClass** ppThis, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** ppThat)
	{
		**ppThis += **ppThat;
		*retstr = *ppThis;
		return ppThis;
	}

	ExtendedStorageClass** __fastcall Operator_Minus(ExtendedStorageClass** ppThis, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** ppThat)
	{
		**retstr = **ppThis - **ppThat;
		return retstr;
	}

	ExtendedStorageClass** __fastcall Operator_MinusEquals(ExtendedStorageClass** ppThis, void*, ExtendedStorageClass** retstr, ExtendedStorageClass** ppThat)
	{
		**ppThis -= **ppThat;
		*retstr = *ppThis;
		return ppThis;
	}

	int __fastcall FirstUsedSlot(ExtendedStorageClass** ppStorage)
	{
		return (*ppStorage)->FirstUsedSlot();
	}
}

DEFINE_JUMP(LJMP, 0x6C9600, GET_OFFSET(StorageClass_Wrappers::GetTotalValue))
DEFINE_JUMP(LJMP, 0x6C9650, GET_OFFSET(StorageClass_Wrappers::GetTotalAmount))
DEFINE_JUMP(LJMP, 0x6C9680, GET_OFFSET(StorageClass_Wrappers::GetAmount))
DEFINE_JUMP(LJMP, 0x6C9690, GET_OFFSET(StorageClass_Wrappers::IncreaseAmount))
DEFINE_JUMP(LJMP, 0x6C96B0, GET_OFFSET(StorageClass_Wrappers::DecreaseAmount))
DEFINE_JUMP(LJMP, 0x6C96E0, GET_OFFSET(StorageClass_Wrappers::Operator_Plus))
DEFINE_JUMP(LJMP, 0x6C9740, GET_OFFSET(StorageClass_Wrappers::Operator_PlusEquals))
DEFINE_JUMP(LJMP, 0x6C9780, GET_OFFSET(StorageClass_Wrappers::Operator_Minus))
DEFINE_JUMP(LJMP, 0x6C97E0, GET_OFFSET(StorageClass_Wrappers::Operator_MinusEquals))
DEFINE_JUMP(LJMP, 0x6C9820, GET_OFFSET(StorageClass_Wrappers::FirstUsedSlot))

#include <FactoryClass.h>
#include <HouseClass.h>

#include <Utilities/Macro.h>

#include <Ext/TechnoType/Body.h>

inline int __fastcall QueuedNum(const HouseClass* pHouse, const TechnoTypeClass* pType)
{
	const AbstractType absType = pType->WhatAmI();
	const FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare);
	int queued = 0;

	if (pFactory)
	{
		queued = pFactory->CountTotal(pType);

		if (const auto pObject = pFactory->Object)
		{
			if (pObject->GetType() == pType)
				--queued;
		}
	}

	return queued;
}

inline bool __fastcall ReachedBuildLimit(const HouseClass* pHouse, const TechnoTypeClass* pType, bool ignoreQueued)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->BuildLimit_Group_Types.empty()
		|| pTypeExt->BuildLimit_Group_Limits.empty())
		return false;


	if (pTypeExt->BuildLimit_Group_Limits.size() == 1)
	{
		int num = 0;

		for (const TechnoTypeClass* pTmpType : pTypeExt->BuildLimit_Group_Types)
		{
			if (!ignoreQueued)
				num += QueuedNum(pHouse, pTmpType);

			num += pHouse->CountOwnedNow(pTmpType);
		}

		if (num >= pTypeExt->BuildLimit_Group_Limits.back())
			return true;
	}
	else
	{
		size_t size = Math::min(pTypeExt->BuildLimit_Group_Limits.size(), pTypeExt->BuildLimit_Group_Types.size());
		bool reached = true;

		for (size_t i = 0; i < size; i++)
		{
			const TechnoTypeClass* pTmpType = pTypeExt->BuildLimit_Group_Types[i];
			int queued = ignoreQueued ? 0 : QueuedNum(pHouse, pTmpType);

			if (queued + pHouse->CountOwnedNow(pTmpType) >= pTypeExt->BuildLimit_Group_Limits[i])
			{
				if (pTypeExt->BuildLimit_Group_Any)
					return true;
			}
			else
			{
				reached = false;
			}
		}

		if (reached)
			return true;
	}

	return false;
}

DEFINE_HOOK(0x4F8361, HouseClass_CanBuild, 0x5)
{
	GET(const HouseClass*, pThis, ECX);
	GET_STACK(const TechnoTypeClass*, pType, 0x4);
	GET(CanBuildResult, aresResult, EAX);


	if (aresResult != CanBuildResult::Buildable)
		return 0;

	if (ReachedBuildLimit(pThis, pType, true))
		R->EAX(CanBuildResult::TemporarilyUnbuildable);

	return 0;
}

DEFINE_HOOK(0x50B669, HouseClass_ShouldDisableCameo, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pType, 0x4);
	GET(bool, aresDisable, EAX);

	if (aresDisable)
		return 0;

	if (pType == nullptr)
		return 0;

	if (ReachedBuildLimit(pThis, pType, false))
		R->EAX(true);

	return 0;
}

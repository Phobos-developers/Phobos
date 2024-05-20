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

inline void __fastcall RemoveProduction(const HouseClass* pHouse, const TechnoTypeClass* pType, int num)
{
	const AbstractType absType = pType->WhatAmI();
	FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare);
	if (pFactory)
	{
		int queued = pFactory->CountTotal(pType);
		if (num >= 0)
			queued = Math::min(num, queued);

		for (int i = 0; i < queued; i ++)
		{
			pFactory->RemoveOneFromQueue(pType);
		}
	}
}

inline bool __fastcall ReachedBuildLimit(const HouseClass* pHouse, const TechnoTypeClass* pType, bool ignoreQueued)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->BuildLimitGroup_Types.empty() || pTypeExt->BuildLimitGroup_Nums.empty())
		return false;

	if (pTypeExt->BuildLimitGroup_Nums.size() == 1)
	{
		int count = 0;
		int queued = 0;
		bool inside = false;

		for (const TechnoTypeClass* pTmpType : pTypeExt->BuildLimitGroup_Types)
		{
			if (!ignoreQueued)
				queued += QueuedNum(pHouse, pTmpType);

			count += pHouse->CountOwnedNow(pTmpType);

			if (pTmpType == pType)
				inside = true;
		}

		int num = count - pTypeExt->BuildLimitGroup_Nums.back();

		if (num + queued >= 0)
		{
			if (inside)
				RemoveProduction(pHouse, pType, num + queued);
			else if (num >= 0 || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}
	else
	{
		size_t size = Math::min(pTypeExt->BuildLimitGroup_Nums.size(), pTypeExt->BuildLimitGroup_Types.size());
		bool reached = true;
		bool realReached = true;

		for (size_t i = 0; i < size; i++)
		{
			const TechnoTypeClass* pTmpType = pTypeExt->BuildLimitGroup_Types[i];
			int queued = ignoreQueued ? 0 : QueuedNum(pHouse, pTmpType);
			int num = pHouse->CountOwnedNow(pTmpType) - pTypeExt->BuildLimitGroup_Nums[i];

			if (num + queued >= 0)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 0 || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, -1);

					return true;
				}
				else if (num < 0)
				{
					realReached = false;
				}
			}
			else
			{
				reached = false;
			}
		}

		if (reached)
		{
			if (realReached || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
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

	if (aresDisable || pType == nullptr)
		return 0;

	if (ReachedBuildLimit(pThis, pType, false))
		R->EAX(true);

	return 0;
}

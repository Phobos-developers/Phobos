
#include "AttachEffect.h"
#include "New/Entity/AttachEffectClass.h"
#include "New/Type/AttachEffectTypeClass.h"

DEFINE_EXPORT(int, AE_Attach,
	TechnoClass* pTarget,
	HouseClass* pInvokerHouse,
	TechnoClass* pInvoker,
	AbstractClass* pSource,
	const char** effectTypeNames,
	int typeCount,
	int durationOverride,
	int delay,
	int initialDelay,
	int recreationDelay
)
{
	if (!pTarget || !effectTypeNames || typeCount <= 0)
		return 0;

	AEAttachInfoTypeClass attachInfo;

	for (int i = 0; i < typeCount; i++)
	{
		if (effectTypeNames[i])
		{
			if (auto pType = AttachEffectTypeClass::Find(effectTypeNames[i]))
				attachInfo.AttachTypes.push_back(pType);
		}
	}

	if (attachInfo.AttachTypes.empty())
		return 0;

	if (durationOverride != 0)
		attachInfo.DurationOverrides.push_back(durationOverride);

	if (delay >= 0)
		attachInfo.Delays.push_back(delay);

	if (initialDelay >= 0)
		attachInfo.InitialDelays.push_back(initialDelay);

	if (recreationDelay >= -1)
		attachInfo.RecreationDelays.push_back(recreationDelay);

	return AttachEffectClass::Attach(pTarget, pInvokerHouse, pInvoker, pSource, attachInfo);
}

DEFINE_EXPORT(int, AE_Detach,
	TechnoClass* pTarget,
	const char** effectTypeNames,
	int typeCount
)
{
	if (!pTarget || !effectTypeNames || typeCount <= 0)
		return 0;

	AEAttachInfoTypeClass detachInfo;

	for (int i = 0; i < typeCount; i++)
	{
		if (effectTypeNames[i])
		{
			if (auto pType = AttachEffectTypeClass::Find(effectTypeNames[i]))
				detachInfo.RemoveTypes.push_back(pType);
		}
	}

	if (detachInfo.RemoveTypes.empty())
		return 0;

	return AttachEffectClass::Detach(pTarget, detachInfo);
}

DEFINE_EXPORT(int, AE_DetachByGroups,
	TechnoClass* pTarget,
	const char** groupNames,
	int groupCount
)
{
	if (!pTarget || !groupNames || groupCount <= 0)
		return 0;

	AEAttachInfoTypeClass detachInfo;

	for (int i = 0; i < groupCount; i++)
	{
		if (groupNames[i])
			detachInfo.RemoveGroups.push_back(groupNames[i]);
	}

	if (detachInfo.RemoveGroups.empty())
		return 0;

	return AttachEffectClass::DetachByGroups(pTarget, detachInfo);
}

DEFINE_EXPORT(void, AE_TransferEffects,
	TechnoClass* pSource,
	TechnoClass* pTarget
)
{
	if (!pSource || !pTarget)
		return;

	AttachEffectClass::TransferAttachedEffects(pSource, pTarget);
}

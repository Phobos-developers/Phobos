
#include "AttachEffect.h"
#include "New/Entity/AttachEffectClass.h"
#include "New/Type/AttachEffectTypeClass.h"
#include <climits>

constexpr int RecreationDelay_NoOverride = INT_MIN;

DEFINE_EXPORT(HRESULT, AE_Attach,
	TechnoClass* pTarget,
	HouseClass* pInvokerHouse,
	TechnoClass* pInvoker,
	AbstractClass* pSource,
	const char** effectTypeNames,
	int typeCount,
	int durationOverride,
	int delay,
	int initialDelay,
	int recreationDelay,
	int* pAttachedCount
)
{
	if (!pTarget || !effectTypeNames || !pAttachedCount)
		return E_POINTER;

	if (typeCount <= 0)
		return E_INVALIDARG;

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
	{
		*pAttachedCount = 0;
		return S_FALSE;
	}

	if (durationOverride != 0)
		attachInfo.DurationOverrides.push_back(durationOverride);

	if (delay >= 0)
		attachInfo.Delays.push_back(delay);

	if (initialDelay >= 0)
		attachInfo.InitialDelays.push_back(initialDelay);

	if (recreationDelay != RecreationDelay_NoOverride)
		attachInfo.RecreationDelays.push_back(recreationDelay);

	*pAttachedCount = AttachEffectClass::Attach(pTarget, pInvokerHouse, pInvoker, pSource, attachInfo);
	return S_OK;
}

DEFINE_EXPORT(HRESULT, AE_Detach,
	TechnoClass* pTarget,
	const char** effectTypeNames,
	int typeCount,
	int* pRemovedCount
)
{
	if (!pTarget || !effectTypeNames || !pRemovedCount)
		return E_POINTER;

	if (typeCount <= 0)
		return E_INVALIDARG;

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
	{
		*pRemovedCount = 0;
		return S_FALSE;
	}

	*pRemovedCount = AttachEffectClass::Detach(pTarget, detachInfo);
	return S_OK;
}

DEFINE_EXPORT(HRESULT, AE_DetachByGroups,
	TechnoClass* pTarget,
	const char** groupNames,
	int groupCount,
	int* pRemovedCount
)
{
	if (!pTarget || !groupNames || !pRemovedCount)
		return E_POINTER;

	if (groupCount <= 0)
		return E_INVALIDARG;

	AEAttachInfoTypeClass detachInfo;

	for (int i = 0; i < groupCount; i++)
	{
		if (groupNames[i])
			detachInfo.RemoveGroups.push_back(groupNames[i]);
	}

	if (detachInfo.RemoveGroups.empty())
	{
		*pRemovedCount = 0;
		return S_FALSE;
	}

	*pRemovedCount = AttachEffectClass::DetachByGroups(pTarget, detachInfo);
	return S_OK;
}

DEFINE_EXPORT(HRESULT, AE_TransferEffects,
	TechnoClass* pSource,
	TechnoClass* pTarget
)
{
	if (!pSource || !pTarget)
		return E_POINTER;

	AttachEffectClass::TransferAttachedEffects(pSource, pTarget);
	return S_OK;
}

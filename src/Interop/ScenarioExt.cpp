#include "ScenarioExt.h"
#include <Ext/Scenario/Body.h>

static HRESULT GetVariableImpl(bool bIsGlobal, int index, int* pValue)
{
	if (!pValue)
		return E_POINTER;

	auto pGlobal = ScenarioExt::Global();

	if (!pGlobal)
		return E_FAIL;

	auto& dict = pGlobal->Variables[bIsGlobal];
	auto itr = dict.find(index);

	if (itr != dict.end())
	{
		*pValue = itr->second.Value;
		return S_OK;
	}

	*pValue = 0;
	return S_FALSE;
}

static HRESULT SetVariableImpl(bool bIsGlobal, int index, int value)
{
	auto pGlobal = ScenarioExt::Global();

	if (!pGlobal)
		return E_FAIL;

	pGlobal->Variables[bIsGlobal][index].Value = value;
	return S_OK;
}

DEFINE_EXPORT(HRESULT, Variables_GetLocal_Phobos, int index, int* pValue)
{
	return GetVariableImpl(false, index, pValue);
}

DEFINE_EXPORT(HRESULT, Variables_SetLocal_Phobos, int index, int value)
{
	return SetVariableImpl(false, index, value);
}

DEFINE_EXPORT(HRESULT, Variables_GetGlobal_Phobos, int index, int* pValue)
{
	return GetVariableImpl(true, index, pValue);
}

DEFINE_EXPORT(HRESULT, Variables_SetGlobal_Phobos, int index, int value)
{
	return SetVariableImpl(true, index, value);
}

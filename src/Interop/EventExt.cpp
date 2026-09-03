#include "EventExt.h"

DEFINE_EXPORT(HRESULT, EventExt_AddEvent, EventExt* pEventExt)
{
	if (!pEventExt)
		return E_POINTER;

	return pEventExt->AddEvent() ? S_OK : S_FALSE;
}

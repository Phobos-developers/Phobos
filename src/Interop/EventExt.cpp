#include "EventExt.h"

DEFINE_EXPORT(bool, EventExt_AddEvent, EventExt* pEventExt)
{
	if (pEventExt)
	{
		return pEventExt->AddEvent();
	}
	return false;
}

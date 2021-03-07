#include "Body.h"
#include <Helpers/Macro.h>
#include <BulletClass.h>
#include <HouseClass.h>

#include "../BulletType/Body.h"

DEFINE_HOOK(4666F7, BulletClass_Update, 6)
{
	GET(BulletClass*, pThis, EBP);

	auto pExt = BulletExt::ExtMap.Find(pThis);
	if (pExt->ShouldIntercept)
	{
		pThis->Detonate(pThis->GetCoords());
		pThis->Remove();
		pThis->UnInit();
	}

	if (pExt->Intercepted)
		pExt->ShouldIntercept = true;

	return 0;
}

#include "Body.h"

#include <Ext/VoxelAnimType/Body.h>

DEFINE_HOOK(0x74A70E, VoxelAnimClass_AI_Additional, 0xC)
{
	GET(VoxelAnimClass* const, pThis, EBX);

	if (auto pType = pThis->Type)
	{
		if (auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pType))
		{
			if (auto pExt = VoxelAnimExt::ExtMap.Find(pThis))
			{
				if (!pExt->LaserTrails.empty())
				{
					CoordStruct location = pThis->GetCoords();
					CoordStruct drawnCoords = location;
					for (auto const& trail : pExt->LaserTrails)
					{
						if (!trail->LastLocation.isset())
							trail->LastLocation = location;

						trail->Visible = pThis->IsVisible;
						trail->Update(drawnCoords);
					}
				}
			}
		}
	}

	return 0;
}
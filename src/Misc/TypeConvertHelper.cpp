#include "Misc/TypeConvertHelper.h"

void TypeConvertHelper::Convert(FootClass* pTargetFoot, ConvertPairs& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toTypeIdx, affectedHouses] : convertPairs)
	{
		if (!EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTargetFoot->Owner))
			continue;

		const auto toType = TechnoTypeClass::Array->GetItem(toTypeIdx);
		if (fromTypes.size())
		{
			for (const auto& from : fromTypes)
			{
				// Check if the target matches upgrade-from TechnoType and it has something to upgrade to
				if (from == pTargetFoot->GetTechnoType())
				{
					TechnoExt::ConvertToType(pTargetFoot, toType);
					break;
				}
			}
		}
		else
		{
			TechnoExt::ConvertToType(pTargetFoot, toType);
		}
	}
}

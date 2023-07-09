#pragma once

#include <Utilities/EnumFunctions.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>

namespace TypeConvertHelper
{
	typedef std::vector<std::tuple<ValueableVector<TechnoTypeClass*>, NullableIdx<TechnoTypeClass>, Nullable<AffectedHouse>>> ConvertPairs;

	void Convert(FootClass* pTargetFoot, ConvertPairs& convertPairs, HouseClass* pOwner, AnimTypeClass* pTypeAnim);
}

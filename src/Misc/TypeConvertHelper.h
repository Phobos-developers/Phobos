#pragma once

#include <Utilities/EnumFunctions.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>

class TypeConvertGroup
{
public:
	ValueableVector<TechnoTypeClass*> FromTypes;
	Nullable<TechnoTypeClass*> ToType;
	Nullable<AffectedHouse> AppliedTo;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

namespace TypeConvertHelper
{
	typedef std::vector<std::tuple<ValueableVector<TechnoTypeClass*>, NullableIdx<TechnoTypeClass>, Nullable<AffectedHouse>>> ConvertPairs;

	void Convert(FootClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner, AnimTypeClass* pTypeAnim);
}

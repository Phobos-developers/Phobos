#pragma once

#include <Utilities/TemplateDef.h>

class AnimTypeClass;

class TypeConvertGroup
{
public:
	ValueableVector<TechnoTypeClass*> FromTypes;
	Nullable<TechnoTypeClass*> ToType;
	Nullable<AffectedHouse> AppliedTo;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static void Parse(std::vector<TypeConvertGroup>& list, INI_EX& exINI, const char* section, AffectedHouse defaultAffectHouse);

	static void Convert(FootClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner, AnimTypeClass* pAnimType = nullptr);
	static void ConvertSW(const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner, AnimTypeClass* pAnimType = nullptr);

private:
	template <typename T>
	bool Serialize(T& stm);
};

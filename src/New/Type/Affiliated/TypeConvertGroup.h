#pragma once

#include <Utilities/TemplateDef.h>

class TypeConvertGroup
{
public:
	ValueableVector<TechnoTypeClass*> FromTypes;
	Nullable<TechnoTypeClass*> ToType;
	Nullable<AffectedHouse> AppliedTo;

	// Called by MultiflagValueableVector, do not call directly.
	bool Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, AffectedHouse& defaultAffectsHouse);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static void Convert(TechnoClass* pTarget, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner);
	static void ConvertSW(const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner);

private:
	template <typename T>
	bool Serialize(T& stm);
};

// Declared after TypeConvertGroup is complete, so the MultiflagValueableVector
// constraint (which requires TypeConvertGroup::Read) can be evaluated.
// Read() also handles the un-numbered legacy tags for backward compatibility.
class TypeConvertGroupList : public MultiflagValueableVector<TypeConvertGroup, AffectedHouse>
{
public:
	void Read(INI_EX& parser, const char* const pSection, AffectedHouse defaultAffectsHouse);
};

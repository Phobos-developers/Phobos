#include <Ext/Techno/Body.h>
#include "TypeConvertGroup.h"

void TypeConvertGroup::Convert(FootClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toType, affectedHouses] : convertPairs)
	{
		if (!toType.Get())
			continue;

		if (pOwner && !EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTargetFoot->Owner))
			continue;

		if (fromTypes.size())
		{
			for (const auto& from : fromTypes)
			{
				// Check if the target matches upgrade-from TechnoType and it has something to upgrade to
				if (from == pTargetFoot->GetTechnoType())
				{
					TechnoExt::ConvertToType(pTargetFoot, toType);
					goto end; // Breaking out of nested loops without extra checks one of the very few remaining valid usecases for goto, leave it be.
				}
			}
		}
		else
		{
			TechnoExt::ConvertToType(pTargetFoot, toType);
			break;
		}
	}
end:
	return;
}


bool TypeConvertGroup::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TypeConvertGroup::Save(PhobosStreamWriter& stm) const
{
	return const_cast<TypeConvertGroup*>(this)->Serialize(stm);
}

void TypeConvertGroup::Parse(std::vector<TypeConvertGroup>& list, INI_EX& exINI, const char* pSection, AffectedHouse defaultAffectHouse)
{
	Parse(list, exINI, pSection, defaultAffectHouse, "Convert");
}

// If the header's length plus "1.AffectedHouse" exceeds the capacity of the tempBuffer the game will crash.
// For example, there are 35 characters in "They.Effect.Convert0.AffectedHouses", meaning it crashes the game if the buffer size was only 32.
// The buffer size 64 should be enough for most cases, however it should be furtherly expanded if even longer headers are expected.
void TypeConvertGroup::Parse(std::vector<TypeConvertGroup>& list, INI_EX& exINI, const char* pSection, AffectedHouse defaultAffectHouse, const char* header)
{
	char tempBuffer[64];
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<TechnoTypeClass*> convertFrom;
		Nullable<TechnoTypeClass*> convertTo;
		Nullable<AffectedHouse> convertAffectedHouses;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d.From", header, i);
		convertFrom.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d.To", header, i);
		convertTo.Read(exINI, pSection, tempBuffer);
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d.AffectedHouses", header, i);
		convertAffectedHouses.Read(exINI, pSection, tempBuffer);

		if (!convertTo.isset())
			break;

		if (!convertAffectedHouses.isset())
			convertAffectedHouses = defaultAffectHouse;

		list.emplace_back(convertFrom, convertTo, convertAffectedHouses);
	}
	ValueableVector<TechnoTypeClass*> convertFrom;
	Nullable<TechnoTypeClass*> convertTo;
	Nullable<AffectedHouse> convertAffectedHouses;
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.From", header);
	convertFrom.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.To", header);
	convertTo.Read(exINI, pSection, tempBuffer);
	_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.AffectedHouses", header);
	convertAffectedHouses.Read(exINI, pSection, tempBuffer);
	if (convertTo.isset())
	{
		if (!convertAffectedHouses.isset())
			convertAffectedHouses = defaultAffectHouse;

		if (list.size())
			list[0] = { convertFrom, convertTo, convertAffectedHouses };
		else
			list.emplace_back(convertFrom, convertTo, convertAffectedHouses);
	}
}

template <typename T>
bool TypeConvertGroup::Serialize(T& stm)
{
	return stm
		.Process(this->FromTypes)
		.Process(this->ToType)
		.Process(this->AppliedTo)
		.Success();
}

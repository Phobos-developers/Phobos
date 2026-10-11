#include <Ext/Techno/Body.h>
#include "TypeConvertGroup.h"

void TypeConvertGroup::Convert(TechnoClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toType, affectedHouses] : convertPairs)
	{
		if (!toType.Get())
			continue;

		if (pOwner && !EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTargetFoot->Owner))
			continue;

		if (fromTypes.size())
		{
			const auto pType = pTargetFoot->GetTechnoType();

			for (const auto& from : fromTypes)
			{
				// Check if the target matches upgrade-from TechnoType and it has something to upgrade to
				if (from == pType)
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

void TypeConvertGroup::ConvertSW(const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toType, affectedHouses] : convertPairs)
	{
		if (!toType.Get())
			continue;

		if (fromTypes.size())
		{
			auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
			{
				std::vector<T> vec(dvc.Count);
				std::copy(dvc.begin(), dvc.end(), vec.begin());
				return vec;
			};

			for (const auto& from : fromTypes)
			{
				auto const items = copy_dvc(TechnoTypeExt::Fetch(from)->Array);

				for (const auto pTarget : items)
				{
					if (!pTarget || (pOwner && !EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTarget->Owner)))
						continue;

					TechnoExt::ConvertToType(pTarget, toType);
				}
			}
		}
		else
		{
			for (auto const pTarget : TechnoClass::Array)
			{
				TypeConvertGroup::Convert(pTarget, convertPairs, pOwner);
			}
		}
	}

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

bool TypeConvertGroup::Read(INI_EX& parser, const char* const pSection, const char* const pBaseFlag, AffectedHouse& defaultAffectsHouse)
{
	char flagName[0x40];
	ValueableVector<TechnoTypeClass*> convertFrom;
	Nullable<TechnoTypeClass*> convertTo;
	Nullable<AffectedHouse> convertAffectsHouse;

	_snprintf_s(flagName, sizeof(flagName), _TRUNCATE, "%s.From", pBaseFlag);
	convertFrom.Read(parser, pSection, flagName);
	_snprintf_s(flagName, sizeof(flagName), _TRUNCATE, "%s.To", pBaseFlag);
	convertTo.Read(parser, pSection, flagName);
	_snprintf_s(flagName, sizeof(flagName), _TRUNCATE, "%s.AffectedHouses", pBaseFlag); // Temporary solution for the INI tags renaming issue, see #2093
	convertAffectsHouse.Read(parser, pSection, flagName);
	if (convertAffectsHouse.isset())
	{
		Debug::Log("[Developer warning][%s] %s is deprecated and has been replaced by %s.AffectsHouse! If both are set, the latter will be used.\n",
			pSection, flagName, pBaseFlag);
	}
	_snprintf_s(flagName, sizeof(flagName), _TRUNCATE, "%s.AffectsHouse", pBaseFlag);
	convertAffectsHouse.Read(parser, pSection, flagName);

	if (!convertTo.isset())
		return false;

	if (!convertAffectsHouse.isset())
		convertAffectsHouse = defaultAffectsHouse;

	*this = { std::move(convertFrom), std::move(convertTo), std::move(convertAffectsHouse) };

	return true;
}

void TypeConvertGroupList::Read(INI_EX& parser, const char* const pSection, AffectedHouse defaultAffectsHouse)
{
	MultiflagValueableVector::Read(parser, pSection, "Convert", defaultAffectsHouse);

	// Un-numbered tags, kept for backward compatibility, override the first pair if present
	TypeConvertGroup legacyPair;

	if (legacyPair.Read(parser, pSection, "Convert", defaultAffectsHouse))
	{
		if (this->size())
			(*this)[0] = std::move(legacyPair);
		else
			this->push_back(std::move(legacyPair));
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

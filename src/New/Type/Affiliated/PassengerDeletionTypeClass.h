#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class PassengerDeletionTypeClass
{
public:

	PassengerDeletionTypeClass() = default;

	PassengerDeletionTypeClass(TechnoTypeClass* pOwnerType);

	TechnoTypeClass* OwnerType;

	Valueable<int> Rate;
	Valueable<bool> Rate_SizeMultiply;
	Valueable<bool> UseCostAsRate;
	Valueable<double> CostMultiplier;
	Nullable<int> CostRateCap;
	Valueable<AffectedHouse> AllowedHouses;
	Valueable<bool> DontScore;
	Valueable<bool> Soylent;
	Valueable<double> SoylentMultiplier;
	Valueable<AffectedHouse> SoylentAllowedHouses;
	Valueable<bool> DisplaySoylent;
	Valueable<AffectedHouse> DisplaySoylentToHouses;
	Valueable<Point2D> DisplaySoylentOffset;
	ValueableIdx<VocClass> ReportSound;
	Valueable<AnimTypeClass*> Anim;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::pair<bool,bool> CanParse(INI_EX exINI, const char* pSection);

private:

	template <typename T>
	bool Serialize(T& stm);
};

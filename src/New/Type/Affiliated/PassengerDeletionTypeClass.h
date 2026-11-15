#pragma once

#include <Utilities/TemplateDef.h>

class PassengerDeletionTypeClass
{
public:

	PassengerDeletionTypeClass() = default;

	Valueable<int> Rate { 0 };
	Valueable<bool> Rate_SizeMultiply { true };
	Valueable<bool> UseCostAsRate { false };
	Valueable<double> CostMultiplier { 1.0 };
	Nullable<int> CostRateCap {};
	Valueable<AffectedHouse> AllowedHouses { AffectedHouse::All };
	Valueable<bool> DontScore { false };
	Valueable<bool> Soylent { false };
	Valueable<double> SoylentMultiplier { 1.0 };
	Valueable<AffectedHouse> SoylentAllowedHouses { AffectedHouse::Enemies };
	Valueable<bool> DisplaySoylent { false };
	Valueable<AffectedHouse> DisplaySoylentToHouses { AffectedHouse::All };
	Valueable<Point2D> DisplaySoylentOffset { { 0,0} };
	ValueableIdx<VocClass> ReportSound {};
	ValueableVector<AnimTypeClass*> Anim {};
	Valueable<bool> UnderEMP { false };

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static std::pair<bool, bool> CanParse(INI_EX exINI, const char* pSection);

private:

	template <typename T>
	bool Serialize(T& stm);
};

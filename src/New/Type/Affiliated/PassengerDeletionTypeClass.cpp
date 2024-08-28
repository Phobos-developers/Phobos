#include "PassengerDeletionTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

std::pair<bool,bool> PassengerDeletionTypeClass::CanParse(INI_EX exINI, const char* pSection)
{
	Nullable<int> rate;
	rate.Read(exINI, pSection, "PassengerDeletion.Rate");
	Nullable<bool> useCost;
	useCost.Read(exINI, pSection, "PassengerDeletion.UseCostAsRate");

	bool canParse = rate.Get(0) > 0 || useCost.Get(false);
	bool shouldResetValue = rate.isset() && rate.Get() == 0 && !(useCost.isset() && useCost.Get());
	return std::make_pair(canParse, shouldResetValue);
}

PassengerDeletionTypeClass::PassengerDeletionTypeClass(TechnoTypeClass* pOwnerType)
	: OwnerType(pOwnerType)
	, Rate { 0 }
	, Rate_SizeMultiply { true }
	, UseCostAsRate { false }
	, CostMultiplier { 1.0 }
	, CostRateCap {}
	, AllowedHouses { AffectedHouse::All }
	, DontScore { false }
	, Soylent { false }
	, SoylentMultiplier { 1.0 }
	, SoylentAllowedHouses { AffectedHouse::Enemies }
	, DisplaySoylent { false }
	, DisplaySoylentToHouses { AffectedHouse::All }
	, DisplaySoylentOffset { { 0, 0 } }
	, ReportSound {}
	, Anim {}
{
}
void PassengerDeletionTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Rate.Read(exINI, pSection, "PassengerDeletion.Rate");
	this->Rate_SizeMultiply.Read(exINI, pSection, "PassengerDeletion.Rate.SizeMultiply");
	this->UseCostAsRate.Read(exINI, pSection, "PassengerDeletion.UseCostAsRate");
	this->CostMultiplier.Read(exINI, pSection, "PassengerDeletion.CostMultiplier");
	this->CostRateCap.Read(exINI, pSection, "PassengerDeletion.CostRateCap");
	this->AllowedHouses.Read(exINI, pSection, "PassengerDeletion.AllowedHouses");
	this->DontScore.Read(exINI, pSection, "PassengerDeletion.DontScore");
	this->Soylent.Read(exINI, pSection, "PassengerDeletion.Soylent");
	this->SoylentMultiplier.Read(exINI, pSection, "PassengerDeletion.SoylentMultiplier");
	this->SoylentAllowedHouses.Read(exINI, pSection, "PassengerDeletion.SoylentAllowedHouses");
	this->DisplaySoylent.Read(exINI, pSection, "PassengerDeletion.DisplaySoylent");
	this->DisplaySoylentToHouses.Read(exINI, pSection, "PassengerDeletion.DisplaySoylentToHouses");
	this->DisplaySoylentOffset.Read(exINI, pSection, "PassengerDeletion.DisplaySoylentOffset");
	this->ReportSound.Read(exINI, pSection, "PassengerDeletion.ReportSound");
	this->Anim.Read(exINI, pSection, "PassengerDeletion.Anim");
}

#pragma region(save/load)

template <class T>
bool PassengerDeletionTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->OwnerType)
		.Process(this->Rate)
		.Process(this->Rate_SizeMultiply)
		.Process(this->UseCostAsRate)
		.Process(this->CostMultiplier)
		.Process(this->CostRateCap)
		.Process(this->AllowedHouses)
		.Process(this->DontScore)
		.Process(this->Soylent)
		.Process(this->SoylentMultiplier)
		.Process(this->SoylentAllowedHouses)
		.Process(this->DisplaySoylent)
		.Process(this->DisplaySoylentToHouses)
		.Process(this->DisplaySoylentOffset)
		.Process(this->ReportSound)
		.Process(this->Anim)
		.Success();
}

bool PassengerDeletionTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool PassengerDeletionTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<PassengerDeletionTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

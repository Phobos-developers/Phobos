#include "MobileRefineryTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

std::pair<bool, bool> MobileRefineryTypeClass::CanParse(INI_EX exINI, const char* pSection)
{
	Nullable<bool> isRefinery;
	isRefinery.Read(exINI, pSection, "MobileRefinery");

	bool canParse = isRefinery.Get(false);
	bool shouldResetValue = isRefinery.isset() && !isRefinery.Get();
	return std::make_pair(canParse, shouldResetValue);
}

MobileRefineryTypeClass::MobileRefineryTypeClass(TechnoTypeClass* OwnedBy) : OwnerType { OwnedBy }
, TransDelay { 30 }
, CashMultiplier { 1.0 }
, AmountPerCell { 0 }
, FrontOffset {}
, LeftOffset {}
, Display { true }
, Display_House { AffectedHouse::All }
, Anims {}
, AnimMove { true }
{ }

void MobileRefineryTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->TransDelay.Read(exINI, pSection, "MobileRefinery.TransDelay");
	this->CashMultiplier.Read(exINI, pSection, "MobileRefinery.CashMultiplier");
	this->AmountPerCell.Read(exINI, pSection, "MobileRefinery.AmountPerCell");
	this->FrontOffset.Read(exINI, pSection, "MobileRefinery.FrontOffset");
	this->LeftOffset.Read(exINI, pSection, "MobileRefinery.LeftOffset");
	this->Display.Read(exINI, pSection, "MobileRefinery.Display");
	this->Display_House.Read(exINI, pSection, "MobileRefinery.Display.House");
	this->Anims.Read(exINI, pSection, "MobileRefinery.Anims");
	this->AnimMove.Read(exINI, pSection, "MobileRefinery.AnimMove");
}

#pragma region(save/load)

template <class T>
bool MobileRefineryTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->OwnerType)
		.Process(this->TransDelay)
		.Process(this->CashMultiplier)
		.Process(this->AmountPerCell)
		.Process(this->FrontOffset)
		.Process(this->LeftOffset)
		.Process(this->Display)
		.Process(this->Display_House)
		.Process(this->Anims)
		.Process(this->AnimMove)
		.Success();
}

bool MobileRefineryTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool MobileRefineryTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<MobileRefineryTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

#include "GiftBoxTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

GiftBoxTypeClass::GiftBoxTypeClass(TechnoTypeClass* OwnedBy) : OwnerType { OwnedBy }
	, TechnoList {}
	, Count {}
	, Remove { true }
	, Destroy { false }
	, Delay { 0 }
	, DelayMinMax { { 0, 0 } }
	, RandomRange { false }
	, EmptyCell { false }
	, RandomType { true }
{ }

void GiftBoxTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->TechnoList.Read(exINI, pSection, "GiftBox.Types");
	this->Count.Read(exINI, pSection, "GiftBox.Nums");
	this->Remove.Read(exINI, pSection, "GiftBox.Remove");
	this->Destroy.Read(exINI, pSection, "GiftBox.Destroy");
	this->Delay.Read(exINI, pSection, "GiftBox.Delay");
	this->DelayMinMax.Read(exINI, pSection, "GiftBox.RandomDelay");
	this->RandomRange.Read(exINI, pSection, "GiftBox.CellRandomRange");
	this->EmptyCell.Read(exINI, pSection, "GiftBox.EmptyCell");
	this->RandomType.Read(exINI, pSection, "GiftBox.RandomType");
}

#pragma region(save/load)

template <class T>
bool GiftBoxTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->TechnoList)
		.Process(this->Count)
		.Process(this->Remove)
		.Process(this->Destroy)
		.Process(this->Delay)
		.Process(this->DelayMinMax)
		.Process(this->RandomRange)
		.Process(this->EmptyCell)
		.Process(this->RandomType)
		.Success();
}

bool GiftBoxTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool GiftBoxTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<GiftBoxTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

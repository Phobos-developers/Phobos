#include "TiberiumEaterTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

void TiberiumEaterTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	char tempBuffer[32];

	this->TransDelay.Read(exINI, pSection, "TiberiumEater.TransDelay");
	this->CashMultiplier.Read(exINI, pSection, "TiberiumEater.CashMultiplier");
	this->AmountPerCell.Read(exINI, pSection, "TiberiumEater.AmountPerCell");
	this->FrontOffset.Read(exINI, pSection, "TiberiumEater.FrontOffset");
	this->LeftOffset.Read(exINI, pSection, "TiberiumEater.LeftOffset");
	this->Display.Read(exINI, pSection, "TiberiumEater.Display");
	this->DisplayToHouse.Read(exINI, pSection, "TiberiumEater.DisplayToHouse");
	this->DisplayOffset.Read(exINI, pSection, "TiberiumEater.DisplayOffset");
	this->Anims.Read(exINI, pSection, "TiberiumEater.Anims");

	for (size_t idx = 0; idx < 4; ++idx)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "TiberiumEater.Anims.Tiberium%d", idx);
		this->Anims_Tiberiums[idx].Read(exINI, pSection, tempBuffer);
	}

	this->AnimMove.Read(exINI, pSection, "TiberiumEater.AnimMove");
}

template <class T>
bool TiberiumEaterTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->TransDelay)
		.Process(this->CashMultiplier)
		.Process(this->AmountPerCell)
		.Process(this->FrontOffset)
		.Process(this->LeftOffset)
		.Process(this->Display)
		.Process(this->DisplayToHouse)
		.Process(this->DisplayOffset)
		.Process(this->Anims)
		.Process(this->Anims_Tiberiums)
		.Process(this->AnimMove)
		.Success();
}

#pragma region(save/load)

bool TiberiumEaterTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TiberiumEaterTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<TiberiumEaterTypeClass*>(this)->Serialize(stm);
}

#pragma endregion

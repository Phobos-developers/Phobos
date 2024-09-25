#include "HealthBarTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Ext/Techno/Body.h>

template<>
const char* Enumerable<HealthBarTypeClass>::GetMainSection()
{
	return "HealthBarTypes";
}

void HealthBarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Frame_Background.Read(exINI, section, "Frame.Background");
	this->Frame_Background_ShowWhenNotSelected.Read(exINI, section, "Frame.Background.ShowWhenNotSelected");
	this->Frame_Background_Translucency.Read(exINI, section, "Frame.Background.Translucency");
	this->Frame_Foreground.Read(exINI, section, "Frame.Foreground");
	this->Frame_Foreground_ShowWhenNotSelected.Read(exINI, section, "Frame.Foreground.ShowWhenNotSelected");
	this->HealthBar_XOffset.Read(exINI, section, "HealthBar.OffsetX");
	this->Sections_Pips.Read(exINI, section, "Sections.Pips");
	this->Sections_Amount.Read(exINI, section, "Sections.Amount");
	this->Sections_Size.Read(exINI, section, "Sections.Size");

}

template <typename T>
void HealthBarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Frame_Background)
		.Process(this->Frame_Background_ShowWhenNotSelected)
		.Process(this->Frame_Background_Translucency)
		.Process(this->Frame_Foreground)
		.Process(this->Frame_Foreground_ShowWhenNotSelected)
		.Process(this->HealthBar_XOffset)
		.Process(this->Sections_Pips)
		.Process(this->Sections_Amount)
		.Process(this->Sections_Size)
		;
}

void HealthBarTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void HealthBarTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

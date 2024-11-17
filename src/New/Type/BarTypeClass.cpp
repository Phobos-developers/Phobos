#include "BarTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Ext/Techno/Body.h>

template<>
const char* Enumerable<BarTypeClass>::GetMainSection()
{
	return "BarTypes";
}

void BarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Board_Background_File.Read(exINI, section, "Board.Background.File");
	this->Board_Background_ShowWhenNotSelected.Read(exINI, section, "Board.Background.ShowWhenNotSelected");
	this->Board_Background_Translucency.Read(exINI, section, "Board.Background.Translucency");
	this->Board_Foreground_File.Read(exINI, section, "Board.Foreground.File");
	this->Board_Foreground_ShowWhenNotSelected.Read(exINI, section, "Board.Foreground.ShowWhenNotSelected");
	this->Board_Foreground_Translucency.Read(exINI, section, "Board.Foreground.Translucency");
	this->Board_Offset.Read(exINI, section, "Board.Offset");
	this->Bar_Offset.Read(exINI, section, "Bar.Offset");
	this->Sections_DrawBackwards.Read(exINI, section, "Sections.DrawBackwards");
	this->Sections_Pips_File.Read(exINI, section, "Sections.Pips.File");
	this->Sections_Pips.Read(exINI, section, "Sections.Pips");
	this->Sections_EmptyPip.Read(exINI, section, "Sections.EmptyPip");
	this->Sections_Amount.Read(exINI, section, "Sections.Amount");
	this->Sections_PositionDelta.Read(exINI, section, "Sections.PositionDelta");

}

template <typename T>
void BarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Board_Background_File)
		.Process(this->Board_Background_ShowWhenNotSelected)
		.Process(this->Board_Background_Translucency)
		.Process(this->Board_Foreground_File)
		.Process(this->Board_Foreground_ShowWhenNotSelected)
		.Process(this->Board_Foreground_Translucency)
		.Process(this->Board_Offset)
		.Process(this->Bar_Offset)
		.Process(this->Sections_DrawBackwards)
		.Process(this->Sections_Pips_File)
		.Process(this->Sections_Pips)
		.Process(this->Sections_EmptyPip)
		.Process(this->Sections_Amount)
		.Process(this->Sections_PositionDelta)
		;
}

void BarTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void BarTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

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

	this->BoardBG_File.Read(exINI, section, "BoardBG.File");
	this->BoardBG_ShowWhenNotSelected.Read(exINI, section, "BoardBG.ShowWhenNotSelected");
	this->BoardBG_Translucency.Read(exINI, section, "BoardBG.Translucency");
	this->BoardFG_File.Read(exINI, section, "BoardFG.File");
	this->BoardFG_ShowWhenNotSelected.Read(exINI, section, "BoardFG.ShowWhenNotSelected");
	this->BoardFG_Translucency.Read(exINI, section, "BoardFG.Translucency");
	this->Board_Offset.Read(exINI, section, "Board.Offset");
	this->Bar_Offset.Read(exINI, section, "Bar.Offset");
	this->Sections_DrawBackwards.Read(exINI, section, "Sections.DrawBackwards");
	this->Sections_Pips.Read(exINI, section, "Sections.Pips");
	this->Sections_EmptyPip.Read(exINI, section, "Sections.EmptyPip");
	this->Sections_Amount.Read(exINI, section, "Sections.Amount");
	this->Sections_PositionDelta.Read(exINI, section, "Sections.PositionDelta");

}

template <typename T>
void BarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->BoardBG_File)
		.Process(this->BoardBG_ShowWhenNotSelected)
		.Process(this->BoardBG_Translucency)
		.Process(this->BoardFG_File)
		.Process(this->BoardFG_ShowWhenNotSelected)
		.Process(this->BoardFG_Translucency)
		.Process(this->Board_Offset)
		.Process(this->Bar_Offset)
		.Process(this->Sections_DrawBackwards)
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

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

	this->InfoType.Read(exINI, section, "InfoType");
	this->PipBrd_Background_File.Read(exINI, section, "PipBrd.Background.File");
	this->PipBrd_Background_ShowWhenNotSelected.Read(exINI, section, "PipBrd.Background.ShowWhenNotSelected");
	this->PipBrd_Background_Translucency.Read(exINI, section, "PipBrd.Background.Translucency");
	this->PipBrd_Foreground_File.Read(exINI, section, "PipBrd.Foreground.File");
	this->PipBrd_Foreground_ShowWhenNotSelected.Read(exINI, section, "PipBrd.Foreground.ShowWhenNotSelected");
	this->PipBrd_Foreground_Translucency.Read(exINI, section, "PipBrd.Foreground.Translucency");
	this->PipBrd_Offset.Read(exINI, section, "PipBrd.Offset");
	this->Bar_Offset.Read(exINI, section, "Bar.Offset");
	this->Pips_File.Read(exINI, section, "Pips.File");
	this->Pips_Frames.Read(exINI, section, "Pips.Frames");
	this->Pips_EmptyFrame.Read(exINI, section, "Pips.EmptyFrame");
	this->Pips_Amount.Read(exINI, section, "Pips.Amount");
	this->Pips_PositionDelta.Read(exINI, section, "Pips.PositionDelta");
	this->Pips_DrawBackwards.Read(exINI, section, "Pips.DrawBackwards");

}

template <typename T>
void BarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->InfoType)
		.Process(this->PipBrd_Background_File)
		.Process(this->PipBrd_Background_ShowWhenNotSelected)
		.Process(this->PipBrd_Background_Translucency)
		.Process(this->PipBrd_Foreground_File)
		.Process(this->PipBrd_Foreground_ShowWhenNotSelected)
		.Process(this->PipBrd_Foreground_Translucency)
		.Process(this->PipBrd_Offset)
		.Process(this->Bar_Offset)
		.Process(this->Pips_File)
		.Process(this->Pips_Frames)
		.Process(this->Pips_EmptyFrame)
		.Process(this->Pips_Amount)
		.Process(this->Pips_PositionDelta)
		.Process(this->Pips_DrawBackwards)
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

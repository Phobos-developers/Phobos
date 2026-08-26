#include "ResourceTypeClass.h"

template<>
const char* Enumerable<ResourceTypeClass>::GetMainSection()
{
	return "ResourceTypes";
}

Valueable<ResourceDisplayOrientation> ResourceTypeClass::Global_Display_Orientation { ResourceDisplayOrientation::Vertical };
Valueable<ResourceDisplayAnchor> ResourceTypeClass::Global_Display_Anchor { ResourceDisplayAnchor::TopRight };
Valueable<Point2D> ResourceTypeClass::Global_Display_BaseOffset { Point2D::Empty };
Valueable<int> ResourceTypeClass::Global_Display_Spacing { 14 };

ResourceTypeClass::ResourceTypeClass(const char* pTitle) : Enumerable<ResourceTypeClass>(pTitle)
	, Display_Label {}
	, Display_Label_InvertPosition { false }
	, Display_Label_UseSpace { true }
	, Display_Color { { 255, 255, 255 } }
	, Display_Condition { ResourceDisplayCondition::GreaterThanZero }
	, Display_Offset {}
	, InitialValue { 0 }
	, RequiresCollector { false }
	, Bounty_Enabled { false }
	, Bounty_DefaultValue { 0 }
	, Bounty_DefaultFriendlyValue { 0 }
	, Bounty_CanUseStandardPoints { false }
	, Bounty_MoneyConversion { 0 }
{
}

void ResourceTypeClass::LoadGlobalsFromINI(CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	Global_Display_Orientation.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Orientation");
	Global_Display_Anchor.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Anchor");
	Global_Display_BaseOffset.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.BaseOffset");
	Global_Display_Spacing.Read(exINI, GameStrings::AudioVisual, "Display.ResourceTypes.Spacing");
}

void ResourceTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;
	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	this->Display_Label.Read(exINI, section, "Display.Label");
	this->Display_Label_InvertPosition.Read(exINI, section, "Display.Label.InvertPosition");
	this->Display_Label_UseSpace.Read(exINI, section, "Display.Label.UseSpace");
	this->Display_Color.Read(exINI, section, "Display.Color");
	this->Display_Condition.Read(exINI, section, "Display.Condition");
	this->Display_Offset.Read(exINI, section, "Display.Offset");

	this->InitialValue.Read(exINI, section, "InitialValue");
	this->RequiresCollector.Read(exINI, section, "RequiresCollector");

	this->Bounty_Enabled.Read(exINI, section, "Bounty.Enabled");
	this->Bounty_DefaultValue.Read(exINI, section, "Bounty.DefaultValue");
	this->Bounty_DefaultFriendlyValue.Read(exINI, section, "Bounty.DefaultFriendlyValue");
	this->Bounty_CanUseStandardPoints.Read(exINI, section, "Bounty.CanUseStandardPoints");
	this->Bounty_MoneyConversion.Read(exINI, section, "Bounty.MoneyConversion");
}

template <typename T>
void ResourceTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Display_Label)
		.Process(this->Display_Label_InvertPosition)
		.Process(this->Display_Label_UseSpace)
		.Process(this->Display_Color)
		.Process(this->Display_Condition)
		.Process(this->Display_Offset)
		.Process(this->InitialValue)
		.Process(this->RequiresCollector)
		.Process(this->Bounty_Enabled)
		.Process(this->Bounty_DefaultValue)
		.Process(this->Bounty_DefaultFriendlyValue)
		.Process(this->Bounty_CanUseStandardPoints)
		.Process(this->Bounty_MoneyConversion);
}

void ResourceTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ResourceTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

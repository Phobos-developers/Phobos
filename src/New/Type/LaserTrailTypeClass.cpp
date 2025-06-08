#include "LaserTrailTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

template<>
const char* Enumerable<LaserTrailTypeClass>::GetMainSection()
{
	return "LaserTrailTypes";
}

void LaserTrailTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->DrawType.Read(exINI, section, "DrawType");

	this->IsHouseColor.Read(exINI, section, "IsHouseColor");
	this->Color.Read(exINI, section, "Color");
	this->Thickness.Read(exINI, section, "Thickness");

	this->IsAlternateColor.Read(exINI, section, "IsAlternateColor");

	this->FadeDuration.Read(exINI, section, "FadeDuration");
	this->SegmentLength.Read(exINI, section, "SegmentLength");
	this->IgnoreVertical.Read(exINI, section, "IgnoreVertical");
	this->IsIntense.Read(exINI, section, "IsIntense");
	this->CloakVisible.Read(exINI, section, "CloakVisible");
	this->CloakVisible_DetectedOnly.Read(exINI, section, "CloakVisible.DetectedOnly");
	this->DroppodOnly.Read(exINI, section, "DropPodOnly");
}

template <typename T>
void LaserTrailTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->DrawType)
		.Process(this->IsHouseColor)
		.Process(this->Color)
		.Process(this->Thickness)
		.Process(this->IsAlternateColor)
		.Process(this->FadeDuration)
		.Process(this->SegmentLength)
		.Process(this->IgnoreVertical)
		.Process(this->IsIntense)
		.Process(this->CloakVisible)
		.Process(this->CloakVisible_DetectedOnly)
		.Process(this->DroppodOnly)
		;
}

void LaserTrailTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void LaserTrailTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

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

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);
	char tempBuffer[0x40];

	this->DrawType.Read(exINI, section, "DrawType");

	this->IsHouseColor.Read(exINI, section, "IsHouseColor");
	this->Color.Read(exINI, section, "Color");
	this->Thickness.Read(exINI, section, "Thickness");

	this->IsAlternateColor.Read(exINI, section, "IsAlternateColor");

	for (int idx = 0; idx < 3; ++idx)
	{
		_snprintf_s(tempBuffer, _TRUNCATE, "Bolt.Color%d", idx + 1);
		this->Bolt_Color[idx].Read(exINI, section, tempBuffer);

		_snprintf_s(tempBuffer, _TRUNCATE, "Bolt.Disable%d", idx + 1);
		this->Bolt_Disable[idx].Read(exINI, section, tempBuffer);
	}

	this->Bolt_Arcs.Read(exINI, section, "Bolt.Arcs");

	this->Beam_Color.Read(exINI, section, "Beam.Color");
	this->Beam_Amplitude.Read(exINI, section, "Beam.Amplitude");

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
		.Process(this->Bolt_Color)
		.Process(this->Bolt_Disable)
		.Process(this->Bolt_Arcs)
		.Process(this->Beam_Color)
		.Process(this->Beam_Amplitude)
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

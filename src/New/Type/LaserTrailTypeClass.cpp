#include "LaserTrailTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

Enumerable<LaserTrailTypeClass>::container_t Enumerable<LaserTrailTypeClass>::Array;

const char* Enumerable<LaserTrailTypeClass>::GetMainSection()
{
    return "LaserTrailTypes";
}


/*
    
    [LaserTrailName]
    IsHouseColor=no
    IsRainbow=yes
    Color=255,0,0
    Duration=15
    Thickness=8
    Distance=32
    IgnoreVertical=no
    IsIntense=no
    
*/
void LaserTrailTypeClass::LoadFromINI(CCINIClass* pINI)
{
    const char* section = this->Name;

    INI_EX exINI(pINI);

    this->IsHouseColor.Read(exINI, section, "IsHouseColor");
    this->IsRainbowColor.Read(exINI, section, "IsRainbowColor");

	for (size_t i = 0; i <= this->Colors.size(); ++i)
	{
		Nullable<ColorStruct> color;
		_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Color%d", i);
		color.Read(exINI, section, Phobos::readBuffer);

		if (i == this->Colors.size() && !color.isset())
			break;
        else if (!color.isset())
            continue;

        // first config (game rules)
        // [LaserTrail]
        // IsSweepingColor=yes
        // Color0=255,0,0
        // Color1=255,255,0
        // Color2=0,255,0

        // second config (game rule edits in map file)
        // [LaserTrail]
        // Color0=128,0,0
        // Color2=0,128,0

		// Valueable<CoordStruct> flh;
		// _snprintf_s(Phobos::readBuffer, Phobos::readLength, "LaserTrail%d.FLH", i);
		// flh.Read(exINI, section, Phobos::readBuffer);

		if (i == this->Colors.size())
			this->Colors.push_back(color);
		else
			this->Colors[i] = color;
	}

    this->TransitionDuration.Read(exINI, section, "TransitionDuration");
    this->Duration.Read(exINI, section, "Duration");
    this->Thickness.Read(exINI, section, "Thickness");
    this->Distance.Read(exINI, section, "Distance");
    this->IgnoreVertical.Read(exINI, section, "IgnoreVertical");
    this->IsIntense.Read(exINI, section, "IsIntense");
}

template <typename T>
void LaserTrailTypeClass::Serialize(T& Stm)
{
    Stm
        .Process(this->IsHouseColor)
        .Process(this->IsRainbowColor)
        .Process(this->Colors)
        .Process(this->TransitionDuration)
        .Process(this->Duration)
        .Process(this->Thickness)
        .Process(this->Distance)
        .Process(this->IgnoreVertical)
        .Process(this->IsIntense)
        ;
};

void LaserTrailTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
    this->Serialize(Stm);
}

void LaserTrailTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
    this->Serialize(Stm);
}

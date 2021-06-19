#include "LaserTrailTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

Enumerable<LaserTrailTypeClass>::container_t Enumerable<LaserTrailTypeClass>::Array;

const char* Enumerable<LaserTrailTypeClass>::GetMainSection()
{
    return "LaserTrailTypes";
}

void LaserTrailTypeClass::LoadFromINI(CCINIClass* pINI)
{
    const char* section = this->Name;

    INI_EX exINI(pINI);

    this->IsHouseColor.Read(exINI, section, "IsHouseColor");
    this->Color.Read(exINI, section, "Color");

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
        .Process(this->Color)
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

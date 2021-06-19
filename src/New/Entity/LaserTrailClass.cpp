#include "LaserTrailClass.h"

#include <Utilities/TemplateDef.h>

// Draws LaserTrail if the conditions are suitable.
// Returns true if drawn, false otherwise.
bool LaserTrailClass::Draw(CoordStruct location)
{
    Debug::Log("LaserTrailClass::Draw runs\n");
    bool result = false;

    if (!this->LastLocation.isset())
    {
        // The trail was just inited
        this->LastLocation = location;
    }
    else if (location.DistanceFrom(this->LastLocation.Get()) > this->Type->Distance) // TODO reimplement IgnoreVertical properly?
    {
        if (this->Visible && (this->Type->IgnoreVertical ? (abs(location.X - this->LastLocation.Get().X) > 16 || abs(location.Y - this->LastLocation.Get().Y) > 16) : true))
        {
            // We spawn new laser segment if the distance is long enough, the game will do the rest - Kerbiter
            LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
                this->LastLocation.Get(), location,
                this->CurrentColor.Get(), ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 },
                this->Type->Duration.Get());

            pLaser->Thickness = this->Type->Thickness;
            pLaser->IsHouseColor = true;
            pLaser->IsSupported = this->Type->IsIntense;

            result = true;
        }

        this->LastLocation = location;
    }

    return result;
}

#pragma region Save/Load

template <typename T>
void LaserTrailClass::Serialize(T & Stm)
{
    Stm
        .Process(this->Type)
        .Process(this->Visible)
        .Process(this->LastLocation)
        ;
};

void LaserTrailClass::LoadFromStream(PhobosStreamReader & Stm)
{
    this->Serialize(Stm);
}

void LaserTrailClass::SaveToStream(PhobosStreamWriter & Stm)
{
    this->Serialize(Stm);
}

#pragma endregion
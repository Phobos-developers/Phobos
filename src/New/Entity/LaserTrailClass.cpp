#include "LaserTrailClass.h"

#include <Utilities/TemplateDef.h>

// Draws LaserTrail if the conditions are suitable.
// Returns true if drawn, false otherwise.
bool LaserTrailClass::Update(CoordStruct location)
{
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
                this->GetCurrentColor(), ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 },
				this->Type->FadeDuration.Get());

			pLaser->Thickness = this->Type->Thickness;
			pLaser->IsHouseColor = true;
			pLaser->IsSupported = this->Type->IsIntense;

			result = true;
		}

		this->LastLocation = location;
	}

    (*this->FramesPassed.GetEx())++;

    return result;
}

ColorStruct LaserTrailClass::GetCurrentColor()
{
    if (this->Type->IsRainbowColor.Get())
    {
        auto& colors = this->Type->Colors;

        int transitionCycle = (this->FramesPassed / this->Type->TransitionDuration)
            % colors.size();
        int currentColorIndex = transitionCycle;
        int nextColorIndex = (transitionCycle + 1) % colors.size();

        double blendingCoef = (this->FramesPassed % this->Type->TransitionDuration)
            / this->Type->TransitionDuration;

        ColorStruct color = {
            (BYTE)(colors[currentColorIndex].R * (1 - blendingCoef) + colors[nextColorIndex].R * blendingCoef),
            (BYTE)(colors[currentColorIndex].G * (1 - blendingCoef) + colors[nextColorIndex].G * blendingCoef),
            (BYTE)(colors[currentColorIndex].B * (1 - blendingCoef) + colors[nextColorIndex].B * blendingCoef)
        };

        // Debug::Log("%d %d %d", color.R, color.G, color.B);
        return color;
    }

    return this->Type->Colors[0];
}

BYTE LaserTrailClass::GetColorChannel(int cycle, int offset)
{
    cycle = (cycle + offset) % 6;

    bool maskDown = !((cycle / 3) % 2);
    bool maskUp = !(((cycle + 1) / 3) % 2);

    BYTE colorUp = (this->FramesPassed * 16) % 256;
    BYTE colorDown = 255 - colorUp;

    return colorUp*maskUp + colorDown*maskDown;
}

#pragma region Save/Load

template <typename T>
bool LaserTrailClass::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->Visible)
		.Process(this->FLH)
		.Process(this->IsOnTurret)
		.Process(this->CurrentColor)
		.Process(this->LastLocation)
		.Success();
};

bool LaserTrailClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool LaserTrailClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<LaserTrailClass*>(this)->Serialize(stm);
}

#pragma endregion
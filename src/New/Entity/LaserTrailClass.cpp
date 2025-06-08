#include "LaserTrailClass.h"
#include <EBolt.h>

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
	else if (location.DistanceFrom(this->LastLocation.Get()) > this->Type->SegmentLength) // TODO reimplement IgnoreVertical properly?
	{
		if (this->Visible && !this->Cloaked && (this->Type->IgnoreVertical ? (abs(location.X - this->LastLocation.Get().X) > 16 || abs(location.Y - this->LastLocation.Get().Y) > 16) : true))
		{
			if (this->Type->DrawType == LaserTrailDrawType::Laser)
			{
				// We spawn new laser segment if the distance is long enough, the game will do the rest - Kerbiter
				LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
					this->LastLocation.Get(), location,
					this->CurrentColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 },
					this->Type->FadeDuration.Get(64));

				pLaser->Thickness = this->Type->Thickness;
				pLaser->IsHouseColor = true;
				pLaser->IsSupported = this->Type->IsIntense;
			}
			else if (this->Type->DrawType == LaserTrailDrawType::EBolt)
			{
				const auto pBolt = GameCreate<EBolt>();
				pBolt->Lifetime = 1 << (std::clamp(this->Type->FadeDuration.Get(17), 1, 31) - 1);
				pBolt->AlternateColor = this->Type->IsAlternateColor;
				pBolt->Fire(this->LastLocation, location, 0);
			}
			else if (this->Type->DrawType == LaserTrailDrawType::RadBeam)
			{
				const ColorStruct beamColor = RulesClass::Instance->RadColor;

				const auto pRadBeam = RadBeam::Allocate(RadBeamType::Temporal);
				pRadBeam->SetCoordsSource(this->LastLocation);
				pRadBeam->SetCoordsTarget(location);
				pRadBeam->Period = 15;
				pRadBeam->Amplitude = 40.0;
				pRadBeam->SetColor(beamColor);
			}

			result = true;
		}

		this->LastLocation = location;
	}

	return result;
}

#pragma region Save/Load

template <typename T>
bool LaserTrailClass::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->Visible)
		.Process(this->Cloaked)
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

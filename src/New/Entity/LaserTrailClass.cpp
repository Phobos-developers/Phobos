#include "LaserTrailClass.h"

#include <Utilities/TemplateDef.h>
#include <Ext/EBolt/Body.h>

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
				const auto pBoltExt = EBoltExt::ExtMap.Allocate(pBolt);

				const int alternateIdx = this->Type->IsAlternateColor ? 5 : 10;
				const COLORREF defaultAlternate = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, alternateIdx);
				const COLORREF defaultWhite = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);

				if (this->Type->Bolt_Disable1)
					pBoltExt->Disable1 = true;
				else if (this->Type->Bolt_Color1.isset())
					pBoltExt->Color1 = this->Type->Bolt_Color1.Get();
				else
					pBoltExt->Color1 = Drawing::Int_To_RGB(defaultAlternate);

				if (this->Type->Bolt_Disable2)
					pBoltExt->Disable2 = true;
				else if (this->Type->Bolt_Color2.isset())
					pBoltExt->Color2 = this->Type->Bolt_Color2.Get();
				else
					pBoltExt->Color2 = Drawing::Int_To_RGB(defaultAlternate);

				if (this->Type->Bolt_Disable3)
					pBoltExt->Disable3 = true;
				else if (this->Type->Bolt_Color3.isset())
					pBoltExt->Color3 = this->Type->Bolt_Color3.Get();
				else
					pBoltExt->Color3 = Drawing::Int_To_RGB(defaultWhite);

				pBoltExt->Arcs = this->Type->Bolt_Arcs;
				pBolt->Lifetime = 1 << (std::clamp(this->Type->FadeDuration.Get(17), 1, 31) - 1);
				pBolt->AlternateColor = this->Type->IsAlternateColor;

				pBolt->Fire(this->LastLocation, location, 0);
			}
			else if (this->Type->DrawType == LaserTrailDrawType::RadBeam)
			{
				const ColorStruct beamColor = this->Type->Beam_Color.Get(RulesClass::Instance->RadColor);

				const auto pRadBeam = RadBeam::Allocate(RadBeamType::Temporal);
				pRadBeam->SetCoordsSource(this->LastLocation);
				pRadBeam->SetCoordsTarget(location);
				pRadBeam->Period = this->Type->FadeDuration.Get(15);
				pRadBeam->Amplitude = this->Type->Beam_Amplitude;
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

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
		auto const pType = this->Type;

		// We spawn new laser segment if the distance is long enough, the game will do the rest - Kerbiter
		if (this->Visible && !this->Cloaked && (pType->IgnoreVertical ? (abs(location.X - this->LastLocation.Get().X) > 16 || abs(location.Y - this->LastLocation.Get().Y) > 16) : true))
		{
			if (pType->DrawType == LaserTrailDrawType::Laser)
			{
				const auto pLaser = GameCreate<LaserDrawClass>(
					this->LastLocation.Get(), location,
					this->CurrentColor, ColorStruct { 0, 0, 0 }, ColorStruct { 0, 0, 0 },
					pType->FadeDuration.Get(64));

				pLaser->Thickness = pType->Thickness;
				pLaser->IsHouseColor = true;
				pLaser->IsSupported = pType->IsIntense;
			}
			else if (pType->DrawType == LaserTrailDrawType::EBolt)
			{
				const auto pBolt = GameCreate<EBolt>();
				const auto pBoltExt = EBoltExt::ExtMap.Find(pBolt);
				const auto& boltDisable = pType->Bolt_Disable;
				const auto& boltColor = pType->Bolt_Color;

				const int alternateIdx = pType->IsAlternateColor ? 5 : 10;
				const int defaultAlternate = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, alternateIdx);
				const int defaultWhite = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);

				for (int idx = 0; idx < 3; ++idx)
				{
					if (boltDisable[idx])
						pBoltExt->Disable[idx] = true;
					else if (boltColor[idx].isset())
						pBoltExt->Color[idx] = boltColor[idx].Get();
					else
						pBoltExt->Color[idx] = Drawing::Int_To_RGB(idx < 2 ? defaultAlternate : defaultWhite);
				}

				pBoltExt->Arcs = pType->Bolt_Arcs;
				pBolt->Lifetime = 1 << (std::clamp(pType->FadeDuration.Get(17), 1, 31) - 1);
				pBolt->AlternateColor = pType->IsAlternateColor;

				pBolt->Fire(this->LastLocation, location, 0);
			}
			else if (pType->DrawType == LaserTrailDrawType::RadBeam)
			{
				const auto pRadBeam = RadBeam::Allocate(RadBeamType::RadBeam);
				pRadBeam->SetCoordsSource(this->LastLocation);
				pRadBeam->SetCoordsTarget(location);
				pRadBeam->Period = pType->FadeDuration.Get(15);
				pRadBeam->Amplitude = pType->Beam_Amplitude;

				const ColorStruct beamColor = pType->Beam_Color.Get(RulesClass::Instance->RadColor);
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
		.Process(this->Intrinsic)
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

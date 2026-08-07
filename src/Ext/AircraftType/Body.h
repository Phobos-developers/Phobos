#pragma once

#include <Ext/TechnoType/Body.h>
#include <AircraftTypeClass.h>

// Concrete leaf extension for AircraftTypeClass (empty).
class AircraftTypeExt final : public TechnoTypeExt
{
public:
	using base_type = AircraftTypeClass;

	static constexpr DWORD Canary = 0xA5A6A7A8;

	NullableIdx<VocClass> VoicePickup; // Used by carryalls instead of VoiceMove if set.
	Nullable<EdgeType> SpawnFromEdge;
	Nullable<EdgeType> RetreatToEdge;
	Nullable<Leptons> SpawnDistanceFromTarget;
	Nullable<int> SpawnHeight;
	Nullable<int> LandingDir;
	Nullable<bool> CurleyShuffle;
	Nullable<bool> ExtendedAircraftMissions;
	Nullable<bool> ExtendedAircraftMissions_SmoothMoving;
	Nullable<bool> ExtendedAircraftMissions_EarlyDescend;
	Nullable<bool> ExtendedAircraftMissions_RearApproach;
	Nullable<bool> ExtendedAircraftMissions_FastScramble;
	Nullable<int> ExtendedAircraftMissions_UnlandDamage;
	Nullable<bool> FiringForceScatter;
	Nullable<int> ParadropDelay;
	Nullable<int> ParadropEndDelay;
	Nullable<bool> FlyNoWobbles;
	Nullable<bool> IsALoaner;
	Nullable<AnimTypeClass*> LandingAnim;
	Valueable<bool> Missile_Cruise;
	Valueable<AnimTypeClass*> Missile_TakeOffAnim;
	Valueable<int> Missile_TakeOffSeparation;

	explicit AircraftTypeExt(AircraftTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
		, VoicePickup {}
		, SpawnFromEdge {}
		, RetreatToEdge {}
		, SpawnDistanceFromTarget {}
		, SpawnHeight {}
		, LandingDir {}
		, CurleyShuffle {}
		, ExtendedAircraftMissions {}
		, ExtendedAircraftMissions_SmoothMoving {}
		, ExtendedAircraftMissions_EarlyDescend {}
		, ExtendedAircraftMissions_RearApproach {}
		, ExtendedAircraftMissions_FastScramble {}
		, ExtendedAircraftMissions_UnlandDamage {}
		, FiringForceScatter {}
		, ParadropDelay {}
		, ParadropEndDelay {}
		, FlyNoWobbles {}
		, IsALoaner {}
		, LandingAnim {}
		, Missile_Cruise { false }
		, Missile_TakeOffAnim { nullptr }
		, Missile_TakeOffSeparation { 24 }
	{ }

	AircraftTypeClass* OwnerObject() const
	{
		return static_cast<AircraftTypeClass*>(this->GetAttachedObject());
	}

	class ExtContainer final : public Container<AircraftTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static AircraftTypeExt* Fetch(const AircraftTypeClass* pThis)
	{
		return AbstractExt::Fetch<AircraftTypeExt>(pThis);
	}

	static AircraftTypeExt* TryFetch(const AircraftTypeClass* pThis)
	{
		return AbstractExt::TryFetch<AircraftTypeExt>(pThis);
	}

	virtual void Initialize() override;
	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};

#pragma once
#include <Ext/Techno/Body.h>
#include <Ext/Foot/Body.h>
#include <AircraftClass.h>

// Concrete leaf extension for AircraftClass (empty; techno data lives in TechnoExt).
class AircraftExt final : public FootExt
{
public:
	using base_type = AircraftClass;

	static constexpr DWORD Canary = 0xA1A2A3A4;

	explicit AircraftExt(AircraftClass* const OwnerObject) : FootExt(OwnerObject)
	{ }

	AircraftClass* OwnerObject() const
	{
		return static_cast<AircraftClass*>(this->GetAttachedObject());
	}

	static void FireWeapon(AircraftClass* pThis, AbstractClass* pTarget);
	static bool PlaceReinforcementAircraft(AircraftClass* pThis, CoordStruct edgeCoords);
	static CellStruct PickEdgeCellForPlane(AircraftTypeClass* pPlaneType, CellStruct destCell, Edge edge, bool isOnRetreat = false);
	static DirType GetLandingDir(AircraftClass* pThis, BuildingClass* pDock = nullptr);

	class ExtContainer final : public Container<AircraftExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static AircraftExt* Fetch(const AircraftClass* pThis)
	{
		return AbstractExt::Fetch<AircraftExt>(pThis);
	}

	static AircraftExt* TryFetch(const AircraftClass* pThis)
	{
		return AbstractExt::TryFetch<AircraftExt>(pThis);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};

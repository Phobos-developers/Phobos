#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <TActionClass.h>

class HouseClass;

enum class PhobosTriggerAction : unsigned int
{
	SaveGame = 500,
	EditVariable = 501,
	GenerateRandomNumber = 502,
	PrintVariableValue = 503,
	BinaryOperation = 504,
	RunSuperWeaponAtLocation = 505,
	RunSuperWeaponAtWaypoint = 506,
	ToggleMCVRedeploy = 510,
	UndeployToWaypoint = 511,
	SetFollowsIndexForVehicle = 512,
	SetMissionTimer = 513,

	SetDropCrate = 600,

	EditAngerNode = 606,
	ClearAngerNode = 607,
	SetForceEnemy = 608,
	SetFreeRadar = 609,
	SetTeamDelay = 610,
	SetNextScanario = 611,

	CreateBannerLocal = 800, // any banner w/ local variable
	CreateBannerGlobal = 801, // any banner w/ global variable
	DeleteBanner = 802,
};

class TActionExt final : public AbstractExt
{
public:
	using base_type = TActionClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TActionExt;

	static constexpr DWORD Canary = 0x91919191;

public:
	// typed owner accessor
	TActionClass* OwnerObject() const
	{
		return static_cast<TActionClass*>(this->GetAttachedObject());
	}

	TActionExt(TActionClass* const OwnerObject) : AbstractExt(OwnerObject)
	{ }

	virtual ~TActionExt() = default;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static bool Execute(TActionClass* pThis, HouseClass* pHouse,
			ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& bHandled);

#pragma push_macro("ACTION_FUNC")
#define ACTION_FUNC(name) \
	static bool name(TActionClass* pThis, HouseClass* pHouse, \
		ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)

	ACTION_FUNC(PlayAudioAtRandomWP);
	ACTION_FUNC(SaveGame);
	ACTION_FUNC(EditVariable);
	ACTION_FUNC(GenerateRandomNumber);
	ACTION_FUNC(PrintVariableValue);
	ACTION_FUNC(BinaryOperation);
	ACTION_FUNC(RunSuperWeaponAtLocation);
	ACTION_FUNC(RunSuperWeaponAtWaypoint);
	ACTION_FUNC(ToggleMCVRedeploy);
	ACTION_FUNC(UndeployToWaypoint);
	ACTION_FUNC(SetFollowsIndexForVehicle);

	ACTION_FUNC(SetMissionTimer);

	ACTION_FUNC(SetDropCrate);

	ACTION_FUNC(EditAngerNode);
	ACTION_FUNC(ClearAngerNode);
	ACTION_FUNC(SetForceEnemy);
	ACTION_FUNC(SetFreeRadar);
	ACTION_FUNC(SetTeamDelay);
	ACTION_FUNC(SetNextScanario);

	ACTION_FUNC(CreateBannerLocal);
	ACTION_FUNC(CreateBannerGlobal);
	ACTION_FUNC(DeleteBanner);

	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")

public:
	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TActionExt* Fetch(const TActionClass* pThis)
	{
		return AbstractExt::Fetch<TActionExt>(pThis);
	}

	static TActionExt* TryFetch(const TActionClass* pThis)
	{
		return AbstractExt::TryFetch<TActionExt>(pThis);
	}
};


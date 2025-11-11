#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

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

	EditAngerNode = 606,
	ClearAngerNode = 607,
	SetForceEnemy = 608,

	CreateBannerLocal = 800, // any banner w/ local variable
	CreateBannerGlobal = 801, // any banner w/ global variable
	DeleteBanner = 802,

	WinByID = 19001,
	LoseByID = 19002,
	ProductionBeginsByID = 19003,
	AllToHuntByID = 19006,
	FireSaleByID = 19009,
	AutocreateBeginsByID = 19013,
	ChangeHouseByID = 19014,
	PlayMusicThemeByID = 19020,
	AddOneTimeSuperWeaponByID = 19033,
	AddRepeatingSuperWeaponByID = 19034,
	AllChangeHouseByID = 19036,
	MakeAllyByID = 19037,
	MakeEnemyByID = 19038,
	PlayAnimAtByID = 19041,
	DoExplosionAtByID = 19042,
	CreateVoxelAnimByID = 19043,
	AITriggersBeginByID = 19074,
	AITriggersStopByID = 19075,
	ParticleAnimByID = 19088,
	MakeHouseCheerByID = 19113,
	DestroyAllByID = 19119,
	DestroyAllBuildingsByID = 19120,
	DestroyAllLandUnitsByID = 19121,
	DestroyAllNavalUnitsByID = 19122,
	MindControlBaseByID = 19123,
	RestoreMindControlledBaseByID = 19124,
	RestoreStartingUnitsByID = 19126,
	RestoreStartingBuildingsByID = 19130,
};

class TActionExt
{
public:
	using base_type = TActionClass;

	static constexpr DWORD Canary = 0x91919191;

	class ExtData final : public Extension<TActionClass>
	{
	public:
		ExtData(TActionClass* const OwnerObject) : Extension<TActionClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

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

	ACTION_FUNC(EditAngerNode);
	ACTION_FUNC(ClearAngerNode);
	ACTION_FUNC(SetForceEnemy);

	ACTION_FUNC(CreateBannerLocal);
	ACTION_FUNC(CreateBannerGlobal);
	ACTION_FUNC(DeleteBanner);

	static bool RunSuperWeaponAt(TActionClass* pThis, int X, int Y);

#undef ACTION_FUNC
#pragma pop_macro("ACTION_FUNC")

	class ExtContainer final : public Container<TActionExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

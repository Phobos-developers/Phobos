#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>

class Matrix3D;

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	class ExtData final : public Extension<TechnoTypeClass>
	{
	public:
		Valueable<bool> HealthBar_Hide;
		Valueable<CSFText> UIDescription;
		Valueable<bool> LowSelectionPriority;
		PhobosFixedString<0x20> GroupAs;
		Valueable<int> RadarJamRadius;
		Valueable<int> InhibitorRange;
		Valueable<Leptons> MindControlRangeLimit;
		Valueable<bool> Interceptor;
		Valueable<Leptons> Interceptor_GuardRange;
		Valueable<Leptons> Interceptor_MinimumGuardRange;
		Valueable<Leptons> Interceptor_EliteGuardRange;
		Valueable<Leptons> Interceptor_EliteMinimumGuardRange;
		Valueable<CoordStruct> TurretOffset;
		Valueable<bool> Powered_KillSpawns;
		Valueable<bool> Spawn_LimitedRange;
		Valueable<int> Spawn_LimitedExtraRange;
		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<int> CameoPriority;
		Valueable<bool> NoManualMove;
		Nullable<int> InitialStrength;
		Valueable<bool> PassengerDeletion_Soylent;
		Valueable<bool> PassengerDeletion_SoylentFriendlies;
		Valueable<int> PassengerDeletion_Rate;
		NullableIdx<VocClass> PassengerDeletion_ReportSound;
		Valueable<bool> PassengerDeletion_Rate_SizeMultiply;

		Valueable<ShieldTypeClass*> ShieldType;

		Nullable<AnimTypeClass*> WarpOut;
		Nullable<AnimTypeClass*> WarpIn;
		Nullable<AnimTypeClass*> WarpAway;
		Nullable<bool> ChronoTrigger;
		Nullable<int> ChronoDistanceFactor;
		Nullable<int> ChronoMinimumDelay;
		Nullable<int> ChronoRangeMinimum;
		Nullable<int> ChronoDelay;

		ValueableVector<AnimTypeClass*> OreGathering_Anims;
		ValueableVector<int> OreGathering_Tiberiums;
		ValueableVector<int> OreGathering_FramesPerDir;
		
		std::vector<DynamicVectorClass<CoordStruct>> WeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteWeaponBurstFLHs;

		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> NotHuman_RandomDeathSequence;

		Nullable<InfantryTypeClass*> DefaultDisguise;

		Nullable<int> OpenTopped_RangeBonus;
		Nullable<float> OpenTopped_DamageMultiplier;
		Nullable<int> OpenTopped_WarpDistance;

		struct LaserTrailDataEntry
		{
			ValueableIdx<LaserTrailTypeClass> idxType;
			Valueable<CoordStruct> FLH;
			Valueable<bool> IsOnTurret;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<LaserTrailDataEntry> LaserTrailData;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject),
			HealthBar_Hide(false),
			UIDescription(),
			LowSelectionPriority(false),
			GroupAs(NONE_STR),
			RadarJamRadius(0),
			InhibitorRange(0),
			MindControlRangeLimit(),
			Interceptor(false),
			Interceptor_GuardRange(),
			Interceptor_MinimumGuardRange(),
			Interceptor_EliteGuardRange(),
			Interceptor_EliteMinimumGuardRange(),
			TurretOffset({0, 0, 0}),
			Powered_KillSpawns(false),
			Spawn_LimitedRange(false),
			Spawn_LimitedExtraRange(0),
			Harvester_Counted(),
			Promote_IncludeSpawns(false),
			ImmuneToCrit(false),
			MultiMindControl_ReleaseVictim(false),
			CameoPriority(0),
			NoManualMove(false),
			InitialStrength(),
			ShieldType(),
			WarpOut(),
			WarpIn(),
			WarpAway(),
			ChronoTrigger(),
			ChronoDistanceFactor(),
			ChronoMinimumDelay(),
			ChronoRangeMinimum(),
			ChronoDelay(),
			OreGathering_Anims(),
			OreGathering_Tiberiums(),
			OreGathering_FramesPerDir(),
			LaserTrailData(),
			DestroyAnim_Random(true),
			NotHuman_RandomDeathSequence(false),
			PassengerDeletion_Soylent(false),
			PassengerDeletion_SoylentFriendlies(false),
			PassengerDeletion_Rate(0),
			PassengerDeletion_ReportSound(),
			PassengerDeletion_Rate_SizeMultiply(true),
			DefaultDisguise(),
			OpenTopped_RangeBonus(),
			OpenTopped_DamageMultiplier(),
			OpenTopped_WarpDistance()
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0);
		bool IsCountedAsHarvester();

		// Ares 0.A
		const char* GetSelectionGroupID() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor = 1.0);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
};
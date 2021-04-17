#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

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
		Valueable<double> MindControlRangeLimit;
		Valueable<bool> Interceptor;
		Valueable<double> Interceptor_GuardRange;
		Valueable<double> Interceptor_EliteGuardRange;
		Valueable<CoordStruct> TurretOffset;
		Valueable<bool> Powered_KillSpawns;
		Valueable<bool> Spawn_LimitedRange;
		Valueable<int> Spawn_LimitedExtraRange;
		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;

		Valueable<int> Shield_Strength;
		Valueable<signed int> Shield_Armor;
		Valueable<double> Shield_Respawn;
		Valueable<double> Shield_Respawn_Rate;
		Valueable<double> Shield_SelfHealing;
		Valueable<double> Shield_SelfHealing_Rate;
		Valueable<bool> Shield_AbsorbOverDamage;
		Valueable<int> Shield_BracketDelta;
		Nullable<AnimTypeClass*> Shield_IdleAnim;
		Nullable<AnimTypeClass*> Shield_BreakAnim;
		Nullable<AnimTypeClass*> Shield_RespawnAnim;
		Nullable<AnimTypeClass*> Shield_HitAnim;

		Nullable<AnimTypeClass*> WarpOut;
		Nullable<AnimTypeClass*> WarpIn;
		Nullable<AnimTypeClass*> WarpAway;
		Nullable<bool> ChronoTrigger;
		Nullable<int> ChronoDistanceFactor;
		Nullable<int> ChronoMinimumDelay;
		Nullable<int> ChronoRangeMinimum;
		Nullable<int> ChronoDelay;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject),
			HealthBar_Hide(false),
			UIDescription(),
			LowSelectionPriority(false),
			GroupAs(NONE_STR),
			MindControlRangeLimit(-1.0),
			Interceptor(false),
			Interceptor_GuardRange(0.0),
			Interceptor_EliteGuardRange(0.0),
			TurretOffset({0, 0, 0}),
			Powered_KillSpawns(false),
			Spawn_LimitedRange(false),
			Spawn_LimitedExtraRange(0),
			Harvester_Counted(),
			Promote_IncludeSpawns(false),
			ImmuneToCrit(false),
			MultiMindControl_ReleaseVictim(false),

			Shield_Strength(0),
			Shield_Armor(0),
			Shield_Respawn(0.0),
			Shield_Respawn_Rate(0.0),
			Shield_SelfHealing(0.0),
			Shield_SelfHealing_Rate(0.0),
			Shield_AbsorbOverDamage(false),
			Shield_BracketDelta(0),
			Shield_IdleAnim(),
			Shield_BreakAnim(),
			Shield_RespawnAnim(),
			Shield_HitAnim(),
			WarpOut(),
			WarpIn(),
			WarpAway(),
			ChronoTrigger(),
			ChronoDistanceFactor(),
			ChronoMinimumDelay(),
			ChronoRangeMinimum(),
			ChronoDelay()
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

	class ExtContainer final : public Container<TechnoTypeExt> {
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
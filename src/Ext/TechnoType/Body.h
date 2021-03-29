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

		Valueable<bool> Deployed_RememberTarget;
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

		Valueable<int> Shield_Strength;
		Valueable<signed int> Shield_Armor;
		Valueable<double> Shield_Respawn;
		Valueable<double> Shield_RespawnDelay;
		Valueable<double> Shield_SelfHealing;
		Valueable<double> Shield_SelfHealingDelay;
		Valueable<bool> Shield_AbsorbOverDamage;
		Valueable<int> Shield_BracketDelta;
		Nullable<AnimTypeClass*> Shield_Image;
		Nullable<AnimTypeClass*> Shield_BreakImage;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject),
			Deployed_RememberTarget(false),
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

			Shield_Strength(0),
			Shield_Armor(0),
			Shield_Respawn(0.0),
			Shield_RespawnDelay(0.0),
			Shield_SelfHealing(0.0),
			Shield_SelfHealingDelay(0.0),
			Shield_AbsorbOverDamage(false),
			Shield_BracketDelta(0),
			Shield_Image(),
			Shield_BreakImage()
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
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

	static void TransferMindControl(TechnoClass* From, TechnoClass* To);
	static void ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor = 1.0);

	static void ApplyMindControlRangeLimit(TechnoClass* pThis);
	static void ApplyBuildingDeployerTargeting(TechnoClass* pThis);
	static void ApplyInterceptor(TechnoClass* pThis);
	static void ApplyPowered_KillSpawns(TechnoClass* pThis);
	static void ApplySpawn_LimitRange(TechnoClass* pThis);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
};
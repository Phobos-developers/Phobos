#pragma once
#include <AnimClass.h>
#include <ParticleSystemClass.h>

#include <Ext/AnimType/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class AnimExt
{
public:
	using base_type = AnimClass;

	static constexpr DWORD Canary = 0xAAAAAAAA;
	static constexpr size_t ExtPointerOffset = 0xD0;
	static constexpr bool ShouldConsiderInvalidatePointer = true;

	class ExtData final : public Extension<AnimClass>
	{
	public:
		DirType DeathUnitFacing;
		DirStruct DeathUnitTurretFacing;
		bool FromDeathUnit;
		bool DeathUnitHasTurret;
		TechnoClass* Invoker;
		HouseClass* InvokerHouse;
		ParticleSystemClass* AttachedSystem;
		BuildingClass* ParentBuilding; // Only set on building anims, used for tinting the anims etc. especially when not on same cell as building

		ExtData(AnimClass* OwnerObject) : Extension<AnimClass>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, Invoker {}
			, InvokerHouse {}
			, AttachedSystem {}
			, ParentBuilding {}
		{ }

		void SetInvoker(TechnoClass* pInvoker);
		void SetInvoker(TechnoClass* pInvoker, HouseClass* pInvokerHouse);
		void CreateAttachedSystem();
		void DeleteAttachedSystem();

		virtual ~ExtData()
		{
			this->DeleteAttachedSystem();
		}

		virtual void InvalidatePointer(void* const ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(this->Invoker, ptr);
			AnnounceInvalidPointer(this->InvokerHouse, ptr);
			AnnounceInvalidPointer(this->AttachedSystem, ptr);
			AnnounceInvalidPointer(this->ParentBuilding, ptr);
		}

		virtual void InitializeConstants() override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();

			switch (abs)
			{
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
			case AbstractType::ParticleSystem:
			case AbstractType::House:
				return false;
			}

			return true;
		}
	};

	static ExtContainer ExtMap;

	static bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true, bool defaultToInvokerOwner = false);
	static HouseClass* GetOwnerHouse(AnimClass* pAnim, HouseClass* pDefaultOwner = nullptr);

	static void VeinAttackAI(AnimClass* pAnim);

	static void HandleDebrisImpact(AnimTypeClass* pExpireAnim, AnimTypeClass* pWakeAnim, Iterator<AnimTypeClass*> splashAnims, HouseClass* pOwner, WarheadTypeClass* pWarhead, int nDamage,
	CellClass* pCell, CoordStruct nLocation, bool heightFlag, bool isMeteor, bool warheadDetonate, bool explodeOnWater, bool splashAnimsPickRandom);
};

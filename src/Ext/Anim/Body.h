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

	class ExtData final : public Extension<AnimClass>
	{
	public:
		short DeathUnitFacing;
		DirStruct DeathUnitTurretFacing;
		bool FromDeathUnit;
		bool DeathUnitHasTurret;
		TechnoClass* Invoker;
		HouseClass* InvokerHouse;
		ParticleSystemClass* AttachedSystem;

		ExtData(AnimClass* OwnerObject) : Extension<AnimClass>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, Invoker {}
			, InvokerHouse {}
			, AttachedSystem {}
		{ }

		void SetInvoker(TechnoClass* pInvoker);
		void CreateAttachedSystem(ParticleSystemTypeClass* pSystemType);
		void DeleteAttachedSystem();

		virtual ~ExtData()
		{
			DeleteAttachedSystem();
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(Invoker, ptr);
			AnnounceInvalidPointer(InvokerHouse, ptr);
			AnnounceInvalidPointer(AttachedSystem, ptr);
		}

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
			case AbstractType::Aircraft:
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::ParticleSystem:
			case AbstractType::House:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true);
	static HouseClass* GetOwnerHouse(AnimClass* pAnim, HouseClass* pDefaultOwner = nullptr);

	static void HandleDebrisImpact(AnimTypeClass* pExpireAnim, AnimTypeClass* pWakeAnim, Iterator<AnimTypeClass*> splashAnims, HouseClass* pOwner, WarheadTypeClass* pWarhead, int nDamage,
	CellClass* pCell, CoordStruct nLocation, bool heightFlag, bool isMeteor, bool warheadDetonate, bool explodeOnWater, bool splashAnimsPickRandom);
};

#pragma once
#include <AnimClass.h>

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

		ExtData(AnimClass* OwnerObject) : Extension<AnimClass>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, Invoker {}
			, InvokerHouse {}
		{ }

		void SetInvoker(TechnoClass* pInvoker);

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(Invoker, ptr);
			AnnounceInvalidPointer(InvokerHouse, ptr);
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
			case AbstractType::House:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static const bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker , HouseClass* pVictim, bool defaultToVictimOwner = true);
};

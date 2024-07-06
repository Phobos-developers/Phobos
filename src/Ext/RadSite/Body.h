#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt
{
public:
	using base_type = RadSiteClass;

	static constexpr DWORD Canary = 0x88446622;
	static constexpr size_t ExtPointerOffset = 0x18;
	static constexpr bool ShouldConsiderInvalidatePointer = true;

	class ExtData final : public Extension<RadSiteClass>
	{
	public:
		int LastUpdateFrame;
		WeaponTypeClass* Weapon;
		RadTypeClass* Type;
		HouseClass* RadHouse;
		TechnoClass* RadInvoker;

		ExtData(RadSiteClass* OwnerObject) : Extension<RadSiteClass>(OwnerObject)
			, LastUpdateFrame { -1 }
			, RadHouse { nullptr }
			, RadInvoker { nullptr }
			, Type {}
			, Weapon { nullptr }
		{ }

		virtual ~ExtData() = default;

		bool ApplyRadiationDamage(TechnoClass* pTarget, int& damage, int distance);
		void Add(int amount);
		void SetRadLevel(int amount);
		double GetRadLevelAt(CellStruct const& cell) const;
		void CreateLight();

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(RadInvoker, ptr);
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static void CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, HouseClass* const pOwner, TechnoClass* const pInvoker);

	class ExtContainer final : public Container<RadSiteExt>
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
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;
};

#pragma once

#include <RadSiteClass.h>

#include <Utilities/Container.h>
#include <Utilities/Detach.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt
{
public:
	using base_type = RadSiteClass;

	static constexpr DWORD Canary = 0x88446622;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public AbstractExt, public Detach::Listener<TechnoClass>
	{
	public:
		// typed owner accessor
		RadSiteClass* OwnerObject() const
		{
			return static_cast<RadSiteClass*>(this->GetAttachedObject());
		}

		WeaponTypeClass* Weapon;
		RadTypeClass* Type;
		HouseClass* RadHouse;
		TechnoClass* RadInvoker;

		ExtData(RadSiteClass* OwnerObject) : AbstractExt(OwnerObject)
			, RadHouse { nullptr }
			, RadInvoker { nullptr }
			, Type {}
			, Weapon { nullptr }
		{ }

		virtual ~ExtData() = default;

		bool ApplyRadiationDamage(TechnoClass* pTarget, int& damage);
		void Add(int amount);
		void SetRadLevel(int amount);
		// double GetRadLevelAt(CellStruct const& cell) const;
		void CreateLight();

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void Initialize() override;

		virtual void OnDetach(TechnoClass* pTarget, bool removed) override
		{
			if (removed)
				AnnounceInvalidPointer(this->RadInvoker, pTarget);
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static void CreateInstance(CellStruct location, int spread, int radLevel, WeaponTypeExt::ExtData* pWeaponExt, HouseClass* const pOwner, TechnoClass* const pInvoker);

	class ExtContainer final : public Container<RadSiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;
};

// top-level name for the RadSiteExt extension
using RadSiteClassExtension = RadSiteExt::ExtData;

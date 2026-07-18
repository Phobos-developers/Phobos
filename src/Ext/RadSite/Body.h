#pragma once

#include <RadSiteClass.h>

#include <Utilities/Container.h>
#include <Utilities/Detach.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt final : public AbstractExt, public Detach::Listener<TechnoClass>
{
public:
	using base_type = RadSiteClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = RadSiteExt;

	static constexpr DWORD Canary = 0x88446622;

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

	RadSiteExt(RadSiteClass* OwnerObject) : AbstractExt(OwnerObject)
		, RadHouse { nullptr }
		, RadInvoker { nullptr }
		, Type {}
		, Weapon { nullptr }
	{ }

	virtual ~RadSiteExt() = default;

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

public:

	static void CreateInstance(CellStruct location, int spread, int radLevel, WeaponTypeExt* pWeaponExt, HouseClass* const pOwner, TechnoClass* const pInvoker);

public:
	class ExtContainer final : public Container<RadSiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static RadSiteExt* Fetch(const RadSiteClass* pThis)
	{
		return AbstractExt::Fetch<RadSiteExt>(pThis);
	}

	static RadSiteExt* TryFetch(const RadSiteClass* pThis)
	{
		return AbstractExt::TryFetch<RadSiteExt>(pThis);
	}
};


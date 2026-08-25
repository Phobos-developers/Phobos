#pragma once

#include <HouseTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/EVATypeClass.h>

class HouseTypeExt final : public AbstractTypeExt
{
public:
	using base_type = HouseTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = HouseTypeExt;

	static constexpr DWORD Canary = 0xAFFEAFFE;

public:
	// typed owner accessor
	HouseTypeClass* OwnerObject() const
	{
		return static_cast<HouseTypeClass*>(this->GetAttachedObject());
	}

	Valueable<bool> BattlePoints;
	Valueable<bool> BattlePoints_CanUseStandardPoints;

	EVAType EVATag;

	HouseTypeExt(HouseTypeClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, BattlePoints { false }
		, BattlePoints_CanUseStandardPoints { false }
		, EVATag { -2 }
	{ }

	virtual ~HouseTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void Initialize() override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static ExtContainer ExtMap;

	static HouseTypeExt* Fetch(const HouseTypeClass* pThis)
	{
		return AbstractExt::Fetch<HouseTypeExt>(pThis);
	}

	static HouseTypeExt* TryFetch(const HouseTypeClass* pThis)
	{
		return AbstractExt::TryFetch<HouseTypeExt>(pThis);
	}
};

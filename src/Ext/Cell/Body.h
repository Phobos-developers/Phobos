#pragma once
#include <CellClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class CellExt final : public AbstractExt
{
public:
	using base_type = CellClass;
	using ExtData = CellExt;

	static constexpr DWORD Canary = 0x13371337;
	static constexpr size_t ExtPointerOffset = 0x18;

	struct RadLevel
	{
		RadSiteClass* Rad { nullptr };
		int Level { 0 };

		RadLevel() = default;
		RadLevel(RadSiteClass* pRad, int level) : Rad(pRad), Level(level)
		{ }

		bool Load(PhobosStreamReader& stm, bool registerForChange);
		bool Save(PhobosStreamWriter& stm) const;

	private:
		template <typename T>
		bool Serialize(T& stm);
	};

public:
	// typed owner accessor
	CellClass* OwnerObject() const
	{
		return static_cast<CellClass*>(this->GetAttachedObject());
	}

	std::vector<RadSiteClass*> RadSites {};
	std::vector<RadLevel> RadLevels { };
	int InfantryCount{ 0 };

	CellExt(CellClass* OwnerObject) : AbstractExt(OwnerObject)
	{ }

	virtual ~CellExt() = default;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static CellExt* Fetch(const CellClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static CellExt* TryFetch(const CellClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
};


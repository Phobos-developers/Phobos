#pragma once
#include <CellClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

class CellExt
{
public:
	using base_type = CellClass;

	static constexpr DWORD Canary = 0x13371337;
	static constexpr size_t ExtPointerOffset = 0x144;

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

	class ExtData final : public Extension<CellClass>
	{
	public:
		std::vector<RadSiteClass*> RadSites {};
		std::vector<RadLevel> RadLevels { };
		UnitClass* IncomingUnit;
		UnitClass* IncomingUnitAlt;

		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			if (ptr == static_cast<void*>(this->IncomingUnit))
			{
				this->OwnerObject()->OccupationFlags &= ~0x20;
				this->IncomingUnit = nullptr;
			}

			if (ptr == static_cast<void*>(this->IncomingUnitAlt))
			{
				this->OwnerObject()->AltOccupationFlags &= ~0x20;
				this->IncomingUnitAlt = nullptr;
			}
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override;
	};

	static ExtContainer ExtMap;
};

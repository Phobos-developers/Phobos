#pragma once

#include <CellClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

class CellExt
{
public:
	using base_type = CellClass;

	static constexpr DWORD Canary = 0x13371337;
	// static constexpr size_t ExtPointerOffset = 0x144;

	class ExtData final : public Extension<CellClass>
	{
	public:
		UnitClass* IncomingUnit;
		UnitClass* IncomingUnitAlt;

		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject)
			, IncomingUnit()
			, IncomingUnitAlt()
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
		virtual void Initialize() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

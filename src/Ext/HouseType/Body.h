#pragma once

#include <HouseTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/EVATypeClass.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;

	static constexpr DWORD Canary = 0xAFFEAFFE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public AbstractTypeClassExtension
	{
	public:
		// typed owner accessor
		HouseTypeClass* OwnerObject() const
		{
			return static_cast<HouseTypeClass*>(this->GetAttachedObject());
		}

		EVAType EVATag;

		ExtData(HouseTypeClass* OwnerObject) : AbstractTypeClassExtension(OwnerObject)
			, EVATag { -2 }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static ExtContainer ExtMap;
};

// top-level name for the HouseTypeExt extension
using HouseTypeClassExtension = HouseTypeExt::ExtData;

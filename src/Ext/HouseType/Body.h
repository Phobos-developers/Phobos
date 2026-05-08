#pragma once

#include <HouseTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/EVATypeClass.h>

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;

	static constexpr DWORD Canary = 0xAFFEAFFE;
	static constexpr size_t ExtPointerOffset = 0x1AC;

	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		EVAType EVATag;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
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

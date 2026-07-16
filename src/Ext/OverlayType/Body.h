#pragma once
#include <OverlayTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class OverlayTypeExt
{
public:
	using base_type = OverlayTypeClass;

	static constexpr DWORD Canary = 0xADF48498;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public ObjectTypeClassExtension
	{
	public:
		// typed owner accessor
		OverlayTypeClass* OwnerObject() const
		{
			return static_cast<OverlayTypeClass*>(this->GetAttachedObject());
		}

		Valueable<int> ZAdjust;
		PhobosFixedString<32u> PaletteFile;
		DynamicVectorClass<ColorScheme*>* Palette; // Intentionally not serialized - rebuilt from the palette file on load.

		ExtData(OverlayTypeClass* OwnerObject) : ObjectTypeClassExtension(OwnerObject)
			, ZAdjust { 0 }
			, PaletteFile {}
			, Palette {}
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<OverlayTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

// top-level name for the OverlayTypeExt extension
using OverlayTypeClassExtension = OverlayTypeExt::ExtData;

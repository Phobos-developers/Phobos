#pragma once

#include <map>

#include <IsometricTileTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/Constructs.h>
#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>

class IsometricTileTypeExt
{
public:
	using base_type = IsometricTileTypeClass;
	static constexpr DWORD Canary = 0x91577125;

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:
		Valueable<int> Tileset;
		PhobosFixedString<32U> PaletteName;

		ExtData(IsometricTileTypeClass* OwnerObject);

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& stm) override;
		virtual void SaveToStream(PhobosStreamWriter& stm) override;

	private:
		template <typename T>
		void Serialize(T& stm);
	};

	class ExtContainer final : public Container<IsometricTileTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static int CurrentTileset;
	static std::map<std::string, std::vector<LightConvertClass*>> LightConvertEntities;
	static bool InRender;

	static LightConvertClass* GetLightConvert(const char* paletteName, int r, int g, int b, const bool isDefault);

	static void Clear();
};

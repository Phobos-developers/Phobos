#pragma once
#include <IsometricTileTypeClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

#include <unordered_map>
#include <string>

class IsometricTileTypeExt
{
public:
	using base_type = IsometricTileTypeClass;

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:
		Valueable<int> TileSetNumber;
		PhobosFixedString<0x20> CustomPalette;
		BytePalette* Palette;
		LightConvertClass* LightConvert;

		ExtData(IsometricTileTypeClass* OwnerObject) : Extension<IsometricTileTypeClass>(OwnerObject),
			TileSetNumber(-1),
			CustomPalette(""),
			Palette(nullptr),
			LightConvert(nullptr)
		{ }

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void GetSectionName(char* buffer);

	private:
		template <typename T>
		void Serialize(T & Stm);
	};

	class ExtContainer final : public Container<IsometricTileTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static std::unordered_map<PhobosFixedString<0x20>, BytePalette*, std::hash<PhobosFixedString<0x20>>, PhobosFixedString<0x20>::CaseInsensitiveCompare> Palettes;
};

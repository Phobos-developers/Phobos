#pragma once
#include <SideClass.h>

#include <Helpers/Macro.h>
#include "../_Container.hpp"
#include "../../Utilities/TemplateDef.h"

class SideExt
{
public:
	using base_type = SideClass;

	class ExtData final : public Extension<SideClass>
	{
	public:
		Valueable<int> ArrayIndex;
		Valueable<bool> Sidebar_GDIPositions;
		Valueable<int> IngameScore_WinTheme;
		Valueable<int> IngameScore_LoseTheme;

		ExtData(SideClass* OwnerObject) : Extension<SideClass>(OwnerObject),
			ArrayIndex(-1),
			Sidebar_GDIPositions(false),
			IngameScore_WinTheme(-2),
			IngameScore_LoseTheme(-2)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SideExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool isNODSidebar();
};

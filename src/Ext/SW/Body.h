#pragma once
#include <SuperClass.h>

#include <Helpers/Enumerators.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Phobos.h>

class SuperExt
{
public:
	using base_type = SuperClass;

	static constexpr DWORD Canary = 0x37373735;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<SuperClass>
	{
	public:
		bool TimerRestarted;

		ExtData(SuperClass* OwnerObject) : Extension<SuperClass>(OwnerObject)
			, TimerRestarted { false }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SuperExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		/*virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();

			switch (abs)
			{
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return false;
			}

			return true;
		}*/
	};

	static ExtContainer ExtMap;

};

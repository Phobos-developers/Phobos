#pragma once
#include <ParticleSystemTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

class ParticleSystemTypeExt
{
public:
	using base_type = ParticleSystemTypeClass;

	static constexpr DWORD Canary = 0xF9984EFE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<ParticleSystemTypeClass>
	{
	public:
		Valueable<bool> AdjustTargetCoordsOnRotation;

		ExtData(ParticleSystemTypeClass* OwnerObject) : Extension<ParticleSystemTypeClass>(OwnerObject)
			, AdjustTargetCoordsOnRotation { true }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleSystemTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

#pragma once
#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>

class Matrix3D;

class ParticleTypeExt
{
public:
	using base_type = ParticleTypeClass;

	class ExtData final : public Extension<ParticleTypeClass>
	{
	public:
		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;

		ExtData(ParticleTypeClass* OwnerObject) : Extension<ParticleTypeClass>(OwnerObject),
			LaserTrail_Types()
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

	class ExtContainer final : public Container<ParticleTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

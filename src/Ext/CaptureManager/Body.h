#pragma once
#include <CaptureManagerClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class CaptureExt
{
public:
	using base_type = CaptureManagerClass;

	class ExtData final : public Extension<base_type>
	{
	public:


		TypeList<int> OverloadCount;
		TypeList<int> OverloadFrames;
		TypeList<int> OverloadDamage;
		int OverloadDeathSound;

		ExtData(CaptureManagerClass* OwnerObject) : Extension<base_type>(OwnerObject)
			, OverloadCount { }
			, OverloadFrames { }
			, OverloadDamage { }
			, OverloadDeathSound { }
		{ }

		virtual ~ExtData() = default;

		virtual size_t Size() const {
			return sizeof(*this);
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved)
		{ }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

		void CleanUp()
		{
			this->OverloadCount.Clear();
			this->OverloadFrames.Clear();
			this->OverloadDamage.Clear();
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CaptureExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
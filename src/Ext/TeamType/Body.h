#pragma once
#include <TeamTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TeamTypeExt
{
public:
	using base_type = TeamTypeClass;

	static constexpr DWORD Canary = 0xABCDEF01;
	static constexpr size_t ExtPointerOffset = 0xBC;

	class ExtData final : public Extension<TeamTypeClass>
	{
	public:
		ExtData(TeamTypeClass* OwnerObject) : Extension<TeamTypeClass>(OwnerObject)
			, SetRecruitableOnLiberate { }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		// virtual void Initialize() override;

		Nullable<int> SetRecruitableOnLiberate;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

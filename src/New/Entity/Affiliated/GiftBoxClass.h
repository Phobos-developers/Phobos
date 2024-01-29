#pragma once

/*
#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
*/

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>
#include <New/Type/Affiliated/GiftBoxTypeClass.h>

class GiftBoxClass
{
public:
	TechnoClass* Techno { nullptr };
	bool IsEnabled { false };
	bool IsTechnoChange { false };
	bool IsOpen { false };
	int Delay { 0 };
	char TechnoID[0x18];

	GiftBoxClass() :
		Techno { nullptr },
		IsEnabled { false },
		Delay { 0 }
	{
	}

	GiftBoxClass(TechnoClass* pTechno) :
		Techno { pTechno },
		IsEnabled { false },
		Delay { 0 }
	{
		strcpy_s(this->TechnoID, this->Techno->get_ID());
	}

	~GiftBoxClass() = default;

	const void AI();
	bool Open();
	bool CheckDelay();
	void Reset(int delay);
	const bool CreateType(int nAt, GiftBoxTypeClass* nGbox, CoordStruct nCoord, CoordStruct nDestCoord);
	const bool OpenDisallowed();

	static CoordStruct GetRandomCoordsNear(GiftBoxTypeClass* nGiftBox, CoordStruct nCoord);
	static void SyncToAnotherTechno(TechnoClass* pFrom, TechnoClass* pTo);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

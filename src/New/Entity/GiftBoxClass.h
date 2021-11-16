#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

// Name : GiftBox
// Original Author : ChrisLV-CN
// https://github.com/ChrisLv-CN/PatcherExtension/blob/main/MyExtension/GiftBox.cs
// Port : Otamaa

struct GiftBoxData
{
public:

	ValueableVector<TechnoTypeClass*> TechnoList;
	ValueableVector<int> Count;
	Valueable<bool> Remove;
	Valueable<bool> Destroy;
	Valueable<int> Delay;
	Valueable<Point2D> DelayMinMax;
	Valueable<int> RandomRange;
	Valueable<bool> EmptyCell;
	Valueable<bool> RandomType;

	const void Read(INI_EX& parser, const char* pSection)
	{
		if (!pSection)
			return;

		TechnoList.Read(parser, pSection, "GiftBox.Types");
		Count.Read(parser, pSection, "GiftBox.Nums");
		Remove.Read(parser, pSection, "GiftBox.Remove");
		Destroy.Read(parser, pSection, "GiftBox.Destroy");
		Delay.Read(parser, pSection, "GiftBox.Delay");
		DelayMinMax.Read(parser, pSection, "GiftBox.RandomDelay");
		RandomRange.Read(parser, pSection, "GiftBox.CellRandomRange");
		EmptyCell.Read(parser, pSection, "GiftBox.EmptyCell");
		RandomType.Read(parser, pSection, "GiftBox.RandomType");

	}

	operator bool() const
	{
		return !TechnoList.empty() && !Count.empty();
	}

	GiftBoxData() noexcept :
		TechnoList(),
		Count(),
		Remove(true),
		Destroy(false),
		Delay(0),
		//min(X),max(Y)
		DelayMinMax({ 0,0 }),
		RandomRange(false),
		EmptyCell(false),
		RandomType(true)
	{}

	explicit GiftBoxData(noinit_t) noexcept
	{ }

};

class GiftBoxClass
{
public:

	GiftBoxClass() :
		Techno{nullptr},
		IsEnabled{ false },
		Delay{ 0 }
	{}

	GiftBoxClass(TechnoClass* pTechno) :
		Techno{ pTechno },
		IsEnabled{ false },
		Delay{ 0 }
	{
		strcpy_s(this->TechnoID, this->Techno->get_ID());
	}

	~GiftBoxClass() = default;

	bool Open()
	{
		return IsOpen ? false : CheckDelay();
	}

	bool CheckDelay()
	{
		//dont execute everytime if not enabled
		if (IsEnabled)
		{
			if (--Delay <= 0)
			{
				IsOpen = true;
				return true;
			}
		}

		return false;
	}

	void Reset(int delay)
	{
		Delay = delay;
		IsOpen = false;
		IsTechnoChange = false;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<GiftBoxClass*>(this)->Serialize(Stm);
	}

	const void AI();
	const bool CreateType(int nAt, GiftBoxData &nGbox,CoordStruct nCoord, CoordStruct nDestCoord);
	const bool OpenDisallowed();

	static CoordStruct GetRandomCoordsNear(GiftBoxData& nGiftBox, CoordStruct nCoord);
	static void SyncToAnotherTechno(TechnoClass* pFrom, TechnoClass* pTo);

	TechnoClass* Techno{ nullptr };
	bool IsEnabled{ false };
	bool IsTechnoChange{ false };
	bool IsOpen{ false };
	int Delay{ 0 };
	char TechnoID[0x18];

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(TechnoID)
			.Process(IsTechnoChange)
			.Process(Techno)
			.Process(IsEnabled)
			.Process(IsOpen)
			.Process(Delay)
			.Success();
	}
};


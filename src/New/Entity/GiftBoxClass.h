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

	static const void LoadFromINI(GiftBoxData& nGiftboxData, INI_EX& parser, const char* pSection)
	{
		if (!pSection)
			return;

		nGiftboxData.TechnoList.Read(parser, pSection, "GiftBox.Types");
		nGiftboxData.Count.Read(parser, pSection, "GiftBox.Nums");
		nGiftboxData.Remove.Read(parser, pSection, "GiftBox.Remove");
		nGiftboxData.Destroy.Read(parser, pSection, "GiftBox.Destroy");
		nGiftboxData.Delay.Read(parser, pSection, "GiftBox.Delay");
		nGiftboxData.DelayMinMax.Read(parser, pSection, "GiftBox.RandomDelay");
		nGiftboxData.RandomRange.Read(parser, pSection, "GiftBox.CellRandomRange");
		nGiftboxData.EmptyCell.Read(parser, pSection, "GiftBox.EmptyCell");
		nGiftboxData.RandomType.Read(parser, pSection, "GiftBox.RandomType");

	}

	bool operator==(const GiftBoxData& that) const
	{
		return false;
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
		IsEnabled{ false },
		Delay{ 0 }
	{}

	GiftBoxClass(GiftBoxData& nGData) :
		IsEnabled{ nGData },
		Delay{ nGData.DelayMinMax.Get().Y == 0 ? abs(nGData.Delay.Get()) : abs(ScenarioClass::Instance->Random.RandomRanged(nGData.DelayMinMax.Get().X, nGData.DelayMinMax.Get().Y)) }
	{}

	~GiftBoxClass() = default;

	bool Open()
	{
		return IsEnabled && (IsOpen ? false : CheckDelay());
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
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<GiftBoxClass*>(this)->Serialize(Stm);
	}

	static const bool CreateType(int nAt, GiftBoxData &nGbox, HouseClass* pOwner, CoordStruct nCoord, CoordStruct nDestCoord);
	static const void AI(TechnoClass* pTechno);
	static const void Construct(TechnoClass* pTechno);
	static const bool AllowDestroy(TechnoClass* pTechno);

	bool IsEnabled{ false };
	bool IsOpen{ false };
	int Delay{ 0 };

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->IsEnabled)
			.Process(this->IsOpen)
			.Process(this->Delay)
			.Success();
	}
};


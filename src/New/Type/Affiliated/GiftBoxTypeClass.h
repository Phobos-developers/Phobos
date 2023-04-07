#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

// Name : GiftBox
// Original Author : ChrisLV-CN
// https://github.com/ChrisLv-CN/PatcherExtension/blob/main/MyExtension/GiftBox.cs
// Port : Otamaa, Morton
class GiftBoxTypeClass
{
public:
	GiftBoxTypeClass() = default;

	GiftBoxTypeClass(TechnoTypeClass* OwnedBy);

	TechnoTypeClass* OwnerType;

	ValueableVector<TechnoTypeClass*> TechnoList;
	ValueableVector<int> Count;
	Valueable<bool> Remove;
	Valueable<bool> Destroy;
	Valueable<int> Delay;
	Valueable<Point2D> DelayMinMax;
	Valueable<int> RandomRange;
	Valueable<bool> EmptyCell;
	Valueable<bool> RandomType;

	operator bool() const
	{
		return !TechnoList.empty() && !Count.empty();
	}

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};

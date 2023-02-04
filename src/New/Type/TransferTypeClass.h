#pragma once

#include <WarheadTypeClass.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/EnumFunctions.h>

class TransferTypeClass final : public Enumerable<TransferTypeClass>
{
public:
	Valueable<bool> TargetToSource;

	Valueable<TransferTypeResource> Send_Resource;
	Valueable<double> Send_Value;
	Valueable<bool> Send_Value_IsPercent;
	Valueable<bool> Send_Value_PercentOfTotal;
	Valueable<int> Send_Value_FlatMinimum;
	Valueable<int> Send_Value_FlatMaximum;
	Valueable<bool> Send_ConsiderSourceVeterancy;

	Valueable<TransferTypeResource> Receive_Resource;
	Valueable<double> Receive_Value;
	Valueable<bool> Receive_Value_IsPercent;
	Valueable<bool> Receive_Value_PercentOfTotal;
	Valueable<bool> Receive_Value_ProportionalToSent;
	Valueable<int> Receive_Value_FlatMinimum;
	Valueable<int> Receive_Value_FlatMaximum;
	Valueable<bool> Receive_ConsiderSourceVeterancy;

	Valueable<bool> Send_RequireRealResource;
	Valueable<bool> Send_Experience_AllowDemote;
	Valueable<bool> Receive_RefuseOverflow;

	Valueable<bool> Target_ConsiderArmor;
	Nullable<WarheadTypeClass*> Target_Warhead;
	Valueable<AffectedHouse> Target_AffectedHouses;
	Valueable<bool> Target_Spread_CountUnaffected;
	Valueable<SpreadDistribution> Target_Spread_Distribution;

	Valueable<bool> Send_Text;
	Valueable<bool> Send_Text_ShowSign;
	Valueable<AffectedHouse> Send_Text_Houses;
	Valueable<ColorStruct> Send_Text_Color;
	Valueable<Point2D> Send_Text_Offset;

	Valueable<bool> Receive_Text;
	Valueable<bool> Receive_Text_ShowSign;
	Valueable<AffectedHouse> Receive_Text_Houses;
	Valueable<ColorStruct> Receive_Text_Color;
	Valueable<Point2D> Receive_Text_Offset;

	TransferTypeClass(const char* pTitle = NONE_STR) : Enumerable<TransferTypeClass>(pTitle)
		, TargetToSource { true }
		, Send_Resource { TransferTypeResource::Money }
		, Send_Value { 0.0 }
		, Send_Value_IsPercent { false }
		, Send_Value_PercentOfTotal { false }
		, Send_Value_FlatMinimum { 0 }
		, Send_Value_FlatMaximum { 0 }
		, Send_ConsiderSourceVeterancy { false }
		, Receive_Resource { TransferTypeResource::Money }
		, Receive_Value { 0.0 }
		, Receive_Value_IsPercent { false }
		, Receive_Value_PercentOfTotal { false }
		, Receive_Value_ProportionalToSent { false }
		, Receive_Value_FlatMinimum { 0 }
		, Receive_Value_FlatMaximum { 0 }
		, Receive_ConsiderSourceVeterancy { false }
		, Send_RequireRealResource { true }
		, Send_Experience_AllowDemote { true }
		, Receive_RefuseOverflow { false }
		, Target_ConsiderArmor { false }
		, Target_Warhead {}
		, Target_AffectedHouses { AffectedHouse::All }
		, Target_Spread_Distribution { SpreadDistribution::NoDecrease }
		, Target_Spread_CountUnaffected { false }
		, Send_Text { false }
		, Send_Text_ShowSign { false }
		, Send_Text_Houses { AffectedHouse::All }
		, Send_Text_Color { { 0, 0, 0 } }
		, Send_Text_Offset { { 0, 0 } }
		, Receive_Text { false }
		, Receive_Text_ShowSign { false }
		, Receive_Text_Houses { AffectedHouse::All }
		, Receive_Text_Color { { 0, 0, 0 } }
		, Receive_Text_Offset { { 0, 0 } }
	{ }

	virtual ~TransferTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

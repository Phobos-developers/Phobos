#pragma once

#include <WarheadTypeClass.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/EnumFunctions.h>

class TransferTypeClass final: public Enumerable<TransferTypeClass>
{
public:
	Valueable<bool> TargetToSource;

	Valueable<TransferTypeResource> Send_Resource;
	Valueable<double> Send_Value;
	Valueable<TechnoValueType> Send_Value_Type;
	Valueable<Point2D> Send_Value_FlatLimits;
	Valueable<Vector2D<double>> Send_Value_SourceVeterancyMultiplier;

	Valueable<bool> Send_PreventUnderflow;
	Valueable<bool> Send_PreventOverflow;

	Valueable<bool> Send_Text;
	Valueable<bool> Send_Text_ShowSign;
	Valueable<AffectedHouse> Send_Text_Houses;
	Valueable<ColorStruct> Send_Text_Color;
	Valueable<ColorStruct> Send_Text_Color_Positive;
	Valueable<Point2D> Send_Text_Offset;

	Valueable<TransferTypeResource> Receive_Resource;
	Valueable<double> Receive_Value;
	Valueable<TechnoValueType> Receive_Value_Type;
	Valueable<Point2D> Receive_Value_FlatLimits;
	Valueable<Vector2D<double>> Receive_Value_SourceVeterancyMultiplier;

	Valueable<Multiplier> Receive_Multiplier;
	Valueable<bool> Receive_ReturnOverflow;

	Valueable<bool> Receive_Text;
	Valueable<bool> Receive_Text_ShowSign;
	Valueable<AffectedHouse> Receive_Text_Houses;
	Valueable<ColorStruct> Receive_Text_Color;
	Valueable<ColorStruct> Receive_Text_Color_Negative;
	Valueable<Point2D> Receive_Text_Offset;

	Valueable<bool> Decrease_Experience_AllowDemote;
	Valueable<bool> Decrease_Health_AllowKill;

	Valueable<bool> Target_ConsiderArmor;
	Nullable<WarheadTypeClass*> Target_Warhead;
	Valueable<AffectedHouse> Target_AffectHouses;
	Valueable<bool> Target_Spread_IgnoreSelf;
	Valueable<SpreadDistribution> Target_Spread_Distribution;
	Valueable<bool> Target_Spread_CountUnaffected;

	TransferTypeClass(const char* pTitle = NONE_STR): Enumerable<TransferTypeClass>(pTitle)
		, TargetToSource { true }
		, Send_Resource { TransferTypeResource::Money }
		, Send_Value { 0.0 }
		, Send_Value_Type { TechnoValueType::Fixed }
		, Send_Value_FlatLimits { { 0, 0 } }
		, Send_Value_SourceVeterancyMultiplier { { 1.0, 1.0 } }
		, Send_PreventUnderflow { false }
		, Send_PreventOverflow { false }
		, Send_Text { false }
		, Send_Text_ShowSign { false }
		, Send_Text_Houses { AffectedHouse::All }
		, Send_Text_Color { { 0, 0, 0 } }
		, Send_Text_Color_Positive { { 0, 0, 0 } }
		, Send_Text_Offset { { 0, 0 } }
		, Receive_Resource { TransferTypeResource::Money }
		, Receive_Value { 0.0 }
		, Receive_Value_Type { TechnoValueType::Fixed }
		, Receive_Value_FlatLimits { { 0, 0 } }
		, Receive_Value_SourceVeterancyMultiplier { { 1.0, 1.0 } }
		, Receive_Multiplier { Multiplier::None }
		, Receive_ReturnOverflow { false }
		, Receive_Text { false }
		, Receive_Text_ShowSign { false }
		, Receive_Text_Houses { AffectedHouse::All }
		, Receive_Text_Color { { 0, 0, 0 } }
		, Receive_Text_Color_Negative { { 0, 0, 0 } }
		, Receive_Text_Offset { { 0, 0 } }
		, Decrease_Experience_AllowDemote { false }
		, Decrease_Health_AllowKill { false }
		, Target_ConsiderArmor { false }
		, Target_Warhead {}
		, Target_AffectHouses { AffectedHouse::All }
		, Target_Spread_CountUnaffected { false }
		, Target_Spread_Distribution { SpreadDistribution::None }
		, Target_Spread_IgnoreSelf { false }
	{ }

	virtual ~TransferTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

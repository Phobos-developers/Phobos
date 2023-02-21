#pragma once

#include <WarheadTypeClass.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/EnumFunctions.h>

class TransferTypeClass final: public Enumerable<TransferTypeClass>
{
public:
	Valueable<TransferDirection> Direction;

	Valueable<bool> Target_ConsiderArmor;
	Nullable<WarheadTypeClass*> Target_VersusWarhead;
	Valueable<AffectedHouse> Target_AffectHouses;
	Valueable<bool> Target_Spread_IgnoreSelf;

	Nullable<WarheadTypeClass*> Extra_Warhead;
	Valueable<bool> Extra_ConsiderArmor;
	Valueable<AffectedHouse> Extra_AffectHouses;
	Valueable<bool> Extra_Spread_EpicenterIsSource;
	Valueable<bool> Extra_Spread_IgnoreEpicenter;

	Valueable<Vector2D<double>> VeterancyMultiplier_SourceOverSender;
	Valueable<Vector2D<double>> VeterancyMultiplier_SourceOverReceiver;
	Valueable<Vector2D<double>> VeterancyMultiplier_TargetOverTarget;
	Valueable<Vector2D<double>> VeterancyMultiplier_ExtraOverExtra;

	Valueable<TransferResource> Send_Resource;
	Valueable<double> Send_Value;
	Valueable<TechnoValueType> Send_Value_Type;
	Valueable<Point2D> Send_Value_FlatLimits;

	Valueable<bool> Send_PreventUnderflow;
	Valueable<bool> Send_PreventOverflow;

	Valueable<TransferResource> Receive_Resource;
	Valueable<double> Receive_Value;
	Valueable<TechnoValueType> Receive_Value_Type;
	Valueable<Point2D> Receive_Value_FlatLimits;

	Valueable<TransferFactor> Receive_SentFactor;
	Valueable<bool>	Receive_SentSplit;

	Valueable<bool> Experience_PreventDemote;
	Valueable<bool> Health_PreventKill;
	Valueable<int> GatlingRate_LimitStageChange;

	Valueable<bool> Money_Display_Sender;
	Valueable<AffectedHouse> Money_Display_Sender_Houses;
	Valueable<Point2D> Money_Display_Sender_Offset;
	Valueable<bool> Money_Display_Receiver;
	Valueable<AffectedHouse> Money_Display_Receiver_Houses;
	Valueable<Point2D> Money_Display_Receiver_Offset;

	TransferTypeClass(const char* pTitle = NONE_STR): Enumerable<TransferTypeClass>(pTitle)
		, Direction { TransferDirection::SourceToTarget }
		, Target_ConsiderArmor { false }
		, Target_VersusWarhead {}
		, Target_AffectHouses { AffectedHouse::All }
		, Target_Spread_IgnoreSelf { false }
		, Extra_Warhead {}
		, Extra_ConsiderArmor { false }
		, Extra_AffectHouses { AffectedHouse::All }
		, Extra_Spread_EpicenterIsSource { false }
		, Extra_Spread_IgnoreEpicenter { false }
		, VeterancyMultiplier_SourceOverSender { { 1.0, 1.0 } }
		, VeterancyMultiplier_SourceOverReceiver { { 1.0, 1.0 } }
		, VeterancyMultiplier_TargetOverTarget { { 1.0, 1.0 } }
		, VeterancyMultiplier_ExtraOverExtra { { 1.0, 1.0 } }
		, Send_Resource { TransferResource::Health }
		, Send_Value { 0.0 }
		, Send_Value_Type { TechnoValueType::Fixed }
		, Send_Value_FlatLimits { { 0, 0 } }
		, Send_PreventUnderflow { false }
		, Send_PreventOverflow { false }
		, Receive_Resource { TransferResource::Health }
		, Receive_Value { 0.0 }
		, Receive_Value_Type { TechnoValueType::Fixed }
		, Receive_Value_FlatLimits { { 0, 0 } }
		, Receive_SentFactor { TransferFactor::None }
		, Receive_SentSplit { false }
		, Experience_PreventDemote { false }
		, Health_PreventKill { false }
		, GatlingRate_LimitStageChange { -1 }
		, Money_Display_Sender { false }
		, Money_Display_Sender_Houses { AffectedHouse::All }
		, Money_Display_Sender_Offset { { 0, 0 } }
		, Money_Display_Receiver { false }
		, Money_Display_Receiver_Houses { AffectedHouse::All }
		, Money_Display_Receiver_Offset { { 0, 0 } }
	{ }

	virtual ~TransferTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

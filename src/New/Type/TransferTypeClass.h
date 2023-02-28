#pragma once

#include <WarheadTypeClass.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/EnumFunctions.h>

class TransferTypeClass final: public Enumerable<TransferTypeClass>
{
public:
	Valueable<TransferDirection> Direction;

	Nullable<WarheadTypeClass*> Extra_Warhead;
	Valueable<bool> Extra_EpicenterIsSource;

	Valueable<Vector3D<double>> VeterancyMultiplier_SourceOverSender;
	Valueable<Vector3D<double>> VeterancyMultiplier_SourceOverReceiver;
	Valueable<Vector3D<double>> VeterancyMultiplier_TargetOverTarget;
	Valueable<Vector3D<double>> VeterancyMultiplier_ExtraOverExtra;

	Valueable<TransferResource> Send_Resource;
	Valueable<float> Send_Value;
	Valueable<TechnoValueType> Send_Type;
	Valueable<float> Send_Overlimit;
	Valueable<Point2D> Send_FlatLimits;

	Valueable<TransferResource> Receive_Resource;
	Valueable<float> Receive_Value;
	Valueable<TechnoValueType> Receive_Type;
	Valueable<float> Receive_Overlimit;
	Valueable<Point2D> Receive_FlatLimits;

	Valueable<TransferFactor> Receive_SentFactor;
	Valueable<bool>	Receive_Split;

	Valueable<bool> Experience_PreventDemote;
	Valueable<bool> Health_PreventKill;
	Valueable<int> GatlingRate_LimitStageChange;

	Valueable<AffectedHouse> Money_Display_Sender_Houses;
	Valueable<Point2D> Money_Display_Sender_Offset;
	Valueable<AffectedHouse> Money_Display_Receiver_Houses;
	Valueable<Point2D> Money_Display_Receiver_Offset;

	TransferTypeClass(const char* pTitle = NONE_STR): Enumerable<TransferTypeClass>(pTitle)
		Direction { TransferDirection::SourceToTarget }
		Extra_Warhead {}
		Extra_EpicenterIsSource { false }
		VeterancyMultiplier_SourceOverSender { { 1.0, 1.0 } }
		VeterancyMultiplier_SourceOverReceiver { { 1.0, 1.0 } }
		VeterancyMultiplier_TargetOverTarget { { 1.0, 1.0 } }
		VeterancyMultiplier_ExtraOverExtra { { 1.0, 1.0 } }
		Send_Resource { TransferResource::Health }
		Send_Value { 0.0 }
		Send_Type { TechnoValueType::Fixed }
		Send_FlatLimits { { 0, 0 } }
		Send_Overflow { false }
		Receive_Resource { TransferResource::Health }
		Receive_Value { 0.0 }
		Receive_Type { TechnoValueType::Fixed }
		Receive_FlatLimits { { 0, 0 } }
		Receive_Overflow { false }
		Receive_SentFactor { TransferFactor::None }
		Receive_Split { false }
		Experience_PreventDemote { false }
		Health_PreventKill { false }
		GatlingRate_LimitStageChange { -1 }
		Money_Display_Sender_Houses { AffectedHouse::None }
		Money_Display_Sender_Offset { { 0, 0 } }
		Money_Display_Receiver_Houses { AffectedHouse::None }
		Money_Display_Receiver_Offset { { 0, 0 } }
	{ }

	virtual ~TransferTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

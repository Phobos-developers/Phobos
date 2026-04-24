#pragma once

#include <LocomotionClass.h>
#include <FootClass.h>
#include <MapClass.h>
#include <Math.h>
#include <ScenarioClass.h>
#include <AircraftTrackerClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Scenario/Body.h>

#include <memory>

class __declspec(uuid("8A3F6C0F-5E2C-4A6F-9C6D-6E9B3F4A1B2E"))
	ShiftLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject) override
	{
		HRESULT hr = this->LocomotionClass::QueryInterface(iid, ppvObject);

		if (hr != E_NOINTERFACE)
			return hr;

		if (iid == __uuidof(IPiggyback))
		{
			*ppvObject = static_cast<IPiggyback*>(this);
		}

		if (*ppvObject)
		{
			this->AddRef();

			return S_OK;
		}

		return E_NOINTERFACE;
	}
	virtual ULONG __stdcall AddRef() override { return this->LocomotionClass::AddRef(); }
	virtual ULONG __stdcall Release() override { return this->LocomotionClass::Release(); }

	// IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override
	{
		if (pClassID == nullptr)
			return E_POINTER;

		*pClassID = __uuidof(this);

		return S_OK;
	}

	// IPersistStream
//	virtual HRESULT __stdcall IsDirty() override { return !this->Dirty; }
	virtual HRESULT __stdcall Load(IStream* pStm) override
	{
		HRESULT hr = this->LocomotionClass::Load(pStm);

		if (FAILED(hr))
			return hr;

		if (this)
		{
			this->Piggybacker.Detach();
			new (this) ShiftLocomotionClass(noinit_t());
		}

		bool piggybackerPresent = false;
		hr = pStm->Read(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

		if (!piggybackerPresent)
			return hr;

		hr = OleLoadFromStream(pStm, __uuidof(ILocomotion), reinterpret_cast<LPVOID*>(&this->Piggybacker));

		return hr;
	}
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override
	{
		HRESULT hr = this->LocomotionClass::Save(pStm, fClearDirty);

		if (FAILED(hr))
			return hr;

		bool piggybackerPresent = this->Piggybacker != nullptr;
		hr = pStm->Write(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

		if (!piggybackerPresent)
			return hr;

		IPersistStreamPtr piggyPersist(this->Piggybacker);
		hr = OleSaveToStream(piggyPersist, pStm);

		return hr;
	}

	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override;
	virtual bool __stdcall Is_Ok_To_End() override;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override;
	virtual bool __stdcall Is_Piggybacking() override;

	virtual bool __stdcall Is_Moving() override;
	virtual CoordStruct __stdcall Destination() override;
	virtual CoordStruct __stdcall Head_To_Coord() override;
	virtual bool __stdcall Process() override;
	virtual void __stdcall Move_To(CoordStruct to) override; // Not allowed
	// virtual void __stdcall Stop_Moving() override; // Not allowed
	virtual HRESULT __stdcall Link_To_Object(void* pointer) override;
	// virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override;
	// virtual bool __stdcall Is_To_Have_Shadow() override;
	// virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* pIndex) override; // TODO
	// virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* pIndex) override; // TODO
	// virtual Point2D __stdcall Draw_Point() override;
	// virtual Point2D __stdcall Shadow_Point() override;
	// virtual VisualType __stdcall Visual_Character(bool raw) override;
	// virtual int __stdcall Z_Adjust() override;
	// virtual ZGradient __stdcall Z_Gradient() override;
	// virtual void __stdcall Do_Turn(DirStruct coord) override;
	// virtual void __stdcall Unlimbo() override;
	// virtual void __stdcall Tilt_Pitch_AI() override; // TODO
	// virtual bool __stdcall Power_On() override;
	// virtual bool __stdcall Power_Off() override;
	// virtual bool __stdcall Is_Powered() override;
	// virtual bool __stdcall Is_Ion_Sensitive() override;
	// virtual bool __stdcall Push(DirStruct dir) override; // Not allowed
	// virtual bool __stdcall Shove(DirStruct dir) override;
	// virtual void __stdcall Force_Track(int track, CoordStruct coord) override; // Not allowed
	virtual Layer __stdcall In_Which_Layer() override;
	// virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override; // Not allowed
	// virtual void __stdcall Force_New_Slope(int ramp) override; // Not allowed
	// virtual bool __stdcall Is_Moving_Now() override;
	// virtual int __stdcall Apparent_Speed() override; // TODO
	// virtual int __stdcall Drawing_Code() override;
	virtual FireError __stdcall Can_Fire() override;
	// virtual int __stdcall Get_Status() override;
	// virtual void __stdcall Acquire_Hunter_Seeker_Target() override; // Not allowed
	// virtual bool __stdcall Is_Surfacing() override;
	virtual void __stdcall Mark_All_Occupation_Bits(MarkType mark) override;
	// virtual bool __stdcall Is_Moving_Here(CoordStruct to) override;
	// virtual bool __stdcall Will_Jump_Tracks() override;
	// virtual bool __stdcall Is_Really_Moving_Now() override;
	// virtual void __stdcall Stop_Movement_Animation() override;
	virtual void __stdcall Limbo() override;
	// virtual void __stdcall Lock() override;
	// virtual void __stdcall Unlock() override;
	// virtual int __stdcall Get_Track_Number() override;
	// virtual int __stdcall Get_Track_Index() override;
	// virtual int __stdcall Get_Speed_Accum() override;

	virtual int Size() override { return sizeof(*this); }

public:
	inline ShiftLocomotionClass() : LocomotionClass{}
		, Elapsed{ 0 }
		, IsShifting{ false }
		, Piggybacker{ nullptr }
	{ }

	inline ShiftLocomotionClass(noinit_t) : LocomotionClass{ noinit_t() } { }

	// begin the shift using the schedule (takes ownership)
	void BeginShift(std::unique_ptr<ShiftSchedule> schedule);

	// finish current shift (internal helper)
	void FinishShift(bool normal = true);

	static bool IsAirLoco(ILocomotion* pLoco);
	static CoordStruct FindShiftDestination(FootClass* pTechno, CoordStruct idealDest, double searchRange = 5.0, bool pathReachable = false);

public:

	std::unique_ptr<ShiftSchedule> Schedule;
	int Elapsed; // elapsed frames since shift started
	bool IsShifting;
	ILocomotionPtr Piggybacker;
};

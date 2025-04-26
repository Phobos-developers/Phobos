#pragma once

#include <LocomotionClass.h>
#include <MapClass.h>

#include <Interfaces.h>

#include <comip.h>
#include <comdef.h>

class __declspec(uuid("4A582751-9839-11d1-B709-00A024DDAFD1"))
	AdvancedDriveLocomotionClass : public LocomotionClass, public IPiggyback
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
			new (this) AdvancedDriveLocomotionClass(noinit_t());
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
/*	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override
	{
		if (pcbSize == nullptr)
			return E_POINTER;

		return this->LocomotionClass::GetSizeMax(pcbSize);
	}*/
	virtual int Size() override { return sizeof(*this); }

	// ILocomotion
/*	virtual HRESULT __stdcall Link_To_Object(void* pointer) override
	{
		HRESULT hr = this->LocomotionClass::Link_To_Object(pointer);

		if (SUCCEEDED(hr))
			Debug::Log("AdvancedDriveLocomotionClass - Sucessfully linked to \"%s\"\n", Owner->get_ID());

		return hr;
	}*/
	virtual bool __stdcall Is_Moving() override
	{
		if (this->TargetCoord != CoordStruct::Empty)
			return true;

		return this->HeadToCoord != CoordStruct::Empty
			&& (this->HeadToCoord.X != this->LinkedTo->Location.X
				|| this->HeadToCoord.Y != this->LinkedTo->Location.Y);
	}
	virtual CoordStruct __stdcall Destination() override { return this->TargetCoord; } // CoordStruct*
	virtual CoordStruct __stdcall Head_To_Coord() override // CoordStruct*
	{
		if (this->HeadToCoord == CoordStruct::Empty)
			return this->LinkedTo->Location;

		return this->HeadToCoord;
	}
//	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override { return Move::OK; }
//	virtual bool __stdcall Is_To_Have_Shadow() override { return true; }
	virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* key) override { JMP_STD(0x4AFF60); } // TODO but lazy
	virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* key) override { JMP_STD(0x4B0410); } // TODO but lazy
//	virtual Point2D __stdcall Draw_Point() override { return this->LocomotionClass::Draw_Point(); } // Point2D*
//	virtual Point2D __stdcall Shadow_Point() override { return this->LocomotionClass::Shadow_Point(); } // Point2D*
//	virtual VisualType __stdcall Visual_Character(bool raw) override { return VisualType::Normal; }
	virtual int __stdcall Z_Adjust() override { return 0; }
	virtual ZGradient __stdcall Z_Gradient() override { return ZGradient::Deg90; }
	virtual bool __stdcall Process() override;
	virtual void __stdcall Move_To(CoordStruct to) override;
	virtual void __stdcall Stop_Moving() override;
	virtual void __stdcall Do_Turn(DirStruct dir) override;
	virtual void __stdcall Unlimbo() override { this->Force_New_Slope(this->LinkedTo->GetCell()->SlopeIndex); }
//	virtual void __stdcall Tilt_Pitch_AI() override {}
/*	virtual bool __stdcall Power_On() override
	{
		this->Powered = true;
		return this->Is_Powered();
	}*/
/*	virtual bool __stdcall Power_Off() override
	{
		this->Powered = false;
		return this->Is_Powered();
	}*/
//	virtual bool __stdcall Is_Powered() override { return this->Powered; }
//	virtual bool __stdcall Is_Ion_Sensitive() override { return false; }
//	virtual bool __stdcall Push(DirStruct dir) override { return false; }
//	virtual bool __stdcall Shove(DirStruct dir) override { return false; }
	virtual void __stdcall Force_Track(int track, CoordStruct coord) override;
	virtual Layer __stdcall In_Which_Layer() override { return Layer::Ground; }
//	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override {}
	virtual void __stdcall Force_New_Slope(int ramp) override
	{
		this->PreviousRamp = ramp;
		this->CurrentRamp = ramp;
		this->SlopeTimer.Start(0);
	}
	virtual bool __stdcall Is_Moving_Now() override
	{
		if (this->LinkedTo->PrimaryFacing.IsRotating())
			return true;

		return (this->TargetCoord != CoordStruct::Empty
				|| this->HeadToCoord.X != this->LinkedTo->Location.X
				|| this->HeadToCoord.Y != this->LinkedTo->Location.Y)
			&& this->HeadToCoord != CoordStruct::Empty
			&& this->LinkedTo->GetCurrentSpeed() > 0;
	}
//	virtual int __stdcall Apparent_Speed() override { return this->LinkedTo->GetCurrentSpeed(); }
//	virtual int __stdcall Drawing_Code() override { return 0; }
//	virtual FireError __stdcall Can_Fire() override { return FireError::OK; }
//	virtual int __stdcall Get_Status() override { return 0; }
//	virtual void __stdcall Acquire_Hunter_Seeker_Target() override {}
//	virtual bool __stdcall Is_Surfacing() override { return false; }
	virtual void __stdcall Mark_All_Occupation_Bits(MarkType mark) override;
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override;
	virtual bool __stdcall Will_Jump_Tracks() override;
//	virtual bool __stdcall Is_Really_Moving_Now() override { return this->Is_Moving_Now(); }
//	virtual void __stdcall Stop_Movement_Animation() override {}
//	virtual void __stdcall Limbo() override {}
	virtual void __stdcall Lock() override { this->UnLocked = false; }
	virtual void __stdcall Unlock() override { this->UnLocked = true; }
	virtual int __stdcall Get_Track_Number() override { return this->TrackNumber; }
	virtual int __stdcall Get_Track_Index() override { return this->TrackIndex; }
	virtual int __stdcall Get_Speed_Accum() override { return this->SpeedAccum; }

	//IPiggy
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override
	{
		if (!pointer)
			return E_POINTER;

		if (this->Piggybacker)
			return E_FAIL;

		this->Piggybacker = pointer;
		pointer->AddRef();

		return S_OK;
	}
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override
	{
		if (!pointer)
			return E_POINTER;

		if (!this->Piggybacker)
			return S_FALSE;

		*pointer = this->Piggybacker.Detach();

		const auto pLinkedTo = this->LinkedTo;

		if (!pLinkedTo->Deactivated && !pLinkedTo->IsUnderEMP())
			this->Power_On();
		else
			this->Power_Off();

		return S_OK;
	}
	virtual bool __stdcall Is_Ok_To_End() override
	{
		return !this->Is_Moving() && this->Piggybacker && this->UnLocked && !this->LinkedTo->IsAttackedByLocomotor;
	}
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override
	{
		if (classid == nullptr)
			return E_POINTER;

		if (this->Piggybacker)
		{
			IPersistStreamPtr piggyAsPersist(this->Piggybacker);
			return piggyAsPersist->GetClassID(classid);
		}

		if (reinterpret_cast<IPiggyback*>(this) == nullptr)
			return E_FAIL;

		IPersistStreamPtr thisAsPersist(this);

		if (thisAsPersist == nullptr)
			return E_FAIL;

		return thisAsPersist->GetClassID(classid);
	}
	virtual bool __stdcall Is_Piggybacking() override
	{
		return this->Piggybacker != nullptr;
	}

	// Constructors
	inline AdvancedDriveLocomotionClass(noinit_t) : LocomotionClass { noinit_t() } { }
	inline AdvancedDriveLocomotionClass() : LocomotionClass { }
		, CurrentRamp { 0 }
		, PreviousRamp { 0 }
		, SlopeTimer {}
		, TargetCoord { CoordStruct::Empty }
		, HeadToCoord { CoordStruct::Empty }
		, SpeedAccum { 0 }
		, MovementSpeed { 0.0 }
		, TrackNumber { -1 }
		, TrackIndex { -1 }
		, IsOnShortTrack { false }
		, IsTurretLockedDown { false }
		, IsRotating { false }
		, IsDriving { false }
		, IsRocking { false }
		, UnLocked { true }
		, IsForward { true }
		, IsShifting { false }
		, Piggybacker { nullptr }
		, ForwardTo { CoordStruct::Empty }
		, TargetFrame { 0 }
		, TargetDistance { 0 }
	{ }

	// Destructor
	inline virtual ~AdvancedDriveLocomotionClass() override = default;

	// Properties
	int CurrentRamp;
	int PreviousRamp;
	RateTimer SlopeTimer;
	CoordStruct TargetCoord;
	CoordStruct HeadToCoord;
	int SpeedAccum;
	double MovementSpeed;
	int TrackNumber;
	int TrackIndex;
	bool IsOnShortTrack;
	bool IsTurretLockedDown;
	bool IsRotating;
	bool IsDriving;
	bool IsRocking;
	bool UnLocked;
	bool IsForward;
	bool IsShifting;
	ILocomotionPtr Piggybacker;
	CoordStruct ForwardTo;
	int TargetFrame;
	int TargetDistance;

private:
	// Vanilla auxiliary function
	bool MovingProcess(bool fix); // 0x4B0F20
	bool PassableCheck(bool* pStop, bool force, bool check); // 0x4B2630
	void MarkOccupation(const CoordStruct& to, MarkType mark); // 0x4B0AD0
	CoordStruct GetTrackOffset(const Point2D& base, int& face, int z = 0); // 0x4B4780

	static CoordStruct CoordLerp(const CoordStruct& crd1, const CoordStruct& crd2, float alpha); // 0x75F540

	// Added inline auxiliary function
	template <bool check = false>
	inline void StopDriving()
	{
		if constexpr (check)
			if (this->HeadToCoord == CoordStruct::Empty)
				return;

		this->HeadToCoord = CoordStruct::Empty;
		this->IsDriving = false;
	}
	inline bool StopMotion()
	{
		const auto pLinked = this->LinkedTo;

		if (!pLinked->unknown_abstract_array_588.Count)
		{
			pLinked->SetDestination(nullptr, true);
			return false;
		}

		pLinked->AbortMotion();
		return pLinked->EnterIdleMode(false, true);
	}
	inline bool InMotion(); // Main loco motion process
	inline bool LinkCannotMove()
	{
		const auto pLinked = this->LinkedTo;

		return !pLinked->IsAlive || pLinked->InLimbo || pLinked->IsFallingDown;
	}
	inline bool TakeMovingAction(bool fix)
	{
		bool stop = false;
		this->PassableCheck(&stop, true, false);

		if (stop || !this->LinkedTo->IsAlive)
			return false;

		this->MovingProcess(true);
		const auto pLinked = this->LinkedTo;

		return pLinked && pLinked->IsAlive;
	}
	inline void UpdateOnBridge(CellClass* pNewCell, CellClass* pOldCell)
	{
		const auto pLinked = this->LinkedTo;

		if (pNewCell->Level == (pOldCell->Level - 4))
		{
			if (pNewCell->ContainsBridge())
				pLinked->OnBridge = true;
			else if (pOldCell->ContainsBridge())
				pLinked->OnBridge = false;
		}
		else if (!pNewCell->ContainsBridge())
		{
			if (pOldCell->ContainsBridge())
				pLinked->OnBridge = false;
		}
	}
	inline int UpdateSpeedAccum(int& speedAccum); // Avoid using goto
};

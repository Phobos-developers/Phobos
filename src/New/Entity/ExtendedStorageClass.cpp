#include "ExtendedStorageClass.h"

int ExtendedStorageClass::Get_Total_Value() const
{
	int total = 0;

	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		total += static_cast<int>(Tiberiums[i] * TiberiumClass::Array->GetItem(i)->Value);
	}

	return total;
}

float ExtendedStorageClass::Get_Total_Amount() const
{
	float total = 0;

	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		total += Tiberiums[i];
	}

	return total;
}

float ExtendedStorageClass::Get_Amount(int tiberium) const
{
	return Tiberiums[tiberium];
}

float ExtendedStorageClass::Increase_Amount(float amount, int tiberium)
{
	Tiberiums[tiberium] += amount;
	return Tiberiums[tiberium];
}

float ExtendedStorageClass::Decrease_Amount(float amount, int tiberium)
{
	if (amount < Tiberiums[tiberium])
		amount = Tiberiums[tiberium];

	Tiberiums[tiberium] -= amount;
	return amount;
}

int ExtendedStorageClass::First_Used_Slot() const
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		if (Tiberiums[i] > 0.0)
			return i;
	}

	return -1;
}

ExtendedStorageClass* ExtendedStorageClass::operator+(ExtendedStorageClass& that) const
{
	const auto storageClass = new ExtendedStorageClass();
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		storageClass->Tiberiums[i] = Tiberiums[i] + that.Tiberiums[i];
	}

	return storageClass;
}

ExtendedStorageClass* ExtendedStorageClass::operator+=(ExtendedStorageClass& that)
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		Tiberiums[i] += that.Tiberiums[i];
	}

	return this;
}

ExtendedStorageClass* ExtendedStorageClass::operator-(ExtendedStorageClass& that) const
{
	const auto storageClass = new ExtendedStorageClass();
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		storageClass->Tiberiums[i] = Tiberiums[i] - that.Tiberiums[i];
	}

	return storageClass;
}

ExtendedStorageClass* ExtendedStorageClass::operator-=(ExtendedStorageClass& that)
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		Tiberiums[i] -= that.Tiberiums[i];
	}

	return this;
}

#pragma region Save/Load

template <typename T>
bool ExtendedStorageClass::Serialize(T& stm)
{
	return stm
		.Process(this->Tiberiums)
		.Success();
};

bool ExtendedStorageClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool ExtendedStorageClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<ExtendedStorageClass*>(this)->Serialize(stm);
}

#pragma endregion

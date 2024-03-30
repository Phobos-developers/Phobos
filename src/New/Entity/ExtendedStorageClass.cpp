#include "ExtendedStorageClass.h"

int ExtendedStorageClass::GetTotalValue() const
{
	int total = 0;

	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		total += static_cast<int>(Tiberiums[i] * TiberiumClass::Array->GetItem(i)->Value);
	}

	return total;
}

float ExtendedStorageClass::GetTotalAmount() const
{
	float total = 0;

	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		total += Tiberiums[i];
	}

	return total;
}

float ExtendedStorageClass::GetAmount(int index) const
{
	return Tiberiums[index];
}

float ExtendedStorageClass::IncreaseAmount(float amount, int index)
{
	Tiberiums[index] += amount;
	return Tiberiums[index];
}

float ExtendedStorageClass::DecreaseAmount(float amount, int index)
{
	if (amount < Tiberiums[index])
		amount = Tiberiums[index];

	Tiberiums[index] -= amount;
	return amount;
}

int ExtendedStorageClass::FirstUsedSlot() const
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		if (Tiberiums[i] > 0.0)
			return i;
	}

	return -1;
}

ExtendedStorageClass ExtendedStorageClass::operator+(ExtendedStorageClass& that) const
{
	auto storageClass = ExtendedStorageClass();
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		storageClass.Tiberiums[i] = Tiberiums[i] + that.Tiberiums[i];
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

ExtendedStorageClass ExtendedStorageClass::operator-(ExtendedStorageClass& that) const
{
	auto storageClass = ExtendedStorageClass();
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		storageClass.Tiberiums[i] = Tiberiums[i] - that.Tiberiums[i];
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

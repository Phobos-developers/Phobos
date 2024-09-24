#include "PhobosStorageClass.h"

int PhobosStorageClass::GetTotalValue() const
{
	float total = 0;

	for (size_t i = 0; i < Tiberiums->size(); i++)
	{
		total += static_cast<int>((*Tiberiums)[i] * TiberiumClass::Array->GetItem(i)->Value);
	}

	return static_cast<int>(total);
}

float PhobosStorageClass::GetTotalAmount() const
{
	float total = 0;

	for (size_t i = 0; i < Tiberiums->size(); i++)
	{
		total += (*Tiberiums)[i];
	}

	return total;
}

float PhobosStorageClass::GetAmount(int index) const
{
	return (*Tiberiums)[index];
}

float PhobosStorageClass::IncreaseAmount(float amount, int index)
{
	(*Tiberiums)[index] += amount;
	return (*Tiberiums)[index];
}

float PhobosStorageClass::DecreaseAmount(float amount, int index)
{
	if (amount < (*Tiberiums)[index])
		amount = (*Tiberiums)[index];

	(*Tiberiums)[index] -= amount;
	return amount;
}

int PhobosStorageClass::FirstUsedSlot() const
{
	for (size_t i = 0; i < Tiberiums->size(); i++)
	{
		if ((*Tiberiums)[i] > 0.0)
			return i;
	}

	return -1;
}

PhobosStorageClass PhobosStorageClass::operator+=(PhobosStorageClass& that)
{
	for (size_t i = 0; i < Tiberiums->size(); i++)
	{
		(*Tiberiums)[i] += (*that.Tiberiums)[i];
	}

	return *this;
}

PhobosStorageClass PhobosStorageClass::operator-=(PhobosStorageClass& that)
{
	for (size_t i = 0; i < Tiberiums->size(); i++)
	{
		(*Tiberiums)[i] -= (*that.Tiberiums)[i];
	}

	return *this;
}

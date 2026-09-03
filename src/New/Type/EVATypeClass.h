#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/AresFunctions.h>
#include <Utilities/AresHelper.h>

class EVAType : public Valueable<int>
{
public:
	template <typename T>
	EVAType(T value)
	{
		this->Value = (int)std::move(value);
	}

	template <typename T>
	EVAType& operator = (T value)
	{
		this->Value = std::move(value);
		return *this;
	}

	bool Read(CCINIClass* pINI, const char* pSection, const char* pKey)
	{
		int buffer = this->Value;

		if (pINI->ReadString(pSection, pKey, "", Phobos::readBuffer))
		{
			if (AresHelper::CanUseAres)
				buffer = AresFunctions::FindEVAIndex(Phobos::readBuffer);
			else if (!strcmp(Phobos::readBuffer, "Allied"))
				buffer = 0;
			else if (!strcmp(Phobos::readBuffer, "Russian"))
				buffer = 1;
			else if (!strcmp(Phobos::readBuffer, "Yuri"))
				buffer = 2;
		}

		if (buffer >= 0 || INIClass::IsBlank(Phobos::readBuffer))
		{
			this->Value = buffer;
			return true;
		}

		return false;
	}
};

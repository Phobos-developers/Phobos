#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#include "Constructs.h"

#include <ConvertClass.h>
#include <FileSystem.h>
#include <Utilities/GeneralUtils.h>

#include "Savegame.h"
#include "Debug.h"

bool CustomPalette::LoadFromINI(
	CCINIClass* pINI, const char* pSection, const char* pKey,
	const char* pDefault)
{
	if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer))
	{
		GeneralUtils::ApplyTheaterSuffixToString(Phobos::readBuffer);

		this->Clear();

		if (auto pPal = FileSystem::AllocatePalette(Phobos::readBuffer))
		{
			this->Palette.reset(pPal);
			this->CreateConvert();
		}

		return this->Convert != nullptr;
	}
	return false;
}

bool CustomPalette::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Clear();

	bool hasPalette = false;
	auto ret = Stm.Load(this->Mode) && Stm.Load(hasPalette);

	if (ret && hasPalette)
	{
		this->Palette.reset(GameCreate<BytePalette>());
		ret = Stm.Load(*this->Palette);

		if (ret)
		{
			this->CreateConvert();
		}
	}

	return ret;
}

bool CustomPalette::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->Mode);
	Stm.Save(this->Palette != nullptr);
	if (this->Palette)
	{
		Stm.Save(*this->Palette);
	}
	return true;
}

void CustomPalette::Clear()
{
	this->Convert = nullptr;
	this->Palette = nullptr;
}

void CustomPalette::CreateConvert()
{
	ConvertClass* buffer = nullptr;
	if (this->Mode == PaletteMode::Temperate)
	{
		buffer = GameCreate<ConvertClass>(
			*this->Palette.get(), FileSystem::TEMPERAT_PAL, DSurface::Primary,
			53, false);
	}
	else
	{
		buffer = GameCreate<ConvertClass>(
			*this->Palette.get(), *this->Palette.get(), DSurface::Alternate,
			1, false);
	}
	this->Convert.reset(buffer);
}


PhobosPCXFile& PhobosPCXFile::operator = (const char* pFilename)
{
	this->filename = pFilename;
	auto& data = this->filename.data();
	_strlwr_s(data);

	this->checked = false;
	this->exists = false;

	if (this->resolve)
	{
		this->Exists();
	}

	return *this;
}

BSurface* PhobosPCXFile::GetSurface(BytePalette* pPalette) const
{
	return this->Exists() ? PCX::Instance->GetSurface(this->filename, pPalette) : nullptr;
}

bool PhobosPCXFile::Exists() const
{
	if (!this->checked)
	{
		this->checked = true;
		if (this->filename)
		{
			auto pPCX = &PCX::Instance();
			this->exists = (pPCX->GetSurface(this->filename) || pPCX->LoadFile(this->filename));
		}
	}
	return this->exists;
}

bool PhobosPCXFile::Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	char buffer[Capacity];
	if (pINI->ReadString(pSection, pKey, pDefault, buffer))
	{
		*this = buffer;

		if (this->checked && !this->exists)
		{
			Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
		}
	}
	return buffer[0] != 0;
}

bool PhobosPCXFile::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->filename = nullptr;
	if (Stm.Load(*this))
	{
		if (this->checked && this->exists)
		{
			this->checked = false;
			if (!this->Exists())
			{
				Debug::Log("PCX file '%s' was not found.\n", this->filename.data());
			}
		}
		return true;
	}
	return false;
}

bool PhobosPCXFile::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(*this);
	return true;
}

const CSFText& CSFText::operator = (const char* label)
{
	if (this->Label != label)
	{
		this->Label = label;
		this->Text = nullptr;

		if (this->Label)
		{
			this->Text = StringTable::LoadString(this->Label);
		}
	}

	return *this;
}

bool CSFText::load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Text = nullptr;
	if (Stm.Load(this->Label.data()))
	{
		if (this->Label)
		{
			this->Text = StringTable::LoadString(this->Label);
		}
		return true;
	}
	return false;
}

bool CSFText::save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->Label.data());
	return true;
}


bool TranslucencyLevel::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	int buf;
	if (parser.ReadInteger(pSection, pKey, &buf))
	{
		*this = buf;
		return true;
	}

	return false;
}

bool TranslucencyLevel::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	Stm.Load(this->value);
	return true;
}

bool TranslucencyLevel::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->value);
	return true;
}

bool TheaterSpecificSHP::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto pValue = parser.value();
		GeneralUtils::ApplyTheaterSuffixToString(pValue);

		std::string Result = pValue;
		if (!strstr(pValue, ".shp"))
			Result += ".shp";

		if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
		{
			value = pImage;
			return true;
		}
		else
		{
			Debug::Log("Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, pValue);
		}
	}
	return false;
}

bool TheaterSpecificSHP::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->value, RegisterForChange);
}

bool TheaterSpecificSHP::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->value);
}


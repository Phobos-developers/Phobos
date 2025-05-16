#include <Helpers/Macro.h>
#include <CCINIClass.h>
#include <ScenarioClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <Straws.h>
#include <Pipes.h>

struct OverlayReader
{
	struct OverlayByteReader
	{
		OverlayByteReader(CCINIClass* pINI, const char* pSection)
			: ls{ TRUE, 0x2000 }, bs{ nullptr, 0 }
		{
			pBuffer = YRMemory::Allocate(512000);
			uuLength = pINI->ReadUUBlock(pSection, pBuffer, 512000);
			if (this->IsAvailable())
			{
				bs.Buffer.Buffer = pBuffer;
				bs.Buffer.Size = uuLength;
				bs.Buffer.Allocated = false;
				ls.Get_From(bs);
			}
		}

		~OverlayByteReader()
		{
			YRMemory::Deallocate(pBuffer);
		}

		bool IsAvailable() const { return uuLength > 0; }

		unsigned char Get()
		{
			if (IsAvailable())
			{
				unsigned char ret;
				ls.Get(&ret, sizeof(ret));
				return ret;
			}
			return 0;
		}

		size_t uuLength;
		void* pBuffer;
		LCWStraw ls;
		BufferStraw bs;
	};

	size_t Get()
	{
		unsigned char ret[4];

		ret[0] = ByteReaders[0].Get();
		ret[1] = ByteReaders[1].Get();
		ret[2] = ByteReaders[2].Get();
		ret[3] = ByteReaders[3].Get();

		return ret[0] == 0xFF ? 0xFFFFFFFF : (ret[0] | (ret[1] << 8) | (ret[2] << 16) | (ret[3] << 24));
	}

	OverlayReader(CCINIClass* pINI)
		:ByteReaders{ { pINI, "OverlayPack" }, { pINI, "OverlayPack2" }, { pINI, "OverlayPack3" }, { pINI, "OverlayPack4" }, }
	{}

private:
	OverlayByteReader ByteReaders[4];
};

struct OverlayWriter
{
	struct OverlayByteWriter
	{
		OverlayByteWriter(const char* pSection, size_t nBufferLength)
			: bp { nullptr,0 }, lp { FALSE,0x2000 }, uuLength { 0 }, lpSectionName { pSection }
		{
			this->Buffer = YRMemory::Allocate(nBufferLength);
			bp.Buffer.Buffer = this->Buffer;
			bp.Buffer.Size = nBufferLength;
			bp.Buffer.Allocated = false;
			lp.Put_To(bp);
		}

		~OverlayByteWriter()
		{
			YRMemory::Deallocate(this->Buffer);
		}

		void Put(unsigned char data)
		{
			uuLength += lp.Put(&data, 1);
		}

		void PutBlock(CCINIClass* pINI)
		{
			pINI->Clear(this->lpSectionName, nullptr);
			pINI->WriteUUBlock(this->lpSectionName, this->Buffer, uuLength);
		}

		const char* lpSectionName;
		size_t uuLength;
		void* Buffer;
		BufferPipe bp;
		LCWPipe lp;
	};

	OverlayWriter(size_t nLen)
		: ByteWriters { { "OverlayPack", nLen}, { "OverlayPack2", nLen }, { "OverlayPack3", nLen }, { "OverlayPack4", nLen } }
	{ }

	void Put(int nOverlay)
	{
		unsigned char bytes[4];
		bytes[0] = (nOverlay & 0xFF);
		bytes[1] = ((nOverlay >> 8) & 0xFF);
		bytes[2] = ((nOverlay >> 16) & 0xFF);
		bytes[3] = ((nOverlay >> 24) & 0xFF);
		ByteWriters[0].Put(bytes[0]);
		ByteWriters[1].Put(bytes[1]);
		ByteWriters[2].Put(bytes[2]);
		ByteWriters[3].Put(bytes[3]);
	}

	void PutBlock(CCINIClass* pINI)
	{
		ByteWriters[0].PutBlock(pINI);
		ByteWriters[1].PutBlock(pINI);
		ByteWriters[2].PutBlock(pINI);
		ByteWriters[3].PutBlock(pINI);
	}

private:
	OverlayByteWriter ByteWriters[4];
};

DEFINE_HOOK(0x5FD2E0, OverlayClass_ReadINI, 0x7)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->CurrentSectionName = nullptr;
	pINI->CurrentSection = nullptr;

	if (ScenarioClass::NewINIFormat > 1)
	{
		OverlayReader reader(pINI);

		for (short i = 0; i < 0x200; ++i)
		{
			for (short j = 0; j < 0x200; ++j)
			{
				CellStruct mapCoord{ j,i };
				size_t nOvl = reader.Get();
				if (nOvl != 0xFFFFFFFF)
				{
					auto const pType = OverlayTypeClass::Array->GetItem(nOvl);
					if (pType->GetImage() || pType->CellAnim)
					{
						if (SessionClass::Instance->GameMode != GameMode::Campaign && pType->Crate)
							continue;
						if (!MapClass::Instance->CoordinatesLegal(mapCoord))
							continue;

						auto pCell = MapClass::Instance->GetCellAt(mapCoord);
						auto const nOriginOvlData = pCell->OverlayData;
						GameCreate<OverlayClass>(pType, mapCoord, -1);

						if (nOvl == 24 || nOvl == 25 || nOvl == 237 || nOvl == 238) // bridges
							pCell->OverlayData = nOriginOvlData;
					}
				}
			}
		}

		auto pBuffer = YRMemory::Allocate(256000);
		size_t uuLength = pINI->ReadUUBlock("OverlayDataPack", pBuffer, 256000);
		if (uuLength > 0)
		{
			BufferStraw bs(pBuffer, uuLength);
			LCWStraw ls(TRUE, 0x2000);
			ls.Get_From(bs);

			for (short i = 0; i < 0x200; ++i)
			{
				for (short j = 0; j < 0x200; ++j)
				{
					CellStruct mapCoord { j,i };
					unsigned char buffer;
					ls.Get(&buffer, sizeof(buffer));

					if (MapClass::Instance->CoordinatesLegal(mapCoord))
					{
						auto pCell = MapClass::Instance->GetCellAt(mapCoord);
						pCell->OverlayData = buffer;
					}
				}
			}
		}
		YRMemory::Deallocate(pBuffer);
	}

	AbstractClass::RemoveAllInactive();

	return 0x5FD69A;
}

DEFINE_HOOK(0x5FD6A0, OverlayClass_WriteINI, 0x6)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->Clear("OVERLAY", nullptr);
	size_t len = DSurface::Alternate->Width * DSurface::Alternate->Height;
	OverlayWriter writer(len);
	OverlayWriter::OverlayByteWriter dataWriter("OverlayDataPack", len);

	for (short i = 0; i < 0x200; ++i)
	{
		for (short j = 0; j < 0x200; ++j)
		{
			CellStruct mapCoord { j,i };
			auto const pCell = MapClass::Instance->GetCellAt(mapCoord);
			writer.Put(pCell->OverlayTypeIndex);
			dataWriter.Put(pCell->OverlayData);
		}
	}

	writer.PutBlock(pINI);
	dataWriter.PutBlock(pINI);

	return 0x5FD8EB;
}


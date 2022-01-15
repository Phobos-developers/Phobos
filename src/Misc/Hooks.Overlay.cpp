#include <Helpers/Macro.h>

#include <CCINIClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <Surface.h>
#include <Straws.h>
#include <SessionClass.h>

// This hook is just original YR code except the overlay reader.
// - Now we need map editors to support integer overlays, but who should take charge of the map editor part?
// - Oh, it's secsome!
// - Yes, it's me!
// - secsome, 2022/01/16
DEFINE_HOOK(0x5FD2E0, OverlayClass_ReadINI, 0x7)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->CurrentSectionName = nullptr;
	pINI->CurrentSection = nullptr;

	if (ScenarioClass::NewINIFormat > 1)
	{
		struct OverlayReader
		{
			struct OverlayByteReader
			{
				OverlayByteReader(CCINIClass* pINI, const char* pSection)
					: ls(TRUE, 0x2000), bs { nullptr,0 }
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

				return *(size_t*)ret;
			}

			OverlayByteReader ByteReaders[4];

			OverlayReader(CCINIClass* pINI)
				:ByteReaders { {pINI,"OverlayPack" }, { pINI,"OverlayPack2" }, { pINI,"OverlayPack3" }, { pINI,"OverlayPack4" }, }
			{}
		};

		OverlayReader reader(pINI);

		for (short i = 0; i < 0x200; ++i)
		{
			for (short j = 0; j < 0x200; ++j)
			{
				CellStruct mapCoord { j,i };
				size_t nOvl = reader.Get();
				if (nOvl != 0xFF)
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
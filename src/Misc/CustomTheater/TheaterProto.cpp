#include <string.h>
#include "TheaterProto.h"

// This is listed default values for vanilla theaters
TheaterProto TheaterProto::Array[] = {
	TheaterProto /* TEMPERATE */ {
		"TEMPERATE",
		"Name:Temperate",
		"TEMPERATMD.INI",
		"TEMPERAT.PAL",
		"UNITTEM.PAL",
		"ISOTEM.PAL",
		"TEM",
		"T",
		"TEMPERAT.MIX,TEM.MIX,ISOTEMMD.MIX,ISOTEMP.MIX",
		false,
		1.0f
	},
	TheaterProto /* SNOW */ {
		"SNOW",
		"Name:Snow",
		"SNOWMD.INI",
		"SNOW.PAL",
		"UNITSNO.PAL",
		"ISOSNO.PAL",
		"SNO",
		"A",
		"SNOWMD.MIX,SNOW.MIX,SNO.MIX,ISOSNOMD.MIX,ISOSNOW.MIX",
		true,
		0.8f
	},
	TheaterProto /* URBAN */ {
		"URBAN",
		"Name:Urban",
		"URBANMD.INI",
		"URBAN.PAL",
		"UNITURB.PAL",
		"ISOURB.PAL",
		"URB",
		"U",
		"URBAN.MIX,URB.MIX,ISOURBMD.MIX,ISOURB.MIX",
		true,
		1.0f
	},
	TheaterProto /* DESERT */ {
		"DESERT",
		"Name:Desert",
		"DESERTMD.INI",
		"DESERT.PAL",
		"UNITDES.PAL",
		"ISODES.PAL",
		"DES",
		"D",
		"DESERT.MIX,DES.MIX,ISODESMD.MIX,ISODES.MIX",
		false,
		1.0f
	},
	TheaterProto /* NEWURBAN */ {
		"NEWURBAN",
		"Name:New Urban",
		"URBANNMD.INI",
		"URBANN.PAL",
		"UNITUBN.PAL",
		"ISOUBN.PAL",
		"UBN",
		"N",
		"URBANN.MIX,UBN.MIX,ISOUBNMD.MIX,ISOUBN.MIX",
		false,
		1.0f
	},
	TheaterProto /* LUNAR */ {
		"LUNAR",
		"Name:Lunar",
		"LUNARMD.INI",
		"LUNAR.PAL",
		"UNITLUN.PAL",
		"ISOLUN.PAL",
		"LUN",
		"L",
		"LUNAR.MIX,LUN.MIX,ISOLUNMD.MIX,ISOLUN.MIX",
		false,
		1.0f
	}
};
constexpr int TheaterProto::ArraySize = sizeof(TheaterProto::Array) / sizeof(*TheaterProto::Array);

TheaterProto* TheaterProto::Temperate = &TheaterProto::Array[0];
TheaterProto* TheaterProto::Snow = &TheaterProto::Array[1];
TheaterProto* TheaterProto::Urban = &TheaterProto::Array[2];
TheaterProto* TheaterProto::Desert = &TheaterProto::Array[3];
TheaterProto* TheaterProto::NewUrban = &TheaterProto::Array[4];
TheaterProto* TheaterProto::Lunar = &TheaterProto::Array[5];

int TheaterProto::FindIndex(const char* id)
{
	for (int i = 0; i < TheaterProto::ArraySize; i++)
	{
		if (_stricmp(TheaterProto::Array[i].ID, id) == 0)
			return i;
	}
	return -1;
}

TheaterProto* TheaterProto::Get(const char* id)
{
	const int index = TheaterProto::FindIndex(id);
	if (index >= 0)
		return &TheaterProto::Array[index];

	return TheaterProto::Temperate;
}

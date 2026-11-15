#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x55E477, Game_ComScenarioDialog_ChatBox, 0x5)
{
	if (RulesExt::Global()->AllowChatBoxInSinglePlayer)
		return 0x55E48D;

	return 0;
}

DEFINE_HOOK(0x55E62F, Game_ComScenarioDialog_ChatBox2, 0x6)
{
	if (RulesExt::Global()->AllowChatBoxInSinglePlayer)
		return 0x55E637;

	return 0;
}

DEFINE_HOOK(0x55E693, Game_ComScenarioDialog_ChatBox3, 0x6)
{
	if (RulesExt::Global()->AllowChatBoxInSinglePlayer)
		return 0x55E69B;

	return 0;
}

DEFINE_HOOK(0x55E746, Game_ComScenarioDialog_ChatBox4, 0x5)
{
	if (RulesExt::Global()->AllowChatBoxInSinglePlayer)
		return 0x55E77B;

	return 0;
}

#include "Body.h"

// score options
// score music for single player missions
DEFINE_HOOK(0x6C924F, ScoreDialog_Handle_ScoreThemeA, 0x5)
{
	GET(char*, pTitle, ECX);
	GET(char*, pMessage, ESI);
	CSFText& Title = ScenarioExt::Global()->ParTitle;
	CSFText& Message = ScenarioExt::Global()->ParMessage;

	strcpy(pTitle, Title.Label);
	strcpy(pMessage, Message.Label);

	return 0;
}

DEFINE_HOOK(0x6C935C, ScoreDialog_Handle_ScoreThemeB, 0x5)
{
	REF_STACK(char*, pTheme, 0x0);

	auto& Theme = ScenarioExt::Global()->ScoreCampaignTheme;

	if (Theme.isset())
		strcpy(pTheme, Theme.Get().data());

	return 0;
}

DEFINE_HOOK(0x5AE192, SelectNextMission, 0x6)
{
	if(ScenarioExt::Global()->NextMission.isset())
		R->EAX(ScenarioExt::Global()->NextMission.Get().data());

	return 0;
}

#include "Themes.h"
ThemePlayer* ThemePlayer::Instance = reinterpret_cast<ThemePlayer*>(0xA83D10);

#include "CCToolTip.h"
CCToolTip* CCToolTip::Instance = reinterpret_cast<CCToolTip*>(0x887368);
bool CCToolTip::HideName = *reinterpret_cast<byte*>(0x884B8C);

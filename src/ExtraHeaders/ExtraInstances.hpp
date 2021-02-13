#include "Themes.h"
ThemePlayer* ThemePlayer::Instance = reinterpret_cast<ThemePlayer*>(0xA83D10);

#include "CCToolTip.h"
bool CCToolTip::HideName = *reinterpret_cast<byte*>(0x884B8C);

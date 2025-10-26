// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <Surface.h>

class SurfaceExt : public Surface
{
public:
	void BlurRect(const RectangleStruct& rect, float blurSize);

};

#pragma once

#include <Surface.h>

class SurfaceExt : public Surface
{
public:
	void BlurRect(const RectangleStruct& rect, float blurSize);

};

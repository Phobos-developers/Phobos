#include "BannerClass.h"

#include <Ext/Scenario/Body.h>

#include <New/Type/BannerTypeClass.h>

#include <Utilities/SavegameDef.h>

std::vector<std::unique_ptr<BannerClass>> BannerClass::Array;

BannerClass::BannerClass
(
	BannerTypeClass* pBannerType,
	int id,
	const CoordStruct& position,
	int variable,
	bool isGlobalVariable
)
	: Type(pBannerType)
	, ID(id)
	, Position(position)
	, Variable(variable)
	, IsGlobalVariable(isGlobalVariable)
{ }

void BannerClass::Render()
{
	int x = static_cast<int>(this->Position.X / 100.0 * DSurface::Composite->Width);
	int y = static_cast<int>(this->Position.Y / 100.0 * DSurface::Composite->Height);

	switch (this->Type->BannerType)
	{
	case BannerType::PCX:
		this->RenderPCX(x, y);
		break;
	case BannerType::SHP:
		this->RenderSHP(x, y);
		break;
	case BannerType::CSF:
		this->RenderCSF(x, y);
		break;
	case BannerType::VariableFormat:
		this->RenderVariable(x, y);
		break;
	default:
		break;
	}
}

void BannerClass::RenderPCX(int x, int y)
{
	int xsize = 0;
	int ysize = 0;

	if (this->Type->BannerType == BannerType::PCX)
	{
		BSurface* pcx = this->Type->PCX;

		if (pcx == nullptr)
			return;

		x = x - pcx->Width / 2;
		y = y - pcx->Height / 2;
		xsize = pcx->Width;
		ysize = pcx->Height;

		RectangleStruct bounds(x, y, xsize, ysize);
		PCX::Instance.BlitToSurface(&bounds, DSurface::Composite, pcx);
	}
}

void BannerClass::RenderSHP(int x, int y)
{
	if (this->Type->BannerType == BannerType::SHP)
	{
		SHPStruct* shape = this->Type->Shape;
		ConvertClass* palette = this->Type->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		if (shape == nullptr)
			return;

		x = x - shape->Width / 2;
		y = y - shape->Height / 2;

		Point2D pos(x, y);
		DSurface::Composite->DrawSHP
		(
			palette,
			shape,
			this->ShapeFrameIndex,
			&pos,
			&DSurface::ViewBounds,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			nullptr,
			0,
			0,
			0
		);

		this->ShapeFrameIndex++;

		if (this->ShapeFrameIndex >= shape->Frames)
			this->ShapeFrameIndex = 0;
	}
}

void BannerClass::RenderCSF(int x, int y)
{
	RectangleStruct rect;
	DSurface::Composite->GetRect(&rect);
	Point2D pos(x, y);

	std::wstring text = this->Type->CSF.Get().Text;

	const auto& variables = ScenarioExt::Global()->Variables[this->IsGlobalVariable != 0];
	const auto& it = variables.find(this->Variable);

	if (it != variables.end())
		text += std::to_wstring(it->second.Value);

	TextPrintType textFlags = TextPrintType::UseGradPal
		| TextPrintType::Center
		| TextPrintType::Metal12
		| (this->Type->CSF_Background
			? TextPrintType::Background
			: TextPrintType::LASTPOINT);

	DSurface::Composite->DrawText
	(
		text.c_str(),
		&rect,
		&pos,
		Drawing::RGB_To_Int(this->Type->CSF_Color),
		0,
		textFlags
	);
}

void BannerClass::RenderVariable(int x, int y)
{
	//nothing here
}

template <typename T>
bool BannerClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->ID)
		.Process(this->Type)
		.Process(this->Position)
		.Process(this->Variable)
		.Process(this->ShapeFrameIndex)
		.Process(this->IsGlobalVariable)
		.Success();
}

bool BannerClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool BannerClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<BannerClass*>(this)->Serialize(stm);
}

void BannerClass::Clear()
{
	Array.clear();
}

bool BannerClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Array)
		.Success();
}

bool BannerClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Array)
		.Success();
}

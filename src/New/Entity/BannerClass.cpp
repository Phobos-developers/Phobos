#include "BannerClass.h"

#include <Ext/Scenario/Body.h>

#include <New/Type/BannerTypeClass.h>

#include <Utilities/SavegameDef.h>

std::vector<std::unique_ptr<BannerClass>> BannerClass::Array;

BannerClass::BannerClass
(
	BannerTypeClass* pBannerType,
	int id,
	Point2D position,
	int variable,
	bool isGlobalVariable
)
	: Type(pBannerType)
	, ID(id)
	, Position(static_cast<int>(position.X / 100.0 * DSurface::Composite->Width), static_cast<int>(position.Y / 100.0 * DSurface::Composite->Height))
	, Variable(variable)
	, IsGlobalVariable(isGlobalVariable)
{ }

void BannerClass::Render()
{
	if (this->Type->PCX.GetSurface())
		this->RenderPCX(this->Position);
	else if (this->Type->Shape)
		this->RenderSHP(this->Position);
	else if (!this->Type->CSF.Get().empty() || this->Type->CSF_VariableFormat != BannerNumberType::None)
		this->RenderCSF(this->Position);
}

void BannerClass::RenderPCX(Point2D position)
{
	BSurface* pcx = this->Type->PCX.GetSurface();
	position.X -= pcx->Width / 2;
	position.Y -= pcx->Height / 2;
	RectangleStruct bounds(position.X, position.Y, pcx->Width, pcx->Height);
	PCX::Instance.BlitToSurface(&bounds, DSurface::Composite, pcx);
}

void BannerClass::RenderSHP(Point2D position)
{
	SHPStruct* shape = this->Type->Shape;
	ConvertClass* palette = this->Type->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	position.X -= shape->Width / 2;
	position.Y -= shape->Height / 2;

	DSurface::Composite->DrawSHP
	(
		palette,
		shape,
		this->ShapeFrameIndex,
		&position,
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

void BannerClass::RenderCSF(Point2D position)
{
	RectangleStruct rect;
	DSurface::Composite->GetRect(&rect);
	std::wstring text;

	if (this->Type->CSF_VariableFormat != BannerNumberType::None)
	{
		const auto& variables = ScenarioExt::Global()->Variables[this->IsGlobalVariable != 0];
		const auto& it = variables.find(this->Variable);

		if (it != variables.end())
		{
			switch (this->Type->CSF_VariableFormat)
			{
				case BannerNumberType::Variable:
					text = std::to_wstring(it->second.Value);
				case BannerNumberType::Prefixed:
					text = std::to_wstring(it->second.Value) + this->Type->CSF.Get().Text;
				case BannerNumberType::Suffixed:
					text = this->Type->CSF.Get().Text + std::to_wstring(it->second.Value);
			}
		}
	}

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
		&position,
		Drawing::RGB_To_Int(this->Type->CSF_Color.Get(Drawing::TooltipColor)),
		0,
		textFlags
	);
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

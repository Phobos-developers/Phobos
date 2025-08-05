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
	, Position(static_cast<int>(position.X / 100.0 * DSurface::ViewBounds.Width), static_cast<int>(position.Y / 100.0 * DSurface::ViewBounds.Height))
	, Variable(variable)
	, IsGlobalVariable(isGlobalVariable)
{
	this->Duration = pBannerType->Duration;
	this->Delay = pBannerType->Delay;
}

void BannerClass::Render()
{
	const auto pType = this->Type;

	if (this->Duration > 0)
	{
		this->Duration--;
	}
	else if (this->Duration == 0)
	{
		if (this->Delay < 0)
		{
			return;
		}
		else if (this->Delay > 0)
		{
			this->Delay--;
			return;
		}
		else if (this->Delay == 0)
		{
			this->Duration = pType->Duration;
			this->Delay = pType->Delay;

			if (pType->Shape_RefreshAfterDelay)
				this->ShapeFrameIndex = 0;
		}
	}

	if (pType->PCX.GetSurface())
		this->RenderPCX(this->Position);
	else if (pType->Shape)
		this->RenderSHP(this->Position);
	else if (!pType->CSF.Get().empty() || pType->CSF_VariableFormat != BannerNumberType::None)
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
	auto const pType = this->Type;
	SHPStruct* shape = pType->Shape;
	ConvertClass* palette = pType->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
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
	auto const pType = this->Type;
	RectangleStruct rect = DSurface::ViewBounds;
	std::wstring text;

	if (pType->CSF_VariableFormat != BannerNumberType::None)
	{
		const auto& variables = ScenarioExt::Global()->Variables[this->IsGlobalVariable != 0];
		const auto& it = variables.find(this->Variable);

		if (it != variables.end())
		{
			switch (pType->CSF_VariableFormat)
			{
				case BannerNumberType::Variable:
					text = std::to_wstring(it->second.Value);
					break;
				case BannerNumberType::Prefixed:
					text = std::to_wstring(it->second.Value) + pType->CSF.Get().Text;
					break;
				case BannerNumberType::Suffixed:
					text = pType->CSF.Get().Text + std::to_wstring(it->second.Value);
					break;
			}
		}
	}
	else
	{
		text = pType->CSF.Get().Text;
	}

	TextPrintType textFlags = TextPrintType::UseGradPal
		| TextPrintType::Center
		| TextPrintType::Metal12
		| (pType->CSF_Background
			? TextPrintType::Background
			: TextPrintType::LASTPOINT);

	DSurface::Composite->DrawText
	(
		text.c_str(),
		&rect,
		&position,
		Drawing::RGB_To_Int(pType->CSF_Color.Get(Drawing::TooltipColor)),
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
		.Process(this->Duration)
		.Process(this->Delay)
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

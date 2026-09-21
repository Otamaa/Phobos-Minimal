#include "BannerClass.h"

#include <Ext/Scenario/Body.h>

#include <New/Type/BannerTypeClass.h>

#include <Utilities/SavegameDef.h>

#include <PCX.h>

BannerManagerClass BannerManagerClass::Instance;

void BannerManagerClass::Clear()
{
	Array.clear();
}

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
	, Position(static_cast<int>(position.X / 100.0 * DSurface::ViewBounds->Width), static_cast<int>(position.Y / 100.0 * DSurface::ViewBounds->Height))
	, Variable(variable)
	, IsGlobalVariable(isGlobalVariable)
	, Duration(pBannerType->Duration)
	, Delay(pBannerType->Delay)
{ }

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

	if (this->Type->PCX.GetSurface())
		this->RenderPCX(this->Position);
	else if (this->Type->Shape)
		this->RenderSHP(this->Position);
	else if (!this->Type->CSF.Get().empty() || this->Type->CSF_VariableFormat != BannerNumberType::None)
		this->RenderCSF(this->Position);
}

void BannerClass::GetRenderPos(Point2D& position, int W , int H)
{
	switch (this->Type->Horizontal)
	{
	case HorizontalPosition::Center:
		position.X -= W / 2;
		break;
	case HorizontalPosition::Right:
		position.X -= W;
		break;
	default:
		break;
	}

	switch (this->Type->Vertical)
	{
	case VerticalPosition::Center:
		position.Y -= H / 2;
		break;
	case VerticalPosition::Bottom:
		position.Y -= H;
		break;
	default:
		break;
	}

	if (this->Type->ClampToScreen) {
		BannerClass::Clamp(position, W, H);
	}
}

void BannerClass::RenderPCX(Point2D position)
{
	BSurface* pcx = this->Type->PCX.GetSurface();

	if (!pcx)
		return;

	this->GetRenderPos(position, pcx->Width, pcx->Height);

	RectangleStruct bounds(position.X, position.Y, pcx->Width, pcx->Height);
	PCXImages::Instance->BlitToSurface(&bounds, DSurface::Composite, pcx);
}

void BannerClass::RenderSHP(Point2D position)
{
	SHPCaches* shape = this->Type->Shape;
	if (!shape)
		return;

	ConvertClass* palette = this->Type->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	this->GetRenderPos(position, shape->CurrentHeader.Width, shape->CurrentHeader.Height);

	DSurface::Composite->DrawSHP
	(
		palette,
		shape,
		this->ShapeFrameIndex,
		&position,
		&DSurface::ViewBounds.get(),
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

	if (this->ShapeFrameIndex >= shape->CurrentHeader.Frames)
		this->ShapeFrameIndex = 0;
}

void BannerClass::RenderCSF(Point2D position)
{
	// the banner is multiple instances, so sharing static buffer is kind a doesnt make sense here
	fmt::basic_memory_buffer<wchar_t> buffer {};

	if (this->Type->CSF_VariableFormat != BannerNumberType::None) {

		const auto& variables = ScenarioExtData::Instance()->GetVariables(this->IsGlobalVariable != 0);
		const auto& it = variables->get_key_iterator(this->Variable);

		if (it != variables->end())
		{
			switch (this->Type->CSF_VariableFormat)
			{
			case BannerNumberType::Variable:
				fmt::format_to(std::back_inserter(buffer), L"{}", it->second.Value);
				break;
			case BannerNumberType::Prefixed:
				fmt::format_to(std::back_inserter(buffer), L"{}{}", it->second.Value, this->Type->CSF.Get().Text);
				break;
			case BannerNumberType::Suffixed:
				fmt::format_to(std::back_inserter(buffer), L"{}{}", this->Type->CSF.Get().Text, it->second.Value);
				break;
			default:
				//Debug::Log("Variation is not recognized for BannerNumberType !\n");
				return;
			}
		}
	} else {
		fmt::format_to(std::back_inserter(buffer), L"{}", this->Type->CSF.Get().Text);
	}

	if (buffer.size() == 0) {
		//Debug::Log("Cannot Draw Empty string with BannerClass::RenderCSF !\n");
		return;
	}
		
	buffer.push_back(L'\0');

	TextPrintType textFlags = TextPrintType::UseGradPal

		| TextPrintType::Metal12
		| (this->Type->CSF_Background
			? TextPrintType::Background
			: TextPrintType::LASTPOINT)
		| (this->Type->ClampToScreen
			? TextPrintType::LASTPOINT
			: TextPrintType::Center);

	RectangleStruct textRect = Drawing::GetTextDimensions(buffer.data(), position, textFlags, 0, 0);
	this->GetRenderPos(position, textRect.Width, textRect.Height);

	DSurface::Composite->DSurfaceDrawText
	(
		buffer.data(),
		&DSurface::ViewBounds(),
		&position,
		this->Type->CSF_Color.Get(Drawing::TooltipColor).ToInit(),
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
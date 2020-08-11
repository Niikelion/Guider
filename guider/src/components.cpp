#include <guider/components.hpp>

namespace Guider
{
	void EmptyComponent::onDraw(RenderBackend& renderer) const
	{
	}

	EmptyComponent::EmptyComponent(Manager& m, const XML::Tag& config)
	{
	}

	void RectangleShape::onDraw(RenderBackend& renderer) const
	{
		Rect bounds = getBounds();
		renderer.setColor(color);
		renderer.drawRectangle(Rect(0, 0, bounds.width, bounds.height));
	}
	void TextComponent::setTextSize(float textSize)
	{
		this->textSize = textSize;
		invalidate();
	}
	void TextComponent::setText(const std::string& text)
	{
		this->text = text;
		invalidate();
	}
	void TextComponent::setTextColor(const Color& color)
	{
		this->color = color;
		invalidateVisuals();
	}
	void TextComponent::onDraw(RenderBackend& renderer) const
	{
		renderer.setTextSize(textSize);
		renderer.setColor(color);
		renderer.drawText(0, 0, text);
	}
}
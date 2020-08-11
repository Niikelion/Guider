#pragma once
#include <guider/base.hpp>
#include <guider/manager.hpp>

namespace Guider
{
	class EmptyComponent : public Component
	{
	public:
		void onDraw(RenderBackend& renderer) const override;

		EmptyComponent() = default;
		EmptyComponent(Manager& m, const XML::Tag& config);
	};

	class RectangleShape : public Component
	{
	private:
		Color color;
	public:
		void onDraw(RenderBackend& renderer) const override;

		RectangleShape() : color(255, 255, 255, 255) {}
		RectangleShape(SizingMode mode, const Color& c)
		{
			setSizingMode(mode, mode);
			color = c;
		}
		RectangleShape(float w, float h, const Color& c) : Component()
		{
			color = c;
			setSize(Vec2(w, h));
			setSizingMode(SizingMode::OwnSize, SizingMode::OwnSize);
		}

		RectangleShape(Manager& m, const XML::Tag& config) : RectangleShape()
		{
			Manager::handleDefaultArguments(*this, config);
			{
				XML::Value fillColor = config.getAttribute("fillColor");
				if (fillColor.exists())
				{
					color = Manager::strToColor(fillColor.val);
				}
			}
		}
	};

	class TextComponent : public Component
	{
	private:
		std::string text;
		float textSize;
		Color color;
	public:
		void setTextSize(float textSize);
		void setText(const std::string& text);
		void setTextColor(const Color& color);

		void onDraw(RenderBackend& renderer) const override;

		TextComponent(): textSize(10),color(0xff), Component() {}

		TextComponent(Manager& manager, const XML::Tag& config): TextComponent()
		{
			Manager::handleDefaultArguments(*this, config);
			XML::Value tmp = config.getAttribute("color");
			if (tmp.exists())
			{
				bool failed = false;
				Color c = Manager::strToColor(tmp.val, failed);
				if (!failed)
					setTextColor(c);
			}
			tmp = config.getAttribute("text");
			if (tmp.exists())
			{
				setText(tmp.val);
			}
			tmp = config.getAttribute("textSize");
			if (tmp.exists())
			{
				bool failed = false;
				float v = Manager::strToFloat(tmp.val, failed);
				if (!failed)
					setTextSize(v);
			}
		}
	};
}
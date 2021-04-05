#pragma once
#include <guider/base.hpp>
#include <guider/manager.hpp>
#include <functional>
#include <algorithm>

namespace Guider
{
	class CommonComponent : public Component
	{
	private:
		Padding paddings;
	protected:
		Rect getContentRect() const noexcept;
	public:
		void setPadding(const Padding& pad);
		Padding getPading() const noexcept;

		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) = 0;

		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height) override;

		CommonComponent() = default;
		CommonComponent(Manager& m, const XML::Tag& tag, const Style& style);
	};

	class EmptyComponent : public Component
	{
	public:
		void onDraw(Canvas& canvas) const override {}

		EmptyComponent() = default;
		EmptyComponent(Manager& m, const XML::Tag& config, const Style& style) {}
	};

	class RectangleShapeComponent : public Component
	{
	private:
		std::shared_ptr<Resources::RectangleShape> shape;
		Color color;
	public:
		void setColor(const Color& c);
		inline Color getColor() const noexcept
		{
			return color;
		}

		virtual void onDraw(Canvas& canvas) const override;

		virtual void handleEvent(const Event& event) override;

		RectangleShapeComponent() : shape(nullptr), color(255, 255, 255, 255) {}
		RectangleShapeComponent(SizingMode mode, const Color& c): shape(nullptr), color(c)
		{
			setSizingMode(mode, mode);
		}
		RectangleShapeComponent(float w, float h, const Color& c) : Component(), shape(nullptr), color(c)
		{
			setSize(Vec2(w, h));
			setSizingMode(SizingMode::OwnSize, SizingMode::OwnSize);
		}

		RectangleShapeComponent(Manager& m, const XML::Tag& config, const Style& style) : RectangleShapeComponent()
		{
			Manager::handleDefaultArguments(*this, config, style);
			{
				XML::Value fillColor = config.getAttribute("fillColor");
				if (fillColor.exists())
				{
					color = Styles::strToColor(fillColor.val);
				}
			}
		}
	};
	
	class TextComponent : public CommonComponent
	{
	private:
		std::shared_ptr<Resources::TextResource> textRes;
		std::string text,font;
		float textSize;
		Color color;
		Gravity horizontalTextAlign, verticalTextAlign;
	public:
		void setTextSize(float textSize);
		void setText(const std::string& text);
		void setTextColor(const Color& color);
		void setFont(const std::string& name);

		void setTextAlignment(Gravity horizontal, Gravity vertical);
		inline Gravity getHorizontalTextAlignment() const
		{
			return horizontalTextAlign;
		}
		inline Gravity getVerticalTextAlignment() const
		{
			return verticalTextAlign;
		}

		virtual void onDraw(Canvas& canvas) const override;
		
		virtual void handleEvent(const Event& event) override;

		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) override;

		TextComponent() : textRes(nullptr), textSize(10),color(0xff), horizontalTextAlign(Gravity::Start), verticalTextAlign(Gravity::Center) {}

		TextComponent(Manager& manager, const XML::Tag& config, const Style& style): CommonComponent(manager, config, style), textRes(nullptr), textSize(10), color(0xff), horizontalTextAlign(Gravity::Start), verticalTextAlign(Gravity::Center)
		{
			Manager::handleDefaultArguments(*this, config, style);
			XML::Value tmp = config.getAttribute("color");
			if (tmp.exists())
			{
				bool failed = false;
				Color c = Styles::strToColor(tmp.val, failed);
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
				float v = Styles::strToFloat(tmp.val, failed);
				if (!failed)
					setTextSize(v);
			}

			tmp = config.getAttribute("font");
			if (tmp.exists())
			{
				auto font = manager.getFontByName(tmp.val);
				if (font)
				{
					setFont(tmp.val);
				}
			}

			Gravity hor = Gravity::Center, ver = Gravity::Center;
			tmp = config.getAttribute("textAlignmentHorizontal");
			if (tmp.exists())
			{
				if (tmp.val == "left" || tmp.val == "start")
					hor = Gravity::Left;
				else if (tmp.val == "center" || tmp.val == "middle")
					hor = Gravity::Center;
				else if (tmp.val == "right" || tmp.val == "end")
					hor = Gravity::Right;
			}
			tmp = config.getAttribute("textAlignmentVertical");
			if (tmp.exists())
			{
				if (tmp.val == "top" || tmp.val == "start")
					ver = Gravity::Top;
				else if (tmp.val == "center" || tmp.val == "middle")
					ver = Gravity::Center;
				else if (tmp.val == "bottom" || tmp.val == "end")
					ver = Gravity::Bottom;
			}
		}
	};

	class ButtonBase
	{
	public:
		enum class ButtonState
		{
			Default,
			Hovered,
			Clicked
		};
	private:
		std::function<void(Component&)> onClickCallback;
		ButtonState buttonState;
	protected:
		void handleClick(const Event& event);
		virtual Component& getThisComponent() = 0;
	public:
		inline ButtonState getButtonState() const noexcept
		{
			return buttonState;
		}
		void setOnClickCallback(const std::function<void(Component&)>& callback);

		ButtonBase() : buttonState(ButtonState::Default) {}
	};

	class BasicButtonComponent : public TextComponent, public ButtonBase
	{
	private:
		std::shared_ptr<Resources::Drawable> backgroundDefault, backgroundSelected, backgroundClicked;
	protected:
		Component& getThisComponent() override;
	public:
		static void defineProperties(Manager& m);

		void drawMask(Backend& b) const override;

		std::shared_ptr<Resources::Drawable> getBackgroundDrawable(ButtonState state) const;
		inline std::shared_ptr<Resources::Drawable> getCurrentBackgroundDrawable() const
		{
			return getBackgroundDrawable(getButtonState());
		}

		void onDraw(Canvas& canvas) const override;

		void handleEvent(const Event& event) override;

		BasicButtonComponent() = default;
		
		BasicButtonComponent(Manager& manager, const XML::Tag& config, const Style& style) : TextComponent(manager, config, style)
		{
			auto backgroundP = style.getAttribute("background");
			if (backgroundP)
				backgroundDefault = backgroundP->as<std::shared_ptr<Resources::Drawable>>();
			auto selectedBackgroundP = style.getAttribute("selectedBackground");
			if (selectedBackgroundP)
				backgroundClicked = selectedBackgroundP->as<std::shared_ptr<Resources::Drawable>>();
			auto hoveredBackgroundP = style.getAttribute("hoveredBackground");
			if (hoveredBackgroundP)
				backgroundSelected = hoveredBackgroundP->as<std::shared_ptr<Resources::Drawable>>();
		}
	};
}
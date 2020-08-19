#pragma once
#include <guider/base.hpp>
#include <guider/manager.hpp>
#include <functional>
#include <algorithm>

namespace Guider
{
	class Padding
	{
	public:
		float left, right, top, bottom;

		Rect calcContentArea(const Rect& bounds) const;

		Padding() : left(0), right(0), top(0), bottom(0) {}
		Padding(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b) {}
		Padding(const Padding&) = default;
	};

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
		CommonComponent(Manager& m, const XML::Tag& tag);
	};

	class EmptyComponent : public Component
	{
	public:
		void onDraw(Backend& renderer) const override;

		EmptyComponent() = default;
		EmptyComponent(Manager& m, const XML::Tag& config);
	};

	class RectangleShape : public Component
	{
	private:
		Backend::RectangleShape* shape;
		Color color;
	public:
		void setColor(const Color& c);
		inline Color getColor() const noexcept
		{
			return color;
		}

		virtual void onDraw(Backend& renderer) const override;

		virtual void handleEvent(const Event& event) override;

		RectangleShape() : shape(nullptr), color(255, 255, 255, 255) {}
		RectangleShape(SizingMode mode, const Color& c): shape(nullptr), color(c)
		{
			setSizingMode(mode, mode);
		}
		RectangleShape(float w, float h, const Color& c) : Component(), shape(nullptr), color(c)
		{
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
	
	class TextComponent : public CommonComponent
	{
	public:
		enum class TextAlignment
		{
			Start,
			Center,
			End
		};
	private:
		std::shared_ptr<Backend::TextResource> textRes;
		std::string text,font;
		float textSize;
		Color color;
		TextAlignment horizontalTextAlign, verticalTextAlign;
	public:
		void setTextSize(float textSize);
		void setText(const std::string& text);
		void setTextColor(const Color& color);
		void setFont(const std::string& name);

		void setTextAlignment(TextAlignment horizontal, TextAlignment vertical);
		inline TextAlignment getHorizontalTextAlignment() const
		{
			return horizontalTextAlign;
		}
		inline TextAlignment getVerticalTextAlignment() const
		{
			return verticalTextAlign;
		}

		virtual void onDraw(Backend& renderer) const override;
		
		virtual void handleEvent(const Event& event) override;

		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) override;

		TextComponent() : textRes(nullptr), textSize(10),color(0xff), horizontalTextAlign(TextAlignment::Start), verticalTextAlign(TextAlignment::Center) {}

		TextComponent(Manager& manager, const XML::Tag& config): CommonComponent(manager, config), textRes(nullptr), textSize(10), color(0xff), horizontalTextAlign(TextAlignment::Start), verticalTextAlign(TextAlignment::Center)
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

			TextAlignment hor = TextAlignment::Start, ver = TextAlignment::Center;
			tmp = config.getAttribute("textAlignmentHorizontal");
			if (tmp.exists())
			{
				if (tmp.val == "left" || tmp.val == "start")
					hor = TextAlignment::Start;
				else if (tmp.val == "center" || tmp.val == "middle")
					hor = TextAlignment::Center;
				else if (tmp.val == "right" || tmp.val == "end")
					hor = TextAlignment::End;
			}
			tmp = config.getAttribute("textAlignmentVertical");
			if (tmp.exists())
			{
				if (tmp.val == "top" || tmp.val == "start")
					ver = TextAlignment::Start;
				else if (tmp.val == "center" || tmp.val == "middle")
					ver = TextAlignment::Center;
				else if (tmp.val == "right" || tmp.val == "end")
					ver = TextAlignment::End;
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
		//TODO: switch to drawables with manager
		std::shared_ptr<Backend::RectangleShape> backgroundDefault, backgroundSelected, backgroundClicked;
	protected:
		Component& getThisComponent() override;
	public:
		void drawMask(Backend& b) const override;

		std::shared_ptr<Backend::RectangleShape> getBackgroundDrawable(ButtonState state) const;
		inline std::shared_ptr<Backend::RectangleShape> getCurrentBackgroundDrawable() const
		{
			return getBackgroundDrawable(getButtonState());
		}

		void onDraw(Backend& backend) const override;

		void handleEvent(const Event& event) override;

		using TextComponent::TextComponent;
	};
}
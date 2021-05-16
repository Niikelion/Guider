#pragma once
#include <guider/base.hpp>
#include <guider/manager.hpp>
#include <functional>
#include <algorithm>

namespace Guider
{
	class CommonComponent : public Component
	{
	public:
		static void registerProperties(Manager& m, const std::string& name);

		void setPadding(const Padding& pad);
		Padding getPading() const noexcept;

		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) = 0;

		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height) override;

		CommonComponent() = default;
		CommonComponent(Manager& m, const XML::Tag& tag, const StylingPack& style);
	protected:
		Rect getContentRect() const noexcept;
	private:
		Padding paddings;
	};

	class EmptyComponent : public Component
	{
	public:
		void onDraw(Canvas& canvas) override {}

		EmptyComponent() = default;
		EmptyComponent(Manager& m, const XML::Tag& config, const StylingPack& style);
	};

	class RectangleShapeComponent : public Component
	{
	public:
		void setColor(const Color& c);
		Color getColor() const noexcept;

		virtual void onDraw(Canvas& canvas) override;

		virtual bool handleEvent(const Event& event) override;

		RectangleShapeComponent();
		RectangleShapeComponent(SizingMode mode, const Color& c);
		RectangleShapeComponent(float w, float h, const Color& c);

		RectangleShapeComponent(Manager& m, const XML::Tag& config, const StylingPack& style);
	private:
		std::shared_ptr<Resources::RectangleShape> shape;
		Color color;
	};

	class TextComponent : public CommonComponent
	{
	public:
		static void registerProperties(Manager& m, const std::string& name);

		void setTextSize(float textSize);
		void setText(const std::string& text);
		void setTextColor(const Color& color);
		void setFont(const std::string& name);

		void setTextAlignment(Gravity horizontal, Gravity vertical);
		Gravity getHorizontalTextAlignment() const;
		Gravity getVerticalTextAlignment() const;

		virtual void onDraw(Canvas& canvas) override;

		virtual bool handleEvent(const Event& event) override;

		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) override;

		TextComponent();

		TextComponent(Manager& manager, const XML::Tag& config, const StylingPack& pack);
	private:
		std::shared_ptr<Resources::TextResource> textRes;
		std::string text, font;
		float textSize;
		Color color;
		Gravity horizontalTextAlign, verticalTextAlign;
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

		ButtonState getButtonState() const noexcept;
		void setOnClickCallback(const std::function<void(Component&)>& callback);

		ButtonBase();
	protected:
		void handleClick(const Event& event);
		virtual Component& getThisComponent() = 0;
	private:
		std::function<void(Component&)> onClickCallback;
		ButtonState buttonState;
	};

	class BasicButtonComponent : public TextComponent, public ButtonBase
	{
	public:
		static void registerProperties(Manager& m, const std::string& name);

		std::shared_ptr<Resources::Drawable> getBackgroundDrawable(ButtonState state) const;
		std::shared_ptr<Resources::Drawable> getCurrentBackgroundDrawable() const;

		virtual void onDraw(Canvas& canvas) override;

		virtual bool handleEvent(const Event& event) override;

		BasicButtonComponent() = default;

		BasicButtonComponent(Manager& manager, const XML::Tag& config, const StylingPack& style);
	protected:
		virtual Component& getThisComponent() override;
	private:
		std::shared_ptr<Resources::Drawable> backgroundDefault, backgroundSelected, backgroundClicked;
	};
}
#pragma once
#include <guider/base.hpp>
#include <guider/manager.hpp>
#include <functional>
#include <algorithm>

namespace Guider
{
	/// @interface CommonComponent
	/// @brief Base for common components.
	class CommonComponent : public Component
	{
	public:
		/// @brief Registers basic properties, is required by Manager.
		/// @param manager Resource manager to register properties to.
		/// @param name Alias for type.
		static void registerProperties(Manager& manager, const std::string& name);

		/// @brief Sets padding.
		void setPadding(const Padding& pad);
		/// @brief Returns padding.
		Padding getPading() const noexcept;

		/// @brief Returns size of content.
		/// 
		/// Convenience method required for measure to be implemented.
		/// @param getWidth True if content width is requested.
		/// @param getHeight True if content height is requested.
		/// @return Returns pair (width, height). Fields that are not required have undefined value.
		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) = 0;

		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height) override;

		CommonComponent() = default;
		/// @brief Constructs component from xml.
		/// @param manager Resource manager.
		/// @param tag Xml representation.
		/// @param pack Styling information.
		CommonComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack);
	protected:
		/// @brief Returns content rect.
		/// 
		/// Content rect includes padding.
		Rect getContentRect() const noexcept;
	private:
		Padding paddings;
	};

	/// @brief Empty component.
	/// @note EmptyComponent will try to maintain size equal to zero.
	class EmptyComponent : public Component
	{
	public:
		static void registerProperties(Manager& manager, const std::string& name);

		void onDraw(Canvas& canvas) override {}

		EmptyComponent() = default;
		/// @brief Constructs component from xml.
		/// @param manager Resource manager.
		/// @param tag Xml representation.
		/// @param pack Styling information.
		EmptyComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack);
	};

	/// @brief Simple solid color rectangle
	class RectangleShapeComponent : public Component
	{
	public:
		/// @brief Registers all ractangle specific properties.
		/// @param manager Resource manager to register properties to.
		/// @param name Alias for type.
		static void registerProperties(Manager& manager, const std::string& name);
		/// @brief Sets color.
		/// @param c Color.
		void setColor(const Color& c);
		/// @brief Returns color.
		Color getColor() const noexcept;

		virtual void onDraw(Canvas& canvas) override;

		virtual bool handleEvent(const Event& event) override;

		RectangleShapeComponent();
		/// @brief Constructs rectangle from sizing modes and color.
		/// @param mode Sizing modes.
		/// @param c Fill color.
		RectangleShapeComponent(SizingMode mode, const Color& c);
		/// @brief Constructs rectangle from size and color.
		/// @param w Width.
		/// @param h Height.
		/// @param c Fill color.
		RectangleShapeComponent(float w, float h, const Color& c);
		/// @brief Constructs rectangle from xml.
		/// @param manager Resource manager.
		/// @param tag Xml representation.
		/// @param pack Styling information.
		RectangleShapeComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack);
	private:
		std::shared_ptr<Resources::RectangleShape> shape;
		Color color;
	};

	/// @brief Simple text component.
	class TextComponent : public CommonComponent
	{
	public:
		/// @brief Registers all text specific properties.
		/// @param manager Resources manager.
		/// @param name Alias for type.
		static void registerProperties(Manager& manager, const std::string& name);

		/// @brief Sets text size.
		/// @param textSize Text size.
		void setTextSize(float textSize);
		/// @brief Sets text.
		/// @param text Text.
		void setText(const std::string& text);
		/// @brief Sets text color.
		/// @param color Color.
		void setTextColor(const Color& color);
		/// @brief Sets font.
		/// @param name Font.
		void setFont(const std::string& name);

		/// @brief Sets text alignment.
		/// @param horizontal Horizontal alignment.
		/// @param vertical Vertical alignment.
		void setTextAlignment(Gravity horizontal, Gravity vertical);
		/// @brief Returns horizontal text alignment.
		Gravity getHorizontalTextAlignment() const;
		/// @brief Returns vertical text alignment.
		Gravity getVerticalTextAlignment() const;

		virtual void onDraw(Canvas& canvas) override;

		virtual bool handleEvent(const Event& event) override;

		virtual std::pair<float, float> getContentSize(bool getWidth, bool getHeight) override;

		TextComponent();
		/// @brief Constructs text from xml.
		/// @param manager Resource manager.
		/// @param tag Xml representation.
		/// @param pack Styling information.
		TextComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack);
	private:
		std::shared_ptr<Resources::TextResource> textRes;
		std::string text, font;
		float textSize;
		Color color;
		Gravity horizontalTextAlign, verticalTextAlign;
	};

	/// @interface ButtonBase
	/// @brief Base for button components.
	class ButtonBase
	{
	public:
		/// @brief State of button.
		enum class ButtonState
		{
			Default,
			Hovered,
			Clicked
		};

		/// @brief Returns button state.
		ButtonState getButtonState() const noexcept;
		/// @brief Sets callback for click.
		/// @param callback Callback function.
		void setOnClickCallback(const std::function<void(Component&)>& callback);

		ButtonBase();
	protected:
		/// @brief Event handling callback.
		/// @note Should be called by every derived class when handling event.
		/// @param event Event.
		void handleClick(const Event& event);
		/// @brief Returns this component.
		/// 
		/// Required by handleClick.
		/// @return *this
		virtual Component& getThisComponent() = 0;
	private:
		std::function<void(Component&)> onClickCallback;
		ButtonState buttonState;
	};

	/// @brief Simple button implementation.
	class BasicButtonComponent : public TextComponent, public ButtonBase
	{
	public:
		/// @brief Registers all button specific properties.
		/// @param manager Resources manager.
		/// @param name Alias for type.
		static void registerProperties(Manager& manager, const std::string& name);

		/// @brief Returns drawable for given state.
		/// @param state State.
		std::shared_ptr<Resources::Drawable> getBackgroundDrawable(ButtonState state) const;
		/// @brief Returns drawable for current state.
		std::shared_ptr<Resources::Drawable> getCurrentBackgroundDrawable() const;

		virtual void onDraw(Canvas& canvas) override;

		virtual bool handleEvent(const Event& event) override;

		BasicButtonComponent() = default;
		/// @brief Constructs button from xml.
		/// @param manager Resource manager.
		/// @param tag Xml representation.
		/// @param pack Styling information.
		BasicButtonComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack);
	protected:
		virtual Component& getThisComponent() override;
	private:
		std::shared_ptr<Resources::Drawable> backgroundDefault, backgroundSelected, backgroundClicked;
	};
}
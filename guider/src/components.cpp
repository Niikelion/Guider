#include <guider/components.hpp>
#include <iostream>

namespace Guider
{
	Rect CommonComponent::getContentRect() const noexcept
	{
		return paddings.calcContentArea(getBounds());
	}

	void CommonComponent::registerProperties(Manager& manager, const std::string& name)
	{
		manager.registerPropertyForComponent<Padding>(name, "padding", [](const std::string& value) {
			std::vector<std::string> args = Styles::splitString(value);
			switch (args.size())
			{
			case 1:
			{
				bool failed = false;
				float v = Styles::strToFloat(args[0], failed);
				if (!failed)
					return Padding(v, v, v, v);
				break;
			}
			case 2:
			{
				bool failed = false;
				float w = Styles::strToFloat(args[0], failed);
				if (!failed)
				{
					float h = Styles::strToFloat(args[1], failed);
					if (!failed)
					{
						return Padding(w, w, h, h);
					}
				}
				break;
			}
			case 4:
			{
				bool failed = false;
				float left = Styles::strToFloat(args[0], failed);
				if (!failed)
				{
					float right = Styles::strToFloat(args[1], failed);
					if (!failed)
					{
						float top = Styles::strToFloat(args[2], failed);
						if (!failed)
						{
							float bottom = Styles::strToFloat(args[3], failed);
							if (!failed)
							{
								return Padding(left, right, top, bottom);
							}
						}
					}
				}
				break;
			}
			}
			throw std::invalid_argument("invalid padding value");
		});
	}

	void CommonComponent::setPadding(const Padding& pad)
	{
		paddings = pad;
	}

	Padding CommonComponent::getPading() const noexcept
	{
		return paddings;
	}

	std::pair<Component::DimensionDesc, Component::DimensionDesc> CommonComponent::measure(const DimensionDesc& width, const DimensionDesc& height)
	{
		std::pair<DimensionDesc, DimensionDesc> measurements = Component::measure(width, height);

		bool w = getSizingModeHorizontal() == SizingMode::WrapContent;
		bool h = getSizingModeVertical() == SizingMode::WrapContent;

		std::pair<float, float> contentSizes = getContentSize(w, h);

		if (w)
		{
			measurements.first.mode = DimensionDesc::Exact;
			measurements.first.value = contentSizes.first + paddings.left + paddings.right;
		}

		if (h)
		{
			measurements.second.mode = DimensionDesc::Exact;
			measurements.second.value = contentSizes.second + paddings.top + paddings.bottom;
		}

		return measurements;
	}

	CommonComponent::CommonComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack)
	{
		auto padding = pack.style.getAttribute("padding");
		if (padding)
		{
			setPadding(padding->as<Padding>());
		}
	}

	void EmptyComponent::registerProperties(Manager& manager, const std::string& name)
	{
		//yeah, thats it, nothing to see there
	}

	EmptyComponent::EmptyComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack)
	{
	}


	void RectangleShapeComponent::registerProperties(Manager& manager, const std::string& name)
	{
		CommonComponent::registerProperties(manager, name);
		manager.registerColorProperty(name, "fillColor");
	}

	void RectangleShapeComponent::setColor(const Color& c)
	{
		color = c;
		if (shape != nullptr)
		{
			shape->setColor(color);
			invalidateVisuals();
		}
	}

	Color RectangleShapeComponent::getColor() const noexcept
	{
		return color;
	}

	void RectangleShapeComponent::onDraw(Canvas& canvas)
	{
		Rect bounds = getBounds();
		shape->setSize(Vec2(bounds.width, bounds.height));
		shape->draw(canvas, Rect(0, 0, bounds.width, bounds.height));
	}
	bool RectangleShapeComponent::handleEvent(const Event& event)
	{
		bool r = Component::handleEvent(event);
		if (event.type == Event::BackendConnected)
		{
			shape = getBackend()->createRectangle(Vec2(), color);
		}
		return r;
	}
	RectangleShapeComponent::RectangleShapeComponent() : shape(nullptr), color(255, 255, 255, 255)
	{
	}
	RectangleShapeComponent::RectangleShapeComponent(SizingMode mode, const Color& c) : shape(nullptr), color(c)
	{
		setSizingMode(mode, mode);
	}
	RectangleShapeComponent::RectangleShapeComponent(float w, float h, const Color& c) : Component(), shape(nullptr), color(c)
	{
		setSize(Vec2(w, h));
		setSizingMode(SizingMode::OwnSize, SizingMode::OwnSize);
	}
	RectangleShapeComponent::RectangleShapeComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack) : RectangleShapeComponent()
	{
		Manager::handleDefaultArguments(*this, tag, pack.style);
		{
			auto color = pack.style.getAttribute("fillColor");
			if (color)
				setColor(color->as<Color>());
		}
	}
	void TextComponent::registerProperties(Manager& manager, const std::string& name)
	{
		CommonComponent::registerProperties(manager, name);
		manager.registerColorProperty(name, "color");
		manager.registerStringProperty(name, "text");
		manager.registerNumericProperty(name, "textSize");
	}
	void TextComponent::setTextSize(float textSize)
	{
		this->textSize = textSize;
		if (textRes != nullptr)
		{
			textRes->setTextSize(textSize);
		}
		if (getSizingModeHorizontal() == SizingMode::WrapContent || getSizingModeVertical() == SizingMode::WrapContent)
			invalidate();
		else
			invalidateVisuals();
	}
	void TextComponent::setText(const std::string& text)
	{
		this->text = text;
		if (textRes != nullptr)
		{
			textRes->setText(text);
		}
		if (getSizingModeHorizontal() == SizingMode::WrapContent)
			invalidate();
		else
			invalidateVisuals();
	}
	void TextComponent::setTextColor(const Color& color)
	{
		this->color = color;
		if (textRes != nullptr)
		{
			textRes->setColor(color);
		}
		invalidateVisuals();
	}
	void TextComponent::setFont(const std::string& name)
	{
		font = name;
		if (textRes != nullptr)
		{
			auto f = getBackend()->getFontByName(name);
			if (f)
				textRes->setFont(*f);
		}
		invalidate();
	}
	void TextComponent::setTextAlignment(Gravity horizontal, Gravity vertical)
	{
		horizontalTextAlign = horizontal;
		verticalTextAlign = vertical;

		invalidateVisuals();
	}
	Gravity TextComponent::getHorizontalTextAlignment() const
	{
		return horizontalTextAlign;
	}
	Gravity TextComponent::getVerticalTextAlignment() const
	{
		return verticalTextAlign;
	}
	void TextComponent::onDraw(Canvas& canvas)
	{
		Rect bounds = getBounds();
		Rect contentRect = getPading().calcContentArea(bounds.at(Vec2(0, 0)));
		textRes->horizontalAlignment = getHorizontalTextAlignment();
		textRes->verticalAlignment = getVerticalTextAlignment();

		textRes->draw(canvas, contentRect);
	}
	bool TextComponent::handleEvent(const Event& event)
	{
		bool r = Component::handleEvent(event);
		if (event.type == Event::BackendConnected)
		{
			textRes = getBackend()->createText(text, *getBackend()->getFontByName(font), textSize, color);
			textRes->setFont(*event.backendConnected.backend.getFontByName(font));
			textRes->setTextSize(textSize);
			textRes->setColor(color);
			textRes->setText(text);
		}
		return r;
	}
	std::pair<float, float> TextComponent::getContentSize(bool getWidth, bool getHeight)
	{
		float w = 0, h = 0;

		if (getWidth)
		{
			w = (textRes) ? textRes->getLineWidth() : 0;
		}
		if (getHeight)
		{
			h = (textRes) ? textRes->getLineHeight() : 0;
		}

		return std::pair<float, float>(w, h);
	}
	TextComponent::TextComponent() : textRes(nullptr), textSize(10), color(0xff), horizontalTextAlign(Gravity::Start), verticalTextAlign(Gravity::Center)
	{
	}
	TextComponent::TextComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack) : CommonComponent(manager, tag, pack), textRes(nullptr), textSize(10), color(0xff), horizontalTextAlign(Gravity::Start), verticalTextAlign(Gravity::Center)
	{
		Manager::handleDefaultArguments(*this, tag, pack.style);
		setBackend(manager.getBackend());

		{
			auto color = pack.style.getAttribute("color");
			if (color)
				setTextColor(color->as<Color>());
		}

		{
			auto text = pack.style.getAttribute("text");
			if (text)
				setText(text->as<std::string>());
		}

		{
			auto textSize = pack.style.getAttribute("textSize");
			if (textSize)
				setTextSize(textSize->as<float>());
		}

		auto tmp = tag.getAttribute("font");
		if (tmp.exists())
		{
			auto font = manager.getFontByName(tmp.val);
			if (font)
			{
				setFont(tmp.val);
			}
		}

		Gravity hor = Gravity::Center, ver = Gravity::Center;
		//TODO: move to style approach
		tmp = tag.getAttribute("textAlignmentHorizontal");
		if (tmp.exists())
		{
			if (tmp.val == "left" || tmp.val == "start")
				hor = Gravity::Left;
			else if (tmp.val == "center" || tmp.val == "middle")
				hor = Gravity::Center;
			else if (tmp.val == "right" || tmp.val == "end")
				hor = Gravity::Right;
		}
		//TODO: move to style approach
		tmp = tag.getAttribute("textAlignmentVertical");
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
	void ButtonBase::handleClick(const Event& event)
	{
		switch (event.type)
		{
		case Event::Type::MouseButtonDown:
		{
			if (event.mouseEvent.button == 0)
			{
				if (onClickCallback)
					onClickCallback(getThisComponent());
				buttonState = ButtonState::Clicked;
				getThisComponent().invalidateVisuals();
			}
			break;
		}
		case Event::Type::MouseButtonUp:
		{
			if (event.mouseEvent.button == 0)
			{
				ButtonState p = buttonState;
				if (getThisComponent().getBounds().at(Vec2(0, 0)).contains(Vec2(event.mouseEvent.x, event.mouseEvent.y)))
					buttonState = ButtonState::Hovered;
				else
					buttonState = ButtonState::Default;
				if (p != buttonState)
					getThisComponent().invalidateVisuals();
			}
			break;
		}
		case Event::Type::MouseLeft:
		{
			if (buttonState == ButtonState::Hovered)
			{
				buttonState = ButtonState::Default;
				getThisComponent().invalidateVisuals();
			}
			break;
		}

		case Event::Type::MouseMoved:
		{
			if (buttonState != ButtonState::Clicked)
			{
				if (buttonState != ButtonState::Hovered)
				{
					buttonState = ButtonState::Hovered;
					getThisComponent().invalidateVisuals();
				}
			}
			break;
		}
		}
	}
	ButtonBase::ButtonState ButtonBase::getButtonState() const noexcept
	{
		return buttonState;
	}
	void ButtonBase::setOnClickCallback(const std::function<void(Component&)>& callback)
	{
		onClickCallback = callback;
	}
	ButtonBase::ButtonBase() : buttonState(ButtonState::Default)
	{
	}
	BasicButtonComponent::BasicButtonComponent(Manager& manager, const XML::Tag& tag, const StylingPack& pack) : TextComponent(manager, tag, pack)
	{
		auto backgroundP = pack.style.getAttribute("background");
		if (backgroundP)
			backgroundDefault = backgroundP->as<std::shared_ptr<Resources::Drawable>>();
		auto selectedBackgroundP = pack.style.getAttribute("selectedBackground");
		if (selectedBackgroundP)
			backgroundClicked = selectedBackgroundP->as<std::shared_ptr<Resources::Drawable>>();
		auto hoveredBackgroundP = pack.style.getAttribute("hoveredBackground");
		if (hoveredBackgroundP)
			backgroundSelected = hoveredBackgroundP->as<std::shared_ptr<Resources::Drawable>>();
	}
	Component& BasicButtonComponent::getThisComponent()
	{
		return *this;
	}
	void BasicButtonComponent::registerProperties(Manager& manager, const std::string& name)
	{
		TextComponent::registerProperties(manager, name);
		manager.registerDrawableProperty(name, "selectedBackground");
		manager.registerDrawableProperty(name, "hoveredBackground");
	}
	std::shared_ptr<Resources::Drawable> BasicButtonComponent::getBackgroundDrawable(ButtonState state) const
	{
		if (state == ButtonState::Clicked)
		{
			return backgroundClicked;
		}
		else if (state == ButtonState::Hovered)
		{
			return backgroundSelected;
		}

		return backgroundDefault;
	}
	std::shared_ptr<Resources::Drawable> BasicButtonComponent::getCurrentBackgroundDrawable() const
	{
		return getBackgroundDrawable(getButtonState());
	}
	void BasicButtonComponent::onDraw(Canvas& canvas)
	{
		Rect bounds = getBounds();
		std::shared_ptr<Resources::Drawable> shape = getCurrentBackgroundDrawable();

		if (!shape)
			shape = getBackgroundDrawable(ButtonState::Default);
		if (shape)
			shape->draw(canvas, bounds.at(Vec2(0, 0)));
		TextComponent::onDraw(canvas);
	}
	bool BasicButtonComponent::handleEvent(const Event& event)
	{
		bool r = TextComponent::handleEvent(event);
		handleClick(event);
		if (event.type == Event::Type::BackendConnected)
		{
			if (!backgroundDefault)
				backgroundDefault = getBackend()->createRectangle(Vec2(0, 0), Color(255, 255, 255));
		}
		return r;
	}
}
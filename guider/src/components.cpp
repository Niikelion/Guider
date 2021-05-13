#include <guider/components.hpp>
#include <iostream>

namespace Guider
{
	Rect CommonComponent::getContentRect() const noexcept
	{
		return paddings.calcContentArea(getBounds());
	}

	void CommonComponent::registerProperties(Manager& m, const std::string& name)
	{
		m.registerPropertyForComponent<Padding>(name, "padding", [](const std::string& value) {
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

	CommonComponent::CommonComponent(Manager& m, const XML::Tag& tag, const StylingPack& pack)
	{
		{
			auto padding = pack.style.getAttribute("padding");
			if (padding)
			{
				setPadding(padding->as<Padding>());
			}
		}
	}

	void RectangleShapeComponent::setColor(const Color& c)
	{
		color = c;
		if (shape != nullptr)
		{
			shape->setColor(color);
		}
	}

	void RectangleShapeComponent::onDraw(Canvas& canvas)
	{
		Rect bounds = getBounds();
		shape->setSize(Vec2(bounds.width, bounds.height));
		shape->draw(canvas, Rect(0, 0, bounds.width, bounds.height));
	}
	void RectangleShapeComponent::handleEvent(const Event& event)
	{
		Component::handleEvent(event);
		if (event.type == Event::BackendConnected)
		{
			shape = getBackend()->createRectangle(Vec2(), color);
		}
	}
	void TextComponent::registerProperties(Manager& m, const std::string& name)
	{
		CommonComponent::registerProperties(m, name);
		m.registerColorProperty(name, "color");
		m.registerStringProperty(name, "text");
	}
	void TextComponent::setTextSize(float textSize)
	{
		this->textSize = textSize;
		if (textRes != nullptr)
		{
			textRes->setTextSize(textSize);
		}
		invalidate();
	}
	void TextComponent::setText(const std::string& text)
	{
		this->text = text;
		if (textRes != nullptr)
		{
			textRes->setText(text);
		}
		invalidate();
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
	void TextComponent::onDraw(Canvas& canvas)
	{
		//float xoff = 0, yoff = 0;

		Rect bounds = getBounds();
		Rect contentRect = getPading().calcContentArea(bounds.at(Vec2(0, 0)));
		textRes->horizontalAlignment = getHorizontalTextAlignment();
		textRes->verticalAlignment = getVerticalTextAlignment();

		textRes->draw(canvas, contentRect);
	}
	void TextComponent::handleEvent(const Event& event)
	{
		Component::handleEvent(event);
		if (event.type == Event::BackendConnected)
		{
			textRes = getBackend()->createText(text, *getBackend()->getFontByName(font), textSize, color);
			textRes->setFont(*event.backendConnected.backend.getFontByName(font));
			textRes->setTextSize(textSize);
			textRes->setColor(color);
			textRes->setText(text);
		}
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
	void ButtonBase::setOnClickCallback(const std::function<void(Component&)>& callback)
	{
		onClickCallback = callback;
	}
	Component& BasicButtonComponent::getThisComponent()
	{
		return *this;
	}
	void BasicButtonComponent::registerProperties(Manager& m, const std::string& name)
	{
		TextComponent::registerProperties(m, name);
		m.registerDrawableProperty(name, "selectedBackground");
		m.registerDrawableProperty(name, "hoveredBackground");
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
	void BasicButtonComponent::handleEvent(const Event& event)
	{
		handleClick(event);
		TextComponent::handleEvent(event);
		switch (event.type)
		{
		case Event::Type::BackendConnected:
		{
			if (!backgroundDefault)
				backgroundDefault = getBackend()->createRectangle(Vec2(0, 0), Color(255, 255, 255));
			break;
		}
		}
	}
}
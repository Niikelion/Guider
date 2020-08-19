#include <guider/components.hpp>
#include <iostream>

namespace Guider
{
	Rect Padding::calcContentArea(const Rect& bounds) const
	{
		Rect ret = Rect(bounds.left+left,bounds.top+top,bounds.width-left-right,bounds.height-top-bottom);

		if (ret.width < 0)
		{
			ret.width = 0;
			ret.left = (2 * bounds.left + left + bounds.width - right) / 2;
		}

		if (ret.height < 0)
		{
			ret.height = 0;
			ret.top = (2 * bounds.top + top + bounds.height - bottom) / 2;
		}

		return ret;
	}

	Rect CommonComponent::getContentRect() const noexcept
	{
		return paddings.calcContentArea(getBounds());
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

		std::pair<float, float> contentSizes = getContentSize(w,h);

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

	CommonComponent::CommonComponent(Manager& m, const XML::Tag& tag)
	{
		XML::Value tmp = tag.getAttribute("padding");
		if (tmp.exists())
		{
			std::vector<std::string> args = Manager::splitString(tmp.val);
			switch (args.size())
			{
			case 1:
			{
				bool failed = false;
				float v = Manager::strToFloat(args[0], failed);
				if (!failed)
					setPadding(Padding(v, v, v, v));
				break;
			}
			case 2:
			{
				bool failed = false;
				float w = Manager::strToFloat(args[0], failed);
				if (!failed)
				{
					float h = Manager::strToFloat(args[1], failed);
					if (!failed)
					{
						setPadding(Padding(w, w, h, h));
					}
				}
				break;
			}
			case 4:
			{
				bool failed = false;
				float left = Manager::strToFloat(args[0],failed);
				if (!failed)
				{
					float right = Manager::strToFloat(args[1],failed);
					if (!failed)
					{
						float top = Manager::strToFloat(args[2],failed);
						if (!failed)
						{
							float bottom = Manager::strToFloat(args[3],failed);
							if (!failed)
							{
								setPadding(Padding(left,right,top,bottom));
							}
						}
					}
				}
				break;
			}
			}
		}
	}

	void EmptyComponent::onDraw(Backend& renderer) const
	{
	}

	EmptyComponent::EmptyComponent(Manager& m, const XML::Tag& config)
	{
	}

	void RectangleShape::setColor(const Color& c)
	{
		color = c;
		if (shape != nullptr)
		{
			shape->setColor(color);
		}
	}

	void RectangleShape::onDraw(Backend& renderer) const
	{
		Rect bounds = getBounds();
		shape->setSize(Vec2(bounds.width,bounds.height));
		shape->draw(Vec2(0,0));
	}
	void RectangleShape::handleEvent(const Event& event)
	{
		if (event.type == Event::BackendConnected)
		{
			if (shape != nullptr)
			{
				getBackend()->deleteResource(shape->getId());
			}
			auto r = event.backendConnected.backend.createRectangle(Vec2(),color);
			shape = r.get();
		}
		Component::handleEvent(event);
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
	void TextComponent::setTextAlignment(TextAlignment horizontal, TextAlignment vertical)
	{
		horizontalTextAlign = horizontal;
		verticalTextAlign = vertical;
		invalidateVisuals();
	}
	void TextComponent::onDraw(Backend& renderer) const
	{
		float xoff = 0, yoff = 0;

		Rect bounds = getBounds();
		Rect contentRect = getPading().calcContentArea(bounds.at(Vec2(0,0)));

		float width = textRes->getLineWidth();
		float height = textRes->getLineHeight();

		switch (getHorizontalTextAlignment())
		{
		case TextAlignment::Start:
		{
			xoff = contentRect.left;
			break;
		}
		case TextAlignment::Center:
		{
			xoff = contentRect.left + (contentRect.width - width) / 2;
			break;
		}
		case TextAlignment::End:
		{
			xoff = contentRect.left + contentRect.width - width;
			break;
		}
		}

		switch (getVerticalTextAlignment())
		{
		case TextAlignment::Start:
		{
			yoff = contentRect.top;
			break;
		}
		case TextAlignment::Center:
		{
			yoff = contentRect.top + (contentRect.height - height) / 2;
			break;
		}
		case TextAlignment::End:
		{
			yoff = contentRect.top + contentRect.height - height;
			break;
		}
		}

		textRes->draw(Vec2(xoff,yoff));
	}
	void TextComponent::handleEvent(const Event& event)
	{
		if (event.type == Event::BackendConnected)
		{
			if (textRes != nullptr)
			{
				getBackend()->deleteResource(textRes->getId());
			}
			textRes = event.backendConnected.backend.createText(text,*event.backendConnected.backend.getFontByName(font), textSize, color);
			textRes->setFont(*event.backendConnected.backend.getFontByName(font));
			textRes->setTextSize(textSize);
			textRes->setColor(color);
			textRes->setText(text);
		}
		Component::handleEvent(event);
	}
	std::pair<float, float> TextComponent::getContentSize(bool getWidth, bool getHeight)
	{
		Backend* backend = getBackend();

		float w = 0, h = 0;

		if (backend != nullptr)
		{
			if (getWidth)
			{
				w = (textRes != nullptr) ? textRes->getLineWidth() : 0;
			}
			if (getHeight)
			{
				h = (textRes != nullptr) ? textRes->getLineHeight() : 0;
			}
		}

		return std::pair<float, float>(w,h);
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
				getThisComponent().invalidateVisuals();
				buttonState = ButtonState::Clicked;
			}
			break;
		}
		case Event::Type::MouseButtonUp:
		{
			if (event.mouseEvent.button == 0)
			{
				
				if (getThisComponent().getBounds().at(Vec2(0,0)).contains(Vec2(event.mouseEvent.x, event.mouseEvent.y)))
					buttonState = ButtonState::Hovered;
				else
					buttonState = ButtonState::Default;
				getThisComponent().invalidateVisuals();
			}
			break;
		}
		case Event::Type::MouseLeft:
		{
			buttonState = ButtonState::Default;
			getThisComponent().invalidateVisuals();
			break;
		}

		case Event::Type::MouseMoved:
		{
			if (buttonState != ButtonState::Clicked)
				buttonState = ButtonState::Hovered;
			getThisComponent().invalidateVisuals();
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
	void BasicButtonComponent::drawMask(Backend& b) const
	{
		Component::drawMask(b);
		std::cout << "drawButtonMask\n";
	}
	std::shared_ptr<Backend::RectangleShape> BasicButtonComponent::getBackgroundDrawable(ButtonState state) const
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
	void BasicButtonComponent::onDraw(Backend& backend) const
	{
		Rect bounds = getBounds();
		std::shared_ptr<Backend::RectangleShape> shape = getCurrentBackgroundDrawable();
		shape->setSize(Vec2(bounds.width,bounds.height));
		shape->draw(Vec2(0,0));
		TextComponent::onDraw(backend);
	}
	void BasicButtonComponent::handleEvent(const Event& event)
	{
		handleClick(event);
		switch (event.type)
		{
		case Event::Type::BackendConnected:
		{
			Backend& backend = event.backendConnected.backend;
			backgroundDefault = backend.createRectangle(Vec2(0, 0), Color(0x252525ff)); //0x252525ff
			backgroundClicked = backend.createRectangle(Vec2(0, 0), Color(0x3f3f3fff)); //0x3f3f3fff
			backgroundSelected = backend.createRectangle(Vec2(0,0), Color(0x303030ff)); //0x2f2f2fff
			break;
		}
		}
		TextComponent::handleEvent(event);
	}
}
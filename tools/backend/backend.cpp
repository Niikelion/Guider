#include "..\..\guider\include\guider\base.hpp"
#include <backend.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>

namespace Guider
{
	void SfmlCanvas::drawRectangle(const Rect& rect, const Color& color)
	{
		rectangle.setSize(sf::Vector2f(rect.width, rect.height));
		rectangle.setPosition(rect.left, rect.top);
		rectangle.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
		target.draw(rectangle);
	}

	namespace SFMLResources
	{
		void ImageResource::draw(Canvas& canvas, const Rect& bounds)
		{
			sprite.setScale(bounds.width/width,bounds.height/height);
			canvas.as<SfmlCanvas>().getTarget().draw(sprite);
		}
		ImageResource::ImageResource(const std::string& path)
		{
			texture.loadFromFile(path);
			sf::Vector2u size = texture.getSize();
			width = size.x;
			height = size.y;
			sprite.setTexture(texture, true);
		}
		Guider::Resources::ImageResource& ImageCanvas::getImage()
		{
			return imageResourceWrapper;
		}
		void ImageCanvas::drawRectangle(const Rect& rect, const Color& color)
		{
			SfmlCanvas::drawRectangle(rect, color);
		}
		void ConstImageResource::draw(Canvas& canvas, const Rect& rect)
		{
			sf::Vector2u size = texture.getSize();
			sprite.setTexture(texture,true);
			sprite.setScale(rect.width / size.x, rect.height / size.y);
		}
		void RectangleShape::draw(Canvas& canvas, const Rect& bounds)
		{
			bool resetSize = false;
			rectangle.setPosition(bounds.left, bounds.top);
			if (rectangle.getSize() == sf::Vector2f(0, 0))
			{
				resetSize = true;
				rectangle.setSize(sf::Vector2f(bounds.width,bounds.height));
			}
			canvas.as<SfmlCanvas>().getTarget().draw(rectangle);
			if (resetSize)
			{
				rectangle.setSize(sf::Vector2f(0,0));
			}
		}
		void RectangleShape::setSize(const Vec2& size)
		{
			rectangle.setSize(sf::Vector2f(size.x,size.y));
		}
		void RectangleShape::setColor(const Color& color)
		{
			rectangle.setFillColor(sf::Color(color.r,color.g,color.b,color.a));
		}
		std::pair<unsigned, float> FontResource::unpackTextSize(float textSize)
		{
			unsigned s = std::ceil(textSize);
			return std::pair<unsigned, float>(s,textSize/s);
		}
		float FontResource::getLineHeight(float textSize) const
		{
			std::pair<unsigned, float> p = unpackTextSize(textSize);
			return font.getLineSpacing(p.first) * p.second;
		}
		float FontResource::getLineWidth(float textSize, const std::string& text) const
		{
			std::pair<unsigned, float> p = unpackTextSize(textSize);
			sf::Text t;
			t.setCharacterSize(p.first);
			t.setString(text);
			t.setFont(font);
			return t.getLocalBounds().width * p.second;
		}
		void TextResource::draw(Canvas& canvas, const Rect& bounds)
		{
			Rect adjustedRect = getAdjustedRect(bounds);
			text.setPosition(adjustedRect.left, adjustedRect.top);
			canvas.as<SfmlCanvas>().getTarget().draw(text);
		}
		void TextResource::setText(const std::string& text)
		{
			this->text.setString(text);
		}
		void TextResource::setTextSize(float size)
		{
			textSize = size;
			auto fontSize = SFMLResources::FontResource::unpackTextSize(size);
			text.setCharacterSize(fontSize.first);
			text.setScale(fontSize.second, fontSize.second);
		}
		void TextResource::setFont(const Guider::Resources::FontResource& font)
		{
			const SFMLResources::FontResource* f = dynamic_cast<const SFMLResources::FontResource*>(&font);
			if (f != nullptr)
			{
				text.setFont(f->getFont());
				fontResource = f;
			}
		}
		void TextResource::setColor(const Color& color)
		{
			text.setFillColor(sf::Color(color.r,color.g,color.b,color.a));
		}
		float TextResource::getLineHeight() const
		{
			if (fontResource != nullptr)
			{
				return fontResource->getLineHeight(textSize);
			}
			return 0;
		}
		float TextResource::getLineWidth() const
		{
			if (fontResource != nullptr)
			{
				return fontResource->getLineWidth(textSize,text.getString());
			}
			return 0;
		}
	}

	void SfmlBackend::setDrawOrigin(float x, float y)
	{
		sf::Vector2u size = canvas->getTarget().getSize();
		origin = sf::Vector2f(x, y);
		view.setCenter(-origin+sf::Vector2f(size.x,size.y)*0.5f);
		view.setViewport(sf::FloatRect(0, 0, 1, 1));
		
		view.setSize(sf::Vector2f(size.x,size.y));
		canvas->getTarget().setView(view);
	}

	void SfmlBackend::clearMask()
	{
		canvas->getTarget().pushGLStates();

		glEnable(GL_STENCIL_TEST);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
	}
	void SfmlBackend::setupMask()
	{
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilMask(0xFF);
	}
	void SfmlBackend::useMask()
	{
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilMask(0x00);
	}
	void SfmlBackend::disableMask()
	{
		canvas->getTarget().popGLStates();
	}
	void SfmlBackend::pushMaskLayer()
	{
		maskLevel++;
		glStencilFunc(GL_GEQUAL, maskLevel, 0xFF);
	}
	void SfmlBackend::popMaskLayer()
	{
		maskLevel--;
		glStencilFunc(GL_GEQUAL, maskLevel, 0xFF);
	}
	void SfmlBackend::addToMask(const Rect& rect)
	{
		glColorMask(false, false, false, false);
		maskRect.setSize(Vec2(0,0));
		maskRect.draw(*canvas,Rect(rect.left, rect.top, rect.width, rect.height));
		glColorMask(true, true, true, true);
	}

	Vec2 SfmlBackend::getSize() const noexcept
	{
		sf::Vector2u size = canvas->getTarget().getSize();
		return Vec2((float)size.x, (float)size.y);
	}
	void SfmlBackend::setSize(const Vec2& size)
	{
		//sfml does not have api for resizing sf::RenderTarget
	}

	void SfmlBackend::setBounds(const Rect& rect)
	{
		glScissor(origin.x+rect.left,origin.y+rect.top,rect.width,rect.height);
	}

	std::shared_ptr<Resources::RectangleShape> SfmlBackend::createRectangle(const Vec2& size, const Color& color)
	{
		std::shared_ptr<Resources::RectangleShape> ret = std::make_shared<SFMLResources::RectangleShape>(size.x,size.y);
		ret->setColor(color);
		return ret;
	}
	std::shared_ptr<Resources::TextResource> SfmlBackend::createText(const std::string& text, const Resources::FontResource& font, float size, const Color& color)
	{
		std::shared_ptr<Resources::TextResource> res = std::make_shared<SFMLResources::TextResource>();
		res->setText(text);
		res->setFont(font);
		res->setTextSize(size);
		res->setColor(color);

		return res;
	}
	std::shared_ptr<Resources::FontResource> SfmlBackend::getFontByName(const std::string& name)
	{
		auto it = fonts.find(name);
		if (it != fonts.end())
		{
			return it->second;
		}
		return std::shared_ptr<Resources::FontResource>();
	}
	std::shared_ptr<Resources::FontResource> SfmlBackend::loadFontFromFile(const std::string& filename, const std::string& name)
	{
		sf::Font f;
		if (f.loadFromFile(filename))
		{
			std::shared_ptr<SFMLResources::FontResource> font = std::make_shared<SFMLResources::FontResource>(f,name);
			fonts[name] = font;
			return font;
		}
		return std::shared_ptr<Resources::FontResource>();
	}
	std::shared_ptr<Resources::ImageResource> SfmlBackend::loadImageFromFile(const std::string& filename)
	{
		return std::make_shared<SFMLResources::ImageResource>(filename);
	}
	std::shared_ptr<Resources::ImageCanvas> SfmlBackend::createImage(const Vec2& size)
	{
		return std::make_shared<SFMLResources::ImageCanvas>();
	}
	void SfmlBackend::deleteResource(Resources::Resource& resource) {}

	std::shared_ptr<Canvas> SfmlBackend::getCanvas()
	{
		return std::static_pointer_cast<Canvas>(canvas);
	}
}

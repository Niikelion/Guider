#include <backend.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>

namespace Guider
{
	void SfmlBackend::SfmlRectangle::setSize(const Vec2& size)
	{
		shape.setSize(sf::Vector2f(size.x, size.y));
	}

	void SfmlBackend::SfmlRectangle::setColor(const Color& color)
	{
		shape.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
	}

	void SfmlBackend::SfmlRectangle::draw(const Vec2& offset)
	{
		sf::Vector2f origin = backend.getCurrentDrawingoffset();
		shape.setPosition(offset.x + origin.x, offset.y + origin.y);
		backend.target.draw(shape);
	}

	std::pair<unsigned, float> SfmlBackend::SfmlFont::unpackTextSize(float textSize)
	{
		float c = std::ceil(textSize);
		return std::pair<unsigned, float>(c,textSize/c);
	}

	float SfmlBackend::SfmlFont::getLineHeight(float textSize) const
	{
		std::pair<unsigned, float> p = unpackTextSize(textSize);
		return font.getLineSpacing(p.first)*p.second;
	}

	float SfmlBackend::SfmlFont::getLineWidth(float textSize, const std::string& text) const
	{
		std::pair<unsigned, float> p = unpackTextSize(textSize);
		sf::Text t;
		t.setCharacterSize(p.first);
		t.setString(text);
		t.setFont(font);
		return t.getLocalBounds().width * p.second;
	}

	void SfmlBackend::SfmlText::setText(const std::string& text)
	{
		this->text.setString(text);
	}

	void SfmlBackend::SfmlText::setTextSize(float size)
	{
		this->size = size;
		std::pair<unsigned, float> p = SfmlFont::unpackTextSize(size);
		text.setCharacterSize(p.first);
		text.setScale(p.second, p.second);
	}

	void SfmlBackend::SfmlText::setFont(const FontResource& font)
	{
		this->font = &font;
		text.setFont(static_cast<const SfmlFont&>(font).getFont());
	}

	void SfmlBackend::SfmlText::setColor(const Color& color)
	{
		text.setFillColor(sf::Color(color.r,color.g,color.b));
	}

	float SfmlBackend::SfmlText::getLineHeight() const
	{
		if (font != nullptr)
			return font->getLineHeight(size);
		return 0.0f;
	}

	float SfmlBackend::SfmlText::getLineWidth() const
	{
		return text.getLocalBounds().width;
	}

	void SfmlBackend::SfmlText::draw(const Vec2& offset)
	{
		sf::Vector2f origin = backend.getCurrentDrawingoffset();
		text.setPosition(offset.x + origin.x, offset.y + origin.y);
		backend.target.draw(text);
	}

	void SfmlBackend::setViewport(const Rect& rect)
	{
		Vec2 size = getSize();
		sf::FloatRect bounds(rect.left / size.x, rect.top / size.y, rect.width / size.x, rect.height / size.y);
		sf::View v = target.getView();

		v.setViewport(bounds);
		v.setSize(rect.width, rect.height);
		v.setCenter(rect.width / 2, rect.height / 2);
		target.setView(v);
	}
	void SfmlBackend::addResource(const std::shared_ptr<Resource>& resource)
	{
		resources.emplace(resource->getId(), resource);
	}
	sf::Vector2f SfmlBackend::getCurrentDrawingoffset() const noexcept
	{
		return origin;
	}
	std::shared_ptr<SfmlBackend::RectangleShape> SfmlBackend::createRectangle(const Vec2& size, const Color& color)
	{
		std::shared_ptr<SfmlRectangle> ret = std::make_shared<SfmlRectangle>(*this,lid);
		++lid;
		ret->setSize(size);
		ret->setColor(color);
		addResource(ret);
		return ret;
	}

	std::shared_ptr<SfmlBackend::TextResource> SfmlBackend::createText(const std::string& text, const FontResource& font, float size, const Color& color)
	{
		std::shared_ptr<SfmlText> ret = std::make_shared<SfmlText>(*this,lid);
		++lid;
		ret->setFont(font);
		ret->setTextSize(size);
		ret->setColor(color);
		addResource(ret);
		return ret;
	}

	std::shared_ptr<SfmlBackend::FontResource> SfmlBackend::loadFontFromFile(const std::string& file, const std::string& name)
	{
		sf::Font font;
		font.loadFromFile(file);
		std::shared_ptr<SfmlFont> ret = std::make_shared<SfmlFont>(font,name,lid);
		++lid;
		addResource(ret);
		return ret;
	}

	std::shared_ptr<Backend::FontResource> SfmlBackend::getFontByName(const std::string& name)
	{
		auto it = fonts.find(name);
		if (it != fonts.end())
		{
			return it->second;
		}
		return std::shared_ptr<FontResource>();
	}

	void SfmlBackend::deleteResource(Resource& resource)
	{
		deleteResource(resource.getId());
	}

	void SfmlBackend::deleteResource(uint64_t id)
	{
		auto it = resources.find(id);
		if (it != resources.end())
		{
			resources.erase(it);
			SfmlFont* font = dynamic_cast<SfmlFont*>(it->second.get());
			if (font != nullptr)
			{
				fonts.erase(fonts.find(font->getName()));
			}
		}
	}

	void SfmlBackend::setDrawOrigin(float x, float y)
	{
		origin = sf::Vector2f(x, y);
	}
	void SfmlBackend::clearMask()
	{
		target.pushGLStates();

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
		target.popGLStates();
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
		maskRect.setSize(Vec2(rect.width, rect.height));
		maskRect.draw(Vec2(rect.left, rect.top));
		glColorMask(true, true, true, true);
	}

	Vec2 SfmlBackend::getSize() const noexcept
	{
		sf::Vector2u size = target.getSize();
		return Vec2((float)size.x, (float)size.y);
	}
	void SfmlBackend::setSize(const Vec2& size)
	{
		target.create((unsigned)size.x, (unsigned)size.y, sf::ContextSettings(0, 8));
	}
	void SfmlBackend::loadFont(const std::string& name, const sf::Font& font)
	{
		fonts.emplace(name, std::make_shared<SfmlFont>(font,name,lid));
		++lid;
	}
}
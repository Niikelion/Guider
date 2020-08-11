#include <backend.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>

namespace Guider
{
	void SfmlRenderer::setViewport(const Rect& rect)
	{
		Vec2 size = getSize();
		sf::FloatRect bounds(rect.left / size.x, rect.top / size.y, rect.width / size.x, rect.height / size.y);
		sf::View v = target.getView();

		v.setViewport(bounds);
		v.setSize(rect.width, rect.height);
		v.setCenter(rect.width / 2, rect.height / 2);
		target.setView(v);
	}
	void SfmlRenderer::drawRectangle(const Rect& rect)
	{
		rectangle.setFillColor(color);
		rectangle.setSize(sf::Vector2f(rect.width, rect.height));
		rectangle.setPosition(origin.x + rect.left, origin.y + rect.top);
		target.draw(rectangle);
	}
	void SfmlRenderer::drawText(float x, float y, const String& text)
	{
		this->text.setFillColor(color);
		this->text.setScale(sf::Vector2f(textScale, textScale));
		this->text.setCharacterSize(textSize);
		this->text.setPosition(origin.x+x,origin.y+y);
		this->text.setString(text);

		target.draw(this->text);
	}
	void SfmlRenderer::setDrawOrigin(float x, float y)
	{
		origin = sf::Vector2f(x, y);
	}
	void SfmlRenderer::clearMask()
	{
		target.pushGLStates();

		glEnable(GL_STENCIL_TEST);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
	}
	void SfmlRenderer::setupMask()
	{
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilMask(0xFF);
	}
	void SfmlRenderer::useMask()
	{
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilMask(0x00);
	}
	void SfmlRenderer::disableMask()
	{
		target.popGLStates();
	}
	void SfmlRenderer::pushMaskLayer()
	{
		maskLevel++;
		glStencilFunc(GL_GEQUAL, maskLevel, 0xFF);
	}
	void SfmlRenderer::popMaskLayer()
	{
		maskLevel--;
		glStencilFunc(GL_GEQUAL, maskLevel, 0xFF);
	}
	void SfmlRenderer::addToMask(const Rect& rect)
	{
		glColorMask(false, false, false, false);
		drawRectangle(rect);
		glColorMask(true, true, true, true);
	}
	void SfmlRenderer::setColor(const Color& color)
	{
		this->color = sf::Color(color.hex());
	}
	void SfmlRenderer::setTextSize(float size)
	{
		textSize = std::ceil(size);
		textScale = size / textSize;
	}
	void SfmlRenderer::setFont(const std::string& font)
	{
		if (fonts.count(font))
		{
			text.setFont(fonts.at(font));
		}
	}
	Vec2 SfmlRenderer::getSize() const noexcept
	{
		sf::Vector2u size = target.getSize();
		return Vec2((float)size.x, (float)size.y);
	}
	void SfmlRenderer::setSize(const Vec2& size)
	{
		target.create((unsigned)size.x, (unsigned)size.y, sf::ContextSettings(0, 8));
	}
	void SfmlRenderer::loadFont(const std::string& name, const sf::Font& font)
	{
		fonts.emplace(name, font);
	}
}
#pragma once

#include <guider/base.hpp>
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace Guider
{
	class SfmlRenderer : public RenderBackend
	{
	private:
		sf::Vector2f origin;
		sf::Color color;
		sf::RectangleShape rectangle;
		sf::Text text;
		unsigned char maskLevel;
		std::unordered_map<std::string, sf::Font> fonts;
		float textScale;
		int textSize;
	protected:
		virtual void setViewport(const Rect& rect) override;
	public:
		sf::RenderTexture target;
		virtual void drawRectangle(const Rect& rect) override;
		virtual void drawText(float x, float y, const String& text) override;

		virtual void setDrawOrigin(float x, float y) override;

		virtual void clearMask() override;
		virtual void setupMask() override;
		virtual void useMask() override;
		virtual void disableMask() override;
		virtual void pushMaskLayer() override;
		virtual void popMaskLayer() override;
		virtual void addToMask(const Rect& rect) override;
		virtual void setColor(const Color& color) override;
		virtual void setTextSize(float size) override;
		virtual void setFont(const std::string& font) override;

		virtual Vec2 getSize() const noexcept override;
		virtual void setSize(const Vec2& size) override;

		void loadFont(const std::string& name, const sf::Font& font);

		SfmlRenderer() : origin(0, 0), maskLevel(1), textScale(1), textSize(12) {}
	};
}
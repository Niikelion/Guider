#pragma once

#include <guider/base.hpp>
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace Guider
{
	class SfmlCanvas : public Canvas
	{
	public:
		inline sf::RenderTarget& getTarget()
		{
			return target;
		}

		virtual void drawRectangle(const Rect& rect, const Color& color) override;

		SfmlCanvas(sf::RenderTarget& t) : target(t) {}
	private:
		sf::RenderTarget& target;
		sf::RectangleShape rectangle;
	};


	namespace SFMLResources
	{
		class ImageResource : public Guider::Resources::ImageResource
		{
		public:
			virtual void draw(Canvas& canvas, const Rect& bounds) override;

			ImageResource() = default;
			ImageResource(const std::string& path);
		private:
			sf::Sprite sprite;
			sf::Texture texture;
		};

		class ConstImageResource : public Guider::Resources::ImageResource
		{
		public:
			virtual void draw(Canvas& canvas, const Rect& rect) override;

			ConstImageResource(const sf::Texture& tex) : texture(tex) {}
		private:
			sf::Sprite sprite;
			const sf::Texture& texture;
		};

		class ImageCanvas : public Guider::Resources::ImageCanvas, public SfmlCanvas
		{
		public:
			virtual Guider::Resources::ImageResource& getImage() override;

			virtual void drawRectangle(const Rect& rect, const Color& color) override;

			ImageCanvas() : SfmlCanvas(texture), imageResourceWrapper(texture.getTexture()) {}
			ImageCanvas(uint32_t width,uint32_t height) : SfmlCanvas(texture), imageResourceWrapper(texture.getTexture())
			{
				texture.create(width, height, sf::ContextSettings(0, 8));
			}
		private:
			ConstImageResource imageResourceWrapper;
			sf::RenderTexture texture;

			// Odziedziczono za poœrednictwem elementu ImageCanvas
		};

		class RectangleShape : public Guider::Resources::RectangleShape
		{
		public:
			virtual void draw(Canvas& canvas, const Rect& bounds) override;

			virtual void setSize(const Vec2& size) override;

			virtual void setColor(const Color& color) override;

			RectangleShape() = default;
			RectangleShape(float width,float height)
			{
				rectangle.setSize(sf::Vector2f(width, height));
			}
		private:
			sf::RectangleShape rectangle;
		};

		class FontResource : public Guider::Resources::FontResource
		{
		public:
			static std::pair<unsigned, float> unpackTextSize(float textSize);

			virtual float getLineHeight(float textSize) const override;
			virtual float getLineWidth(float textSize, const std::string& text) const override;

			inline const sf::Font& getFont() const
			{
				return font;
			}

			inline std::string getName() const
			{
				return name;
			}

			FontResource(const sf::Font& f, const std::string n) : font(f), name(n) {}
		private:
			sf::Font font;
			std::string name;
		};

		class TextResource : public Guider::Resources::TextResource
		{
		public:
			virtual void draw(Canvas& canvas, const Rect& bounds) override;
			virtual void setText(const std::string& text) override;
			virtual void setTextSize(float size) override;
			virtual void setFont(const Guider::Resources::FontResource& font) override;
			virtual void setColor(const Color& color) override;
			virtual float getLineHeight() const override;
			virtual float getLineWidth() const override;

			TextResource() : fontResource(nullptr), textSize(0) {}
		private:
			sf::Text text;
			const SFMLResources::FontResource* fontResource;
			float textSize;
		};
	}


	class SfmlBackend : public Backend
	{
	public:
		static bool handleEvent(Engine& engine, const sf::Event& event);

		virtual void setDrawOrigin(float x, float y) override;

		virtual void clearMask() override;
		virtual void setupMask() override;
		virtual void useMask() override;
		virtual void disableMask() override;
		virtual void pushMaskLayer() override;
		virtual void popMaskLayer() override;
		virtual void addToMask(const Rect& rect) override;

		virtual std::shared_ptr<Canvas> getCanvas() override;
		virtual void setBounds(const Rect& rect) override;

		virtual Vec2 getSize() const noexcept override;
		virtual void setSize(const Vec2& size) override;

		virtual std::shared_ptr<Resources::RectangleShape> createRectangle(const Vec2& size, const Color& color) override;
		virtual std::shared_ptr<Resources::TextResource> createText(const std::string& text, const Resources::FontResource& font, float size, const Color& color) override;
		virtual std::shared_ptr<Resources::FontResource> getFontByName(const std::string& name) override;
		virtual std::shared_ptr<Resources::FontResource> loadFontFromFile(const std::string& filename, const std::string& name) override;
		virtual std::shared_ptr<Resources::ImageResource> loadImageFromFile(const std::string& filename) override;
		virtual std::shared_ptr<Resources::ImageCanvas> createImage(const Vec2& size) override;
		virtual void deleteResource(Resources::Resource& resource) override;

		SfmlBackend(sf::RenderTarget& t) : Backend(), canvas(std::make_shared<SfmlCanvas>(t)), origin(0, 0), maskLevel(1) {}
	private:
		sf::View view;

		std::shared_ptr<SfmlCanvas> canvas;

		sf::Vector2f origin;
		SFMLResources::RectangleShape maskRect;
		unsigned char maskLevel;
		std::unordered_map<std::string, std::shared_ptr<SFMLResources::FontResource>> fonts;
	};
}
#pragma once

#include <guider/base.hpp>
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace Guider
{
	class SfmlBackend : public Backend
	{
	private:
		class SfmlRectangle: public RectangleShapeComponent
		{
		private:
			SfmlBackend& backend;
			sf::RectangleShape shape;
		public:
			void setSize(const Vec2& size) override;
			void setColor(const Color& color) override;

			void draw(const Vec2& offset) override;

			SfmlRectangle(SfmlBackend& b,uint64_t id) : RectangleShapeComponent(id), backend(b) {}
		};

		class SfmlFont : public FontResource
		{
		private:
			sf::Font font;
			std::string name;
		public:
			static std::pair<unsigned, float> unpackTextSize(float textSize);

			float getLineHeight(float textSize) const override;
			float getLineWidth(float textSize, const std::string& text) const override;

			inline const sf::Font& getFont() const
			{
				return font;
			}

			inline std::string getName() const
			{
				return name;
			}

			SfmlFont(const sf::Font& f, const std::string n, uint64_t id) : FontResource(id), font(f), name(n) {}
		};

		class SfmlText : public TextResource
		{
		private:
			float size;
			sf::Text text;
			const FontResource* font;
			SfmlBackend& backend;
		public:
			virtual void setText(const std::string& text) override;
			virtual void setTextSize(float size) override;
			virtual void setFont(const FontResource& font) override;
			virtual void setColor(const Color& color) override;
			virtual float getLineHeight() const override;
			virtual float getLineWidth() const override;

			void draw(const Vec2& offset) override;

			SfmlText(SfmlBackend& b, uint64_t id) : TextResource(id), size(0), font(nullptr), backend(b) {}
		};

		sf::Vector2f origin;
		SfmlRectangle maskRect;
		unsigned char maskLevel;
		std::unordered_map<std::string, std::shared_ptr<SfmlFont>> fonts;
		size_t lid;
		std::unordered_map<size_t, std::shared_ptr<Resource>> resources;
	protected:
		virtual void setViewport(const Rect& rect) override;

		void addResource(const std::shared_ptr<Resource>& resource);
	public:
		sf::Vector2f getCurrentDrawingoffset() const noexcept;

		sf::RenderTexture target;
		virtual std::shared_ptr<RectangleShapeComponent> createRectangle(const Vec2& size, const Color& color) override;
		virtual std::shared_ptr<TextResource> createText(const std::string& text, const FontResource& font, float size, const Color& color) override;

		std::shared_ptr<FontResource> loadFontFromFile(const std::string& file,const std::string& name);
		std::shared_ptr<FontResource> getFontByName(const std::string& name);

		virtual void deleteResource(Resource& resource) override;
		virtual void deleteResource(uint64_t id) override;

		virtual void setDrawOrigin(float x, float y) override;

		virtual void clearMask() override;
		virtual void setupMask() override;
		virtual void useMask() override;
		virtual void disableMask() override;
		virtual void pushMaskLayer() override;
		virtual void popMaskLayer() override;
		virtual void addToMask(const Rect& rect) override;

		virtual Vec2 getSize() const noexcept override;
		virtual void setSize(const Vec2& size) override;

		void loadFont(const std::string& name, const sf::Font& font);

		SfmlBackend(): Backend(), origin(0, 0), maskRect(*this,0), maskLevel(1), lid(0) {}
	};
}
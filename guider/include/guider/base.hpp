#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <list>
#include <memory>


namespace Guider
{
	class Rect
	{
	public:
		float left, top, width, height;

		Rect& operator = (const Rect&) = default;
		Rect& operator = (Rect&&) noexcept = default;

		bool operator == (const Rect&) const noexcept;
		bool operator != (const Rect&) const noexcept;

		bool intersects(const Rect&) const noexcept;

		Rect();
		Rect(float l, float t, float w, float h);
		Rect(const Rect&) = default;
		Rect(Rect&&) noexcept = default;
	};
	class Color
	{
	public:
		union
		{
			uint32_t value;
			struct
			{
				uint8_t r, g, b, a;
			};
		};

		inline uint32_t hex() const noexcept
		{
			return r << 24 | g << 16 | b << 8 | a;
		}

		Color() : r(0), g(0), b(0), a(255) {}
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}
		Color(uint32_t v)
		{
			a = v & 0xFF;
			v >>= 8;
			b = v & 0xFF;
			v >>= 8;
			g = v & 0xFF;
			v >>= 8;
			r = v & 0xFF;
		}
		Color(const Color& t) : value(t.value) {}
	};
	class Vec2
	{
	public:
		float x, y;

		Vec2& operator = (const Vec2&) = default;
		Vec2& operator = (Vec2&&) noexcept = default;

		Vec2& operator += (const Vec2& t) noexcept;
		Vec2 operator + (const Vec2& t) const noexcept;

		Vec2& operator -= (const Vec2& t) noexcept;
		Vec2 operator - (const Vec2& t) const noexcept;

		Vec2();
		Vec2(float w, float h);
		Vec2(const Vec2&) = default;
		Vec2(Vec2&&) noexcept = default;
	};
	using String = std::string;

	class RenderBackend
	{
	private:
		Rect bounds;
		std::vector<Vec2> offsets;
	protected:
		virtual void setViewport(const Rect& rect) = 0;
	public:
		virtual void drawRectangle(const Rect& rect) = 0;
		virtual void drawText(float x, float y, const String& text) = 0;

		void pushDrawOffset(const Vec2& offset);
		virtual void setDrawOrigin(float x, float y) = 0;
		void popDrawOffset();

		virtual void clearMask() = 0;
		virtual void setupMask() = 0;
		virtual void useMask() = 0;
		virtual void disableMask() = 0;
		virtual void pushMaskLayer() = 0;
		virtual void popMaskLayer() = 0;
		virtual void addToMask(const Rect& rect) = 0;
		virtual void setColor(const Color& color) = 0;

		void limitView(const Rect& rect);
		void setView(const Rect& rect);
		virtual Vec2 getSize() const noexcept = 0;
		virtual void setSize(const Vec2& size) = 0;
	};

	class Component
	{
	private:
		Component* parent;
		bool clean;
		mutable bool redraw;
		Rect bounds;
	protected:
		inline void setBounds(Component& c, const Rect& r) const
		{
			Rect lastBounds = c.bounds;
			c.bounds = r;
			c.onResize(lastBounds);
		}
		inline void setClean(Component&)
		{
			clean = true;
		}
	public:
		enum class SizingMode
		{
			OwnSize,
			MatchParent,
			WrapContent
		};
	private:
		SizingMode sizingModeW, sizingModeH;
		float width, height;
	public:
		inline void setSizingMode(SizingMode w, SizingMode h)
		{
			if (sizingModeW != w)
			{
				sizingModeW = w;
				invalidate();
			}
			if (sizingModeH != h)
			{
				sizingModeH = h;
				invalidate();
			}
		}
		inline SizingMode getSizingModeVertical() const noexcept
		{
			return sizingModeH;
		}
		inline SizingMode getSizingModeHorizontal() const noexcept
		{
			return sizingModeW;
		}
		inline void setParent(Component& p)
		{
			parent = &p;
			invalidate();
		}
		inline bool isClean() const noexcept
		{
			return clean;
		}
		inline void setWidth(float w)
		{
			width = w;
			invalidate();
		}
		inline void setHeight(float h)
		{
			height = h;
			invalidate();
		}
		inline void setSize(float w, float h)
		{
			width = w;
			height = h;
			invalidate();
		}
		inline void setSize(const Vec2& size)
		{
			setSize(size.x, size.y);
		}

		inline float getWidth() const noexcept
		{
			return width;
		}
		inline float getHeight() const noexcept
		{
			return height;
		}

		virtual void drawMask(RenderBackend& renderer) const;

		virtual void poke();
		virtual void onResize(const Rect& bounds);

		class DimensionDesc
		{
		public:
			enum Mode
			{
				Exact,
				Max,
				Min
			};
			float value;
			Mode mode;

			DimensionDesc(float value, Mode mode);
			DimensionDesc(const DimensionDesc&) = default;
		};

		virtual void onChildStain(Component& c);
		virtual void onChildNeedsRedraw(Component& c);

		void invalidate();
		void invalidateVisuals();
		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height);

		Component* getParent();
		const Component* getParent() const;

		Rect getBounds() const;
		Rect getGlobalBounds() const;

		using Type = std::shared_ptr<Component>;
		virtual void onDraw(RenderBackend& renderer) const = 0;
		void draw(RenderBackend& renderer) const;

		Component() : parent(nullptr), redraw(true), clean(false), sizingModeH(SizingMode::OwnSize), sizingModeW(SizingMode::OwnSize), width(0), height(0) {}
	};

	class Container : public Component
	{
	public:
		virtual void addChild(const Component::Type& child) = 0;
		virtual void removeChild(const Component::Type& child) = 0;
		virtual void clearChildren() = 0;
	};

	class AbsoluteContainer : public Container
	{
	private:
		struct Element
		{
			Component::Type component;
			float x, y;

			Element(const Component::Type& c, float x, float y);
			Element(const Element&) = default;
		};
		std::vector<Element> children;

		mutable std::unordered_set<Component*> toUpdate;
	public:
		virtual void drawMask(RenderBackend& renderer) const override;

		virtual void addChild(const Component::Type& child) override;
		void addChild(const Component::Type& child, float x, float y);
		virtual void removeChild(const Component::Type& child) override;
		virtual void clearChildren() override;

		virtual void poke() override;

		virtual void onResize(const Rect& last) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual void onDraw(RenderBackend& renderer) const override;
	};

	class Engine : public Component
	{
	private:
		RenderBackend& backend;
	public:
		AbsoluteContainer container;

		void resize(const Vec2& size);
		void update();
		void onDraw(RenderBackend& renderer) const override;
		void draw() const;

		Engine(RenderBackend& b) : backend(b) {};
	};
}
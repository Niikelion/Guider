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
	class Rect
	{
	public:
		float left, top, width, height;

		Rect& operator = (const Rect&) = default;
		Rect& operator = (Rect&&) noexcept = default;

		bool operator == (const Rect&) const noexcept;
		bool operator != (const Rect&) const noexcept;

		bool intersects(const Rect&) const noexcept;
		bool contains(const Vec2&) const noexcept;

		inline Rect at(const Vec2& pos) const noexcept
		{
			return Rect(pos.x,pos.y,width,height);
		}
		inline Vec2 position() const noexcept
		{
			return Vec2(left,top);
		}
		inline Vec2 size() const noexcept
		{
			return Vec2(width,height);
		}

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
	
	using String = std::string;

	//TODO: add objects for caching
	class Backend
	{
	public:
		class Resource
		{
		private:
			uint64_t id;
		public:
			inline uint64_t getId() const noexcept
			{
				return id;
			}
			Resource(uint64_t i) : id(i) {};
			Resource(const Resource& r) : id(r.id) {}
			virtual ~Resource() = default;
		};

		class Drawable: public Resource
		{
		public:
			virtual void draw(const Vec2& offset) = 0;

			using Resource::Resource;
		};

		class RectangleShape : public Drawable
		{
		public:
			virtual void setSize(const Vec2& size) = 0;
			virtual void setColor(const Color& color) = 0;

			using Drawable::Drawable;
		};

		class FontResource : public Resource
		{
		public:
			virtual float getLineHeight(float textSize) const = 0;
			virtual float getLineWidth(float textSize, const std::string& text) const = 0;

			using Resource::Resource;
		};

		class TextResource : public Drawable
		{
		public:
			virtual void setText(const std::string& text) = 0;
			virtual void setTextSize(float size) = 0;
			virtual void setFont(const FontResource& font) = 0;
			virtual void setColor(const Color& color) = 0;
			virtual float getLineHeight() const = 0;
			virtual float getLineWidth() const = 0;

			using Drawable::Drawable;
		};
	private:
		Rect bounds;
		std::vector<Vec2> offsets;

		std::shared_ptr<RectangleShape> rectangle;
	protected:
		virtual void setViewport(const Rect& rect) = 0;
	public:
		virtual std::shared_ptr<RectangleShape> createRectangle(const Vec2& size, const Color& color) = 0;
		virtual std::shared_ptr<TextResource> createText(const std::string& text, const FontResource& font, float size, const Color& color) = 0;
		
		virtual std::shared_ptr<FontResource> getFontByName(const std::string& name) = 0;

		virtual void deleteResource(Resource& resource) = 0;
		virtual void deleteResource(uint64_t id) = 0;

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

		void limitView(const Rect& rect);
		void setView(const Rect& rect);
		virtual Vec2 getSize() const noexcept = 0;
		virtual void setSize(const Vec2& size) = 0;

		void drawRectangle(const Rect& rect,const Color& color);
	};

	class Event
	{
	public:
		struct BackendConnectedEvent
		{
			Backend& backend;

			BackendConnectedEvent(Backend& b) : backend(b) {}
			BackendConnectedEvent(const BackendConnectedEvent& t) : backend(t.backend) {}
		};

		struct MouseEvent
		{
			float x, y;
			uint8_t button;

			enum Subtype
			{
				Moved,
				ButtonDown,
				ButtonUp,
				Left
			};

			MouseEvent(float xpos,float ypos, uint8_t buttonCode) : x(xpos), y(ypos), button(buttonCode) {}
			MouseEvent(const MouseEvent& t, const Rect& bounds) : MouseEvent(t)
			{
				x -= bounds.left;
				y -= bounds.top;
			}
			MouseEvent(const MouseEvent& t) = default;
		};

		enum Type
		{
			None,
			Invalidated,
			BackendConnected,
			MouseMoved,
			MouseButtonDown,
			MouseButtonUp,
			MouseLeft
		};
		Type type;

		union
		{
			BackendConnectedEvent backendConnected;
			MouseEvent mouseEvent;
		};

		static inline Event createInvalidatedEvent()
		{
			return Event(Type::Invalidated);
		}
		static inline Event createBackendConnectedEvent(Backend& backend)
		{
			Event e(Type::BackendConnected);
			new(&e.backendConnected) BackendConnectedEvent(backend);
			return e;
		}
		static Event createMouseEvent(MouseEvent::Subtype subtype,float x,float y, uint8_t button);

		void dispose();

		Event(const Event&);
	private:
		Event(Type type);
	};

	class Component
	{
	private:
		Backend* backend;
		Component* parent;
		bool clean;
		mutable bool redraw;
		Rect bounds;
		bool hasMouseOver;
	protected:
		inline void setBounds(Component& c, const Rect& r) const
		{
			Rect lastBounds = c.bounds;
			c.bounds = r;
			c.onResize(lastBounds);
		}
		inline void setClean()
		{
			clean = true;
		}
	public:
		inline void _resetMouseOver()
		{
			hasMouseOver = false;
		}
		inline void _setMouseOver()
		{
			hasMouseOver = true;
		}
		inline bool _getMouseOver() const
		{
			return hasMouseOver;
		}
		enum class SizingMode
		{
			OwnSize,
			GivenSize,
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
			if (parent->backend != nullptr)
				handleEvent(Event::createBackendConnectedEvent(*parent->backend));
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

		inline Backend* getBackend() const noexcept
		{
			return backend;
		}
		inline void setBackend(Backend& b)
		{
			handleEvent(Event::createBackendConnectedEvent(b));
		}

		virtual void drawMask(Backend& renderer) const;

		virtual void poke();
		virtual void onResize(const Rect& lastBounds);

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

		virtual void handleEvent(const Event& event);

		void invalidate();
		void invalidateVisuals();
		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height);

		Component* getParent();
		const Component* getParent() const;

		Rect getBounds() const;
		Rect getGlobalBounds() const;

		using Type = std::shared_ptr<Component>;
		virtual void onDraw(Backend& renderer) const = 0;
		void draw(Backend& renderer) const;

		Component() :backend(nullptr), parent(nullptr), redraw(true), clean(false), hasMouseOver(false), sizingModeH(SizingMode::OwnSize), sizingModeW(SizingMode::OwnSize), width(0), height(0) {}

		virtual ~Component() = default;
	};

	class Container : public Component
	{
	public:
		virtual void addChild(const Component::Type& child) = 0;
		virtual void removeChild(const Component::Type& child) = 0;
		virtual void clearChildren() = 0;

		void handleEvent(const Event& event) override;
		static Event adjustEventForComponent(const Event& event, Component& component);
		void handleEventForComponent(const Event& event, Component& component);

		virtual void propagateEvent(const Event& event) = 0;
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
		virtual void drawMask(Backend& renderer) const override;

		virtual void addChild(const Component::Type& child) override;
		void addChild(const Component::Type& child, float x, float y);
		virtual void removeChild(const Component::Type& child) override;
		virtual void clearChildren() override;

		virtual void poke() override;

		virtual void onResize(const Rect& last) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual void propagateEvent(const Event& event) override;

		virtual void onDraw(Backend& renderer) const override;
	};

	class Engine : public Component
	{
	private:
		Backend& backend;
	public:
		AbsoluteContainer container;

		void resize(const Vec2& size);
		void update();
		void onDraw(Backend& renderer) const override;
		void draw() const;

		Engine(Backend& b) : backend(b)
		{
			container.setBackend(b);
		};
	};
}
#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <list>
#include <string>
#include <stdexcept>
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
	
	class Padding
	{
	public:
		float left, right, top, bottom;

		Rect calcContentArea(const Rect& bounds) const;

		Padding() : left(0), right(0), top(0), bottom(0) {}
		Padding(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b) {}
		Padding(const Padding&) = default;
	};

	using String = std::string;

	enum class Gravity
	{
		Start = 0,
		Left = Start,
		Top = Start,

		Middle = 1,
		Center = Middle,

		End = 2,
		Right = End,
		Bottom = End
	};

	namespace Resources
	{
		class Drawable;
	}

	class Canvas
	{
	public:
		virtual void drawRectangle(const Rect& rect, const Color& color) = 0;
		void draw(Resources::Drawable& drawable, const Rect& rect);

		template<typename T>T& as()
		{
			return static_cast<T&>(*this);
		}
	};

	namespace Resources
	{
		class Resource
		{
		public:
			virtual ~Resource() = default;
		};

		class Drawable : public Resource
		{
		public:
			virtual void draw(Canvas& canvas, const Rect& bounds) = 0;
		};

		class ImageResource : public Drawable
		{
		public:
			inline size_t getWidth() const noexcept
			{
				return width;
			}
			inline size_t getHeight() const noexcept
			{
				return height;
			}
			ImageResource() : width(0), height(0) {}
			ImageResource(size_t w, size_t h) : width(w), height(h) {}
		protected:
			size_t width, height;
		};

		class ImageCanvas : public Canvas
		{
		public:
			virtual ImageResource& getImage() = 0;
		};

		class RectangleShape : public Drawable
		{
		public:
			virtual void setSize(const Vec2& size) = 0;
			virtual void setColor(const Color& color) = 0;
		};

		class FontResource : public Resource
		{
		public:
			virtual float getLineHeight(float textSize) const = 0;
			virtual float getLineWidth(float textSize, const std::string& text) const = 0;
		};

		class TextResource : public Drawable
		{
		public:
			Gravity verticalAlignment,horizontalAlignment;

			virtual void setText(const std::string& text) = 0;
			virtual void setTextSize(float size) = 0;
			virtual void setFont(const FontResource& font) = 0;
			virtual void setColor(const Color& color) = 0;
			virtual float getLineHeight() const = 0;
			virtual float getLineWidth() const = 0;

			Rect getAdjustedRect(const Rect& r);

			TextResource() : verticalAlignment(Gravity::Center), horizontalAlignment(Gravity::Center) {}
			TextResource(Gravity horizontal, Gravity vertical) : verticalAlignment(vertical), horizontalAlignment(horizontal) {}
		};

		class CompositeDrawable : public Drawable
		{
		public:
			class ElementData
			{
			public:
				Padding padding;
				Gravity horizontalGravity, verticalGravity;
				std::shared_ptr<Drawable> drawable;
			};
		private:
			std::vector<ElementData> elements;
		public:
			virtual void draw(Canvas& canvas, const Rect& bounds);

			CompositeDrawable(uint64_t id, const std::vector<ElementData>& drawables) : elements(drawables) {}
			CompositeDrawable(const CompositeDrawable&) = default;
		};
	}

	class Backend
	{
	public:
		virtual std::shared_ptr<Resources::RectangleShape> createRectangle(const Vec2& size, const Color& color) = 0;
		virtual std::shared_ptr<Resources::TextResource> createText(const std::string& text, const Resources::FontResource& font, float size, const Color& color) = 0;

		virtual std::shared_ptr<Resources::FontResource> getFontByName(const std::string& name) = 0;
		virtual std::shared_ptr<Resources::FontResource> loadFontFromFile(const std::string& filename, const std::string& name) = 0;
		virtual std::shared_ptr<Resources::ImageResource> loadImageFromFile(const std::string& filename) = 0;

		virtual std::shared_ptr<Resources::ImageCanvas> createImage(const Vec2& size) = 0;

		virtual void deleteResource(Resources::Resource& resource) = 0;

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

		virtual std::shared_ptr<Canvas> getCanvas() = 0;

		virtual void setBounds(const Rect& rect) = 0;
		virtual Vec2 getSize() const noexcept = 0;
		virtual void setSize(const Vec2& size) = 0;

	private:
		std::vector<Vec2> offsets;
	};

	/*class Backend
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
			virtual void draw(Canvas& canvas,const Rect& bounds) = 0;

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
	};*/

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
	public:
		inline bool isMouseOver() const
		{
			return hasMouseOver;
		}
		inline bool hasMouseButtonFocus() const
		{
			return hasMouseFocus;
		}
		enum class SizingMode
		{
			OwnSize,
			GivenSize,
			MatchParent,
			WrapContent
		};

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
		inline bool needsRedraw() const
		{
			return redraw;
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
		virtual void invalidateVisuals();
		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height);

		Component* getParent();
		const Component* getParent() const;

		Rect getBounds() const;
		Rect getGlobalBounds() const;

		using Type = std::shared_ptr<Component>;
		virtual void onDraw(Canvas& canvas) const = 0;
		void draw(Canvas& canvas) const;

		Component() :backend(nullptr), parent(nullptr), redraw(true), clean(false), hasMouseOver(false), hasMouseFocus(false), sizingModeH(SizingMode::OwnSize), sizingModeW(SizingMode::OwnSize), width(0), height(0) {}

		virtual ~Component() = default;

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
	private:
		Backend* backend;
		Component* parent;
		bool clean;
		mutable bool redraw;
		Rect bounds;
		bool hasMouseOver;
		bool hasMouseFocus;

		inline void setMouseOver()
		{
			hasMouseOver = true;
		}
		inline void resetMouseOver()
		{
			hasMouseOver = false;
		}
		inline void setMouseFocus()
		{
			hasMouseFocus = true;
		}
		inline void resetMouseFocus()
		{
			hasMouseFocus = false;
		}
		SizingMode sizingModeW, sizingModeH;
		float width, height;
	};

	class Container : public Component
	{
	public:
		class IteratorBase
		{
		public:
			virtual bool end() const = 0;
			virtual void loadNext() = 0;
			virtual Component& current() = 0;
			virtual std::unique_ptr<IteratorBase> clone() const = 0;

			virtual ~IteratorBase() = default;
		};

		template<typename T> class IteratorTemplate: public IteratorBase
		{
		public:
			virtual std::unique_ptr<IteratorBase> clone() const override
			{
				return std::unique_ptr<IteratorBase>(new T(*(T*)(this)));
			}
		};

		template<typename IteratorT> class CommonIteratorTemplate: public IteratorBase
		{
		public:
			virtual bool end() const override
			{
				return endIt == currentIt;
			}

			virtual void loadNext() override
			{
				if (!end())
					++currentIt;
			}

			virtual Component& current() override
			{
				return *currentIt->get();
			}

			virtual std::unique_ptr<IteratorBase> clone() const
			{
				return std::unique_ptr<IteratorBase>(new CommonIteratorTemplate<IteratorT>(*this));
			}

			CommonIteratorTemplate(const IteratorT& begin, const IteratorT& end): currentIt(begin), endIt(end) {}
		private:
			IteratorT currentIt;
			const IteratorT endIt;
		};

		class Iterator
		{
		public:
			inline bool end()
			{
				return ptr->end();
			}

			inline void loadNext()
			{
				ptr->loadNext();
			}

			inline Component& current()
			{
				if (end())
					throw std::logic_error("Iterator out of bounds");
				return ptr->current();
			}

			Iterator& operator ++ (int)
			{
				ptr->loadNext();
				return *this;
			}

			Iterator(IteratorBase* p) : ptr(p) {}
			Iterator(const Iterator& t) : ptr(t.ptr->clone()) {}
			Iterator(Iterator&&) noexcept = default;
		private:
			std::unique_ptr<IteratorBase> ptr;
		};

		template<typename T, typename... Args>static Iterator createIterator(Args... args)
		{
			return Iterator((IteratorBase*)new T(std::forward<Args>(args)...));
		}

		virtual void invalidateVisuals() override;

		virtual void addChild(const Component::Type& child) = 0;
		virtual void removeChild(const Component::Type& child) = 0;
		virtual void clearChildren() = 0;

		virtual Iterator firstElement() = 0;

		void handleEvent(const Event& event) override;
		static Event adjustEventForComponent(const Event& event, Component& component);
		void handleEventForComponent(const Event& event, Component& component);
	};
	//TODO: move to containers.hpp file
	class AbsoluteContainer : public Container
	{
	public:
		virtual void drawMask(Backend& renderer) const override;

		virtual void addChild(const Component::Type& child) override;
		void addChild(const Component::Type& child, float x, float y);
		virtual void removeChild(const Component::Type& child) override;
		virtual void clearChildren() override;

		virtual Iterator firstElement() override;

		virtual void poke() override;

		virtual void onResize(const Rect& last) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual void onDraw(Canvas& canvas) const override;
	private:
		struct Element
		{
			Component::Type component;
			float x, y;

			Element(const Component::Type& c, float x, float y);
			Element(const Element&) = default;
		};

		using iterator = std::vector<Element>::iterator;

		class IteratorType : public IteratorTemplate<IteratorType>
		{
		public:
			virtual bool end() const override
			{
				return endIt == currentIt;
			}
			virtual void loadNext() override
			{
				if (!end())
					++currentIt;
			}
			virtual Component& current() override
			{
				return *currentIt->component;
			}

			IteratorType(const iterator& begin, const iterator& end) : currentIt(begin), endIt(end) {}
			IteratorType(const IteratorType&) = default;
		private:
			iterator currentIt;
			const iterator endIt;
		};
		std::vector<Element> children;

		mutable std::unordered_set<Component*> toUpdate;
	};

	class Engine : public Container
	{
	public:
		virtual void drawMask(Backend& renderer) const override;

		virtual void addChild(const Component::Type& child) override;
		virtual void removeChild(const Component::Type& child) override;
		virtual void clearChildren() override;

		virtual Iterator firstElement() override;

		virtual void onChildNeedsRedraw(Component& c) override;

		void resize(const Vec2& size);
		void update();
		void onDraw(Canvas& canvas) const override;
		void draw() const;

		Engine(Backend& b) : backend(b)
		{
			canvas = b.getCanvas();
			setBackend(b);
		};

		Engine(Backend& b, const std::shared_ptr<Canvas>& c) : backend(b), canvas(c)
		{
			setBackend(b);
		};

	private:
		using IteratorType = CommonIteratorTemplate<std::vector<Component::Type>::iterator>;
		Backend& backend;
		mutable std::unordered_set<Component*> toRedraw;
		std::shared_ptr<Canvas> canvas;
		std::vector<Component::Type> elements;
	};
}
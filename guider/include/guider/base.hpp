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
#include <functional>
#include <algorithm>


namespace Guider
{
	/// @brief Basic class representing 2d point/vector.
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

		/// @brief Default constructor, initializes x and y to 0.
		Vec2();
		/// @brief Initializes x and y with provided values.
		/// @param w x coordinate.
		/// @param h y coordinate.
		Vec2(float w, float h);
		Vec2(const Vec2&) = default;
		Vec2(Vec2&&) noexcept = default;
	};
	/// @brief Basic class representing rectangle.
	class Rect
	{
	public:
		float left, top, width, height;

		Rect& operator = (const Rect&) = default;
		Rect& operator = (Rect&&) noexcept = default;

		bool operator == (const Rect&) const noexcept;
		bool operator != (const Rect&) const noexcept;

		Rect& operator += (const Vec2& offset) noexcept;
		Rect& operator -= (const Vec2& offset) noexcept;

		Rect operator + (const Vec2& offset) const noexcept;
		Rect operator - (const Vec2& offset) const noexcept;

		/// @brief Checks if 2 rects intersects.
		bool intersects(const Rect& rect) const noexcept;
		/// @brief Checks if rect cointains  point.
		bool contains(const Vec2& point) const noexcept;

		/// @brief Returns copy of rect with top left corner at pos.
		inline Rect at(const Vec2& pos) const noexcept
		{
			return Rect(pos.x, pos.y, width, height);
		}
		/// @brief Returns top left corner as point. 
		inline Vec2 position() const noexcept
		{
			return Vec2(left, top);
		}
		/// @brief Returns size of the rect as vector.
		inline Vec2 size() const noexcept
		{
			return Vec2(width, height);
		}

		Rect();
		Rect(float l, float t, float w, float h);
		Rect(const Rect&) = default;
		Rect(Rect&&) noexcept = default;
	};
	/// @brief Basic color class.
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

		/// @brief Returns color as number in standard format.
		/// 
		/// Result is in 0xrrggbbaa format for easier printing and parsing.
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
		/// @brief Constructs color from hex reprezentation.
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

		/// @brief Returns adjusted content rect inside provided rect.
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

	/// @interface Canvas
	/// @brief Base for drawing surface.
	class Canvas
	{
	public:
		virtual void drawRectangle(const Rect& rect, const Color& color) = 0;
		void draw(Resources::Drawable& drawable, const Rect& rect);

		/// @brief Cast from canvas interface to implementation.
		/// @tparam T implementation type.
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
			Gravity verticalAlignment, horizontalAlignment;

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

	/// @interface Backend
	/// @brief Base for rendering backend.
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
		Vec2 getDrawOffset() const;
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

			MouseEvent(float xpos, float ypos, uint8_t buttonCode) : x(xpos), y(ypos), button(buttonCode) {}
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
			VisualsInvalidated,
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

		static inline Event createInvalidatedEvent();
		static inline Event createVisualsInvalidatedEvent();
		static inline Event createBackendConnectedEvent(Backend& backend);
		static Event createMouseEvent(MouseEvent::Subtype subtype, float x, float y, uint8_t button);

		void dispose();

		Event(const Event&);
	private:
		Event(Type type);
	};

	/// @interface Component
	/// @brief Gui component base.
	class Component
	{
	public:
		/// @brief Mode for calculating size.
		enum class SizingMode
		{
			/// @brief Maintain size set by setWidth and setHeight.
			OwnSize,
			/// @brief Maintain given size.
			GivenSize,
			/// @brief Fill parent rect.
			MatchParent,
			/// @brief Wrap content.
			WrapContent
		};

		/// @brief Used as hint in calculating size.
		/// 
		/// Carries data for sizing used in container <-> element communication.
		class DimensionDesc
		{
		public:
			enum Mode
			{
				Exact,
				Max,
				Min
			};
			/// @brief Size
			float value;
			/// @brief Mode
			Mode mode;

			DimensionDesc(float value, Mode mode);
			DimensionDesc(const DimensionDesc&) = default;
		};

		/// @brief Shortcut for defining type to store component.
		using Type = std::shared_ptr<Component>;

		/// @brief Checks if mouse is currently over component
		/// @return True when mouse is inside bounding rect, false otherwise.
		bool isMouseOver() const;

		/// @brief Checks if component has mouse focus.
		/// 
		/// Element gains mouse focus when user starts mouse click
		/// inside bounds of given element.
		/// @return True when has focus, false otherwise.
		bool hasMouseButtonFocus() const;

		/// @brief Set sizing mode for components.
		/// @param w Horizontal sizing mode.
		/// @param h Vertical sizing mode.
		void setSizingMode(SizingMode w, SizingMode h);
		/// @brief Gets vertical sizing mode.
		SizingMode getSizingModeVertical() const noexcept;
		/// @brief Gets horizontal sizing mode. 
		SizingMode getSizingModeHorizontal() const noexcept;
		/// @brief Sets parent for element.
		/// 
		/// Invalidates element.
		/// Additionaly, if parent has connected backed, BackendConnected event is sent to current element.
		/// @warn Should only be used by containers for setting themselfs as parents.
		/// @param p New parent.
		void setParent(Component& p);
		/// @brief Removes parent.
		void removeParent();
		/// @brief Checks if element needs udpate.
		bool isClean() const noexcept;
		/// @brief Checks if elements needs redraw. 
		bool needsRedraw() const;
		/// @brief Sets width.
		/// 
		/// Invalidates element.
		/// @note Note, that depending on sizing mode and container, 
		/// actuall element size might end up different than this.
		/// @param w Width.
		void setWidth(float w);
		/// @brief Sets height.
		/// 
		/// Invalidates element.
		/// @note Note, that depending on sizing mode and container, 
		/// actuall element size might end up different than this.
		/// @param h Height.
		void setHeight(float h);
		/// @brief Sets size.
		/// 
		/// Invalidates element.
		/// @note Note, that depending on sizing mode and container, 
		/// actuall element size might end up different than this.
		/// @param w Width.
		/// @param h Height.
		void setSize(float w, float h);
		/// @brief Sets size.
		/// 
		/// Invalidates element.
		/// @note Note, that depending on sizing mode and container, 
		/// actuall element size might end up different than this.
		/// @param size Size.
		void setSize(const Vec2& size);
		/// @brief Gets width set by setWidth.
		/// 
		/// @note This is may not be the real width,
		/// to get actuall width, use getBounds().
		float getWidth() const noexcept;
		/// @brief Gets height set by setHeight.
		/// 
		/// @note This is may not be the real height,
		/// to get actuall height, use @ref getBounds().
		float getHeight() const noexcept;

		/// @brief Returns pointer to connected backend.
		/// @return Pointer to backend if connected, nullptr otherwise.
		Backend* getBackend() const noexcept;
		/// @brief Sets 
		/// @param b 
		void setBackend(Backend& b);

		/// @brief Updates element.
		/// 
		/// @note When overriding, base @ref poke should be called.
		virtual void poke();

		/// @brief Processes event.
		/// 
		/// @note When overriding, base @ref handleEvent should be called.
		/// @param event 
		/// @return True when event should not be futher processed.
		virtual bool handleEvent(const Event& event);

		/// @brief Marks component as dirty.
		/// 
		/// Tels container that this component needs update.
		void invalidate();
		/// @brief Invalidates visuals.
		/// 
		/// Tells container that this component need redraw.
		virtual void invalidateVisuals();

		/// @brief Recursively invalidates components.
		void invalidateRecursive();
		/// @brief Recursively invalidates visuals.
		void invalidateVisualsRecursive();

		/// @brief Measures components desired size.
		/// @param width Width sugesstion.
		/// @param height Height suggestion.
		/// @return Measurements desired by component.
		virtual std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& width, const DimensionDesc& height);

		/// @brief Returns current parent.
		Component* getParent();
		/// @brief Returns current parent.
		const Component* getParent() const;

		/// @brief Returns local bounding box.
		/// 
		/// Local position is measured relative to parents left top corner.
		Rect getBounds() const;
		/// @brief Returns global rect.
		/// 
		/// @warn Potentially expensive as it needs to get positions of all parent components.
		Rect getGlobalBounds() const;

		/// @name Callbacks
		/// @{

		/// @brief Callback for bounds change.
		/// 
		/// @note Preferred way of handling is to mark internal structure
		/// as invalid and recalculate it in the next poke call. 
		/// @param lastBounds bounds from before the change.
		virtual void onResize(const Rect& lastBounds);
		/// @brief Callback for childs invalidation.
		/// 
		/// @param c child
		virtual void onChildStain(Component& c);
		/// @brief Callback for childs visual invalidation.
		/// 
		/// Called every time direct child is invalidated.
		/// @param c child
		virtual void onChildNeedsRedraw(Component& c);
		/// @brief Callback for drawing mask.
		/// 
		/// Called every time direct child needs redraw.
		/// @note Should not modify any internal state(for potential redrawing in the same frame).
		/// @warning Should not be called directly.
		/// @param canvas Target surface.
		virtual void onMaskDraw(Canvas& canvas) const;
		/// @brief Main callback for drawing component.
		/// 
		/// Enables custom drawing for components.
		/// Called every frame that component has invalided visuals.
		/// @note Component is allowed to not redraw itself every frame, but rather only draw updated fragments.
		/// @warning Should not be called directly.
		/// @param canvas Target surface.
		virtual void onDraw(Canvas& canvas) = 0;
		/// @brief Secondary callback for drawing.
		/// 
		/// Enables custom drawing for components.
		/// Called every frame that component has invalidated visuals and should be entirely redrawn.
		/// @note Unlike onDraw, component should redraw itself completely.
		/// @warning Should not be called directly.
		/// @param canvas Target surface.
		virtual void onRedraw(Canvas& canvas);

		/// @}

		/// @brief Draws mask.
		/// 
		/// Calls @ref onMaskDraw.
		/// @param canvas Canvas to draw on.
		void drawMask(Canvas& canvas) const;
		/// @brief Draws component.
		/// 
		/// Calls @ref onDraw.
		/// @param canvas Canvas to draw on.
		void draw(Canvas& canvas);
		/// @brief Redraws component.
		/// 
		/// Calls @ref onRedraw.
		/// @param canvas Canvas to draw on.
		void redraw(Canvas& canvas);

		/// @brief Sets event callback.
		/// 
		/// Callback is called every time component receives an event.
		/// @note When callback returns true, it steals event(it is not passed to the rest of this subtree).
		/// @param callback Callback to set.
		/// @return Returns last callback.
		std::function<bool(const Event&)> setOnEventCallback(const std::function<bool(const Event&)>& callback);

		/// @brief Casts component to derived type.
		/// 
		/// Performs runtime type checks.
		/// Throws exception on error.
		/// @tparam T Derived type.
		template<typename T>T& as()
		{
			T* ret = dynamic_cast<T*>(this);
			if (ret == nullptr)
				throw std::runtime_error("Invalid cast");
			return *ret;
		}
		/// @brief Checks if component can be casted to given type.
		/// @tparam T Type to check.
		template<typename T>bool is()
		{
			return dynamic_cast<T*>(this) != nullptr;
		}

		Component();

		virtual ~Component() = default;

	protected:
		void setBounds(Component& c, const Rect& r) const;
		void setClean();
	private:
		Backend* backend;
		Component* parent;
		bool clean;
		bool toRedraw;
		Rect bounds;
		bool hasMouseOver;
		bool hasMouseFocus;

		SizingMode sizingModeW, sizingModeH;
		float width, height;

		std::function<bool(const Event&)> eventCallback;

		void setMouseOver();
		void resetMouseOver();
		void setMouseFocus();
		void resetMouseFocus();
	};

	/// @interface Container
	/// @brief Gui container base.
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

		template<typename T> class IteratorTemplate : public IteratorBase
		{
		public:
			virtual std::unique_ptr<IteratorBase> clone() const override
			{
				return std::make_unique<T>(*static_cast<const T*>(this));
			}
		};

		template<typename IteratorT> class CommonIteratorTemplate : public IteratorBase
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
				return std::make_unique<CommonIteratorTemplate<IteratorT>>(*this);
			}

			CommonIteratorTemplate(const IteratorT& begin, const IteratorT& end) : currentIt(begin), endIt(end) {}
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

		template<typename T, typename... Args>static Iterator createIterator(Args&&... args)
		{
			return Iterator((IteratorBase*)new T(std::forward<Args>(args)...));
		}

		virtual void invalidateVisuals() override;

		virtual void addChild(const Component::Type& child) = 0;
		virtual void removeChild(const Component::Type& child) = 0;
		virtual void clearChildren() = 0;

		virtual Iterator firstElement() = 0;

		virtual bool handleEvent(const Event& event) override;
		static Event adjustEventForComponent(const Event& event, Component& component);
		void handleEventForComponent(const Event& event, Component& component);
	};


	class Engine : public Container
	{
	public:
		virtual void addChild(const Component::Type& child) override;
		virtual void removeChild(const Component::Type& child) override;
		virtual void clearChildren() override;

		virtual Iterator firstElement() override;

		virtual void onChildNeedsRedraw(Component& c) override;

		void resize(const Vec2& size);
		void update();

		virtual void onMaskDraw(Canvas& canvas) const override;
		virtual void onDraw(Canvas& canvas) override;
		virtual void onRedraw(Canvas& canvas) override;
		void draw();

		Engine(Backend& b);

		Engine(Backend& b, const std::shared_ptr<Canvas>& c);

	private:
		using IteratorType = CommonIteratorTemplate<std::vector<Component::Type>::iterator>;
		Backend& backend;
		std::unordered_set<Component*> toRedraw;
		std::shared_ptr<Canvas> canvas;
		std::vector<Component::Type> elements;
	};
}
#include <guider/base.hpp>
#include <climits>

namespace Guider
{
	constexpr float feps = std::numeric_limits<float>::epsilon();


	Vec2& Vec2::operator+=(const Vec2& t) noexcept
	{
		x += t.x;
		y += t.y;
		return *this;
	}
	
	Vec2 Vec2::operator+(const Vec2& t) const noexcept
	{
		return Vec2(*this) += t;
	}
	
	Vec2& Vec2::operator-=(const Vec2& t) noexcept
	{
		x -= t.x;
		y -= t.y;
		return *this;
	}
	
	Vec2 Vec2::operator-(const Vec2& t) const noexcept
	{
		return Vec2(*this) -= t;
	}
	
	Vec2::Vec2()
	{
		x = 0;
		y = 0;
	}
	
	Vec2::Vec2(float w, float h)
	{
		x = w;
		y = h;
	}

	
	bool Rect::operator==(const Rect& t) const noexcept
	{
		return
			feps >= std::abs(left - t.left) &&
			feps >= std::abs(width - t.width) &&
			feps >= std::abs(top - t.top) &&
			feps >= std::abs(height - t.height);
	}
	
	bool Rect::operator!=(const Rect& t) const noexcept
	{
		return
			feps < std::abs(left - t.left) ||
			feps < std::abs(width - t.width) ||
			feps < std::abs(top - t.top) ||
			feps < std::abs(height - t.height);
	}
	
	Rect& Rect::operator+=(const Vec2& offset)noexcept
	{
		left += offset.x;
		top += offset.y;
		return *this;
	}
	
	Rect& Rect::operator-=(const Vec2& offset) noexcept
	{
		left -= offset.x;
		top -= offset.y;
		return *this;
	}
	
	Rect Rect::operator+(const Vec2& offset) const noexcept
	{
		Rect tmp = *this;
		tmp += offset;
		return tmp;
	}
	
	Rect Rect::operator-(const Vec2& offset) const noexcept
	{
		Rect tmp = *this;
		tmp -= offset;
		return tmp;
	}
	
	bool Rect::intersects(const Rect& rect) const noexcept
	{
		return	(rect.left <= left + width && rect.left + rect.width >= left) &&
			(rect.top <= top + height && rect.top + rect.height >= top);
	}
	
	bool Rect::contains(const Vec2& pos) const noexcept
	{
		return left < pos.x && top < pos.y && left + width > pos.x && top + height > pos.y;
	}
	
	Rect Rect::at(const Vec2& pos) const noexcept
	{
		return Rect(pos.x, pos.y, width, height);
	}
	
	Vec2 Rect::position() const noexcept
	{
		return Vec2(left, top);
	}
	
	Vec2 Rect::size() const noexcept
	{
		return Vec2(width, height);
	}

	Rect Rect::limit(const Rect& rect) const noexcept
	{
		float	l = std::max(left, rect.left),
			t = std::max(top, rect.top),
			r = std::min(left + width, rect.left + rect.width),
			b = std::min(top + height, rect.top + rect.height);
		Rect ret(l, t, r - l, b - t);
		return ret;
	}

	Rect::Rect()
	{
		left = 0;
		top = 0;
		width = 0;
		height = 0;
	}
	
	Rect::Rect(float l, float t, float w, float h) : left(l), top(t), width(w), height(h)
	{
	}


	uint32_t Color::hex() const noexcept
	{
		return r << 24 | g << 16 | b << 8 | a;
	}
	
	Color::Color() : r(0), g(0), b(0), a(255)
	{
	}
	
	Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	
	Color::Color(uint32_t v)
	{
		a = v & 0xFF;
		v >>= 8;
		b = v & 0xFF;
		v >>= 8;
		g = v & 0xFF;
		v >>= 8;
		r = v & 0xFF;
	}
	
	Color::Color(const Color& t) : value(t.value)
	{
	}

	Rect Padding::calcContentArea(const Rect& bounds) const
	{
		Rect ret = Rect(bounds.left + left, bounds.top + top, bounds.width - left - right, bounds.height - top - bottom);

		if (ret.width < 0)
		{
			ret.width = 0;
			ret.left = (2 * bounds.left + left + bounds.width - right) / 2;
		}

		if (ret.height < 0)
		{
			ret.height = 0;
			ret.top = (2 * bounds.top + top + bounds.height - bottom) / 2;
		}

		return ret;
	}
	
	Padding::Padding() : left(0), right(0), top(0), bottom(0)
	{
	}

	Padding::Padding(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b)
	{
	}


	void Canvas::draw(Resources::Drawable& drawable, const Rect& bounds)
	{
		drawable.draw(*this, bounds);
	}


	void Backend::pushDrawOffset(const Vec2& offset)
	{
		Vec2 pos = offset;
		if (!offsets.empty())
			pos += offsets.back();
		offsets.emplace_back(pos);
		setDrawOrigin(pos.x, pos.y);
	}
	
	Vec2 Backend::getDrawOffset() const
	{
		if (!offsets.empty())
			return offsets.back();
		return Vec2(0.f, 0.f);
	}
	
	void Backend::popDrawOffset()
	{
		offsets.pop_back();
		Vec2 cpos = getDrawOffset();
		setDrawOrigin(cpos.x, cpos.y);
	}

	void Backend::pushBounds(const Rect& rect)
	{
		bounds.push_back(rect.at(getDrawOffset()).limit(getBounds()));
		setBounds(bounds.back());
	}

	void Backend::popBounds()
	{
		bounds.pop_back();
		setBounds(getBounds());
	}

	Rect Backend::getBounds() const
	{
		if (!bounds.empty())
			return bounds.back();
		auto size = getSize();
		return Rect(0, 0, size.x, size.y);
	}

	namespace Resources
	{
		void CompositeDrawable::draw(Canvas& canvas, const Rect& bounds)
		{
			for (auto& sub : elements)
			{
				Rect b = sub.padding.calcContentArea(bounds);
				//TODO: apply gravity(currently not possible-cannot get rect from drawable)
				sub.drawable->draw(canvas, b);
			}
		}

		Rect TextResource::getAdjustedRect(const Rect& r)
		{
			float width = getLineWidth(), height = getLineHeight();

			Vec2 pos;
			switch (horizontalAlignment)
			{
			case Guider::Gravity::Left:
			{
				pos.x = 0;
				break;
			}
			case Guider::Gravity::Middle:
			{
				pos.x = (r.width - width) / 2;
				break;
			}
			case Guider::Gravity::Right:
			{
				pos.x = r.width - width;
				break;
			}
			default:
				break;
			}

			switch (verticalAlignment)
			{
			case Guider::Gravity::Top:
			{
				pos.y = 0;
				break;
			}
			case Guider::Gravity::Middle:
			{
				pos.y = (r.height - height) / 2;
				break;
			}
			case Guider::Gravity::Bottom:
			{
				pos.y = r.height - height;
				break;
			}
			default:
				break;
			}

			return Rect(r.left + pos.x, r.top + pos.y, width, height);
		}
	}

	Event Event::createInvalidatedEvent()
	{
		return Event(Type::Invalidated);
	}

	Event Event::createVisualsInvalidatedEvent()
	{
		return Event(Type::VisualsInvalidated);
	}

	Event Event::createBackendConnectedEvent(Backend& backend)
	{
		Event e(Type::BackendConnected);
		new(&e.backendConnected) BackendConnectedEvent(backend);
		return e;
	}

	Event Event::createMouseEvent(MouseEvent::Subtype subtype, float  x, float y, uint8_t button)
	{
		Event e(Type::MouseMoved);
		switch (subtype)
		{
		case MouseEvent::Subtype::ButtonDown:
		{
			e.type = Type::MouseButtonDown;
			break;
		}
		case MouseEvent::Subtype::ButtonUp:
		{
			e.type = Type::MouseButtonUp;
			break;
		}
		case MouseEvent::Subtype::Left:
		{
			e.type = Type::MouseLeft;
			break;
		}
		}
		new (&e.mouseEvent) MouseEvent(x, y, button);
		return e;
	}

	void Event::dispose()
	{
		switch (type)
		{
		case Guider::Event::None:
		case Guider::Event::Invalidated:
		case Guider::Event::VisualsInvalidated:
			break;
		case Guider::Event::BackendConnected:
		{
			backendConnected.~BackendConnectedEvent();
			break;
		}
		case Guider::Event::MouseMoved:
		case Guider::Event::MouseButtonDown:
		case Guider::Event::MouseButtonUp:
		case Guider::Event::MouseLeft:
		{
			mouseEvent.~MouseEvent();
			break;
		}
		default:
			break;
		}
	}

	Event::Event(const Event& t) : type(t.type)
	{
		switch (t.type)
		{
		case Type::BackendConnected:
		{
			new(&backendConnected) BackendConnectedEvent(t.backendConnected);
			break;
		}
		case Type::MouseButtonDown:
		case Type::MouseButtonUp:
		case Type::MouseMoved:
		case Type::MouseLeft:
		{
			new(&mouseEvent) MouseEvent(t.mouseEvent);
			break;
		}
		default:
			break;
		}
	}

	Event::Event(Type type)
	{
		this->type = type;
	}

	Component::DimensionDesc::DimensionDesc(float value, Mode mode)
	{
		this->value = value;
		this->mode = mode;
	}

	bool Component::isMouseOver() const
	{
		return hasMouseOver;
	}

	bool Component::hasMouseButtonFocus() const
	{
		return hasMouseFocus;
	}

	void Component::setSizingMode(SizingMode w, SizingMode h)
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

	Component::SizingMode Component::getSizingModeVertical() const noexcept
	{
		return sizingModeH;
	}

	Component::SizingMode Component::getSizingModeHorizontal() const noexcept
	{
		return sizingModeW;
	}

	void Component::setParent(Component& p)
	{
		parent = &p;
		if (parent->backend != nullptr)
			handleEvent(Event::createBackendConnectedEvent(*parent->backend));
		invalidate();
	}

	void Component::removeParent()
	{
		parent = nullptr;
		invalidate();
	}

	bool Component::isClean() const noexcept
	{
		return clean;
	}

	bool Component::needsRedraw() const
	{
		return toRedraw;
	}

	void Component::setWidth(float w)
	{
		width = w;
		invalidate();
	}

	void Component::setHeight(float h)
	{
		height = h;
		invalidate();
	}

	void Component::setSize(float w, float h)
	{
		width = w;
		height = h;
		invalidate();
	}

	void Component::setSize(const Vec2& size)
	{
		setSize(size.x, size.y);
	}

	float Component::getWidth() const noexcept
	{
		return width;
	}

	float Component::getHeight() const noexcept
	{
		return height;
	}

	Backend* Component::getBackend() const noexcept
	{
		return backend;
	}

	void Component::setBackend(Backend& b)
	{
		handleEvent(Event::createBackendConnectedEvent(b));
	}

	void Component::poke()
	{
		setClean();
	}

	void Component::onResize(const Rect& bounds)
	{
		invalidateVisuals();
	}

	void Component::onChildStain(Component& c)
	{
	}

	void Component::onChildNeedsRedraw(Component& c)
	{
	}

	bool Component::handleEvent(const Event& event)
	{
		switch (event.type)
		{
		case Event::Type::BackendConnected:
		{
			backend = &event.backendConnected.backend;
			invalidate();
			break;
		}
		case Event::Type::Invalidated:
		{
			invalidate();
			break;
		}
		case Event::Type::VisualsInvalidated:
		{
			invalidateVisuals();
			break;
		}
		case Event::Type::MouseButtonDown:
		case Event::Type::MouseButtonUp:
		case Event::Type::MouseMoved:
		{
			if (getBounds().at(Vec2(0, 0)).contains(Vec2(event.mouseEvent.x, event.mouseEvent.y)))
			{
				setMouseOver();
			}
			else
			{
				resetMouseOver();
			}
			if (event.type == Event::Type::MouseButtonDown)
			{
				setMouseFocus();
			}
			else if (event.type == Event::Type::MouseButtonUp)
			{
				resetMouseFocus();
			}
			break;
		}
		case Event::Type::MouseLeft:
		{
			resetMouseOver();
			break;
		}
		default:
			break;
		}
		if (eventCallback)
			return eventCallback(event);
		return false;
	}

	void Component::invalidate()
	{
		if (clean)
		{
			clean = false;

			Component* p = getParent();
			Component* c = this;

			if (p != nullptr)
			{
				p->onChildStain(*c);
				p->invalidate();
			}
			invalidateVisuals();
		}
	}

	void Component::invalidateVisuals()
	{
		if (!toRedraw)
		{
			toRedraw = true;

			Component* p = getParent();
			Component* c = this;

			if (p != nullptr)
			{
				p->onChildNeedsRedraw(*c);
				p->invalidateVisuals();
			}
		}
	}

	void Component::invalidateRecursive()
	{
		handleEvent(Event::createInvalidatedEvent());
	}

	void Component::invalidateVisualsRecursive()
	{
		handleEvent(Event::createVisualsInvalidatedEvent());
	}

	std::pair<Component::DimensionDesc, Component::DimensionDesc> Component::measure(const DimensionDesc& width, const DimensionDesc& height)
	{
		float w = 0, h = 0;
		Rect pbounds;
		if (parent != nullptr)
			pbounds = parent->getBounds();

		switch (sizingModeW)
		{
		case SizingMode::MatchParent:
		{
			w = pbounds.width;
			break;
		}
		case SizingMode::OwnSize:
		{
			w = getWidth();
			break;
		}
		case SizingMode::GivenSize:
		{
			w = width.value;
			break;
		}
		case SizingMode::WrapContent:
		{
			break;
		}
		}

		switch (sizingModeH)
		{
		case SizingMode::MatchParent:
		{
			h = pbounds.height;
			break;
		}
		case SizingMode::OwnSize:
		{
			h = getHeight();
			break;
		}
		case SizingMode::GivenSize:
		{
			h = height.value;
			break;
		}
		case SizingMode::WrapContent:
		{
			break;
		}
		}
		return std::make_pair(
			DimensionDesc(w, DimensionDesc::Mode::Exact),
			DimensionDesc(h, DimensionDesc::Mode::Exact)
		);
	}
	
	Component* Component::getParent()
	{
		return parent;
	}
	
	const Component* Component::getParent() const
	{
		return parent;
	}
	
	Rect Component::getBounds() const
	{
		return bounds;
	}
	
	Rect Component::getGlobalBounds() const
	{
		Rect b = getBounds();
		if (getParent() != nullptr)
		{
			Rect p = getParent()->getGlobalBounds();
			return Rect(p.left + b.left, p.top + b.top, b.width, b.height);
		}
		return b;
	}
	
	void Component::onMaskDraw(Canvas& canvas) const
	{
		getBackend()->addToMask(bounds.at(Vec2(0.f, 0.f)));
	}
	
	void Component::onRedraw(Canvas& canvas)
	{
		onDraw(canvas);
	}
	
	void Component::drawMask(Canvas& canvas) const
	{
		getBackend()->pushDrawOffset(Vec2(bounds.left, bounds.top));
		getBackend()->pushBounds(bounds.at({ 0.f, 0.f }));
		onMaskDraw(canvas);
		getBackend()->popBounds();
		getBackend()->popDrawOffset();
	}
	
	void Component::draw(Canvas& canvas)
	{
		toRedraw = false;
		getBackend()->pushDrawOffset(Vec2(bounds.left, bounds.top));
		getBackend()->pushBounds(bounds.at(Vec2(0, 0)));
		onDraw(canvas);
		getBackend()->popBounds();
		getBackend()->popDrawOffset();
	}

	void Component::redraw(Canvas& canvas)
	{
		toRedraw = false;
		getBackend()->pushDrawOffset(Vec2(bounds.left, bounds.top));
		getBackend()->pushBounds(bounds.at(Vec2(0, 0)));
		onRedraw(canvas);
		getBackend()->popBounds();
		getBackend()->popDrawOffset();
	}

	std::function<bool(const Event&)> Component::setOnEventCallback(const std::function<bool(const Event&)>& callback)
	{
		auto lf = eventCallback;
		eventCallback = callback;
		return lf;
	}

	Component::Component() :backend(nullptr), parent(nullptr), toRedraw(true), clean(false), hasMouseOver(false), hasMouseFocus(false), sizingModeH(SizingMode::OwnSize), sizingModeW(SizingMode::OwnSize), width(0), height(0)
	{
	}

	void Component::setBounds(Component& c, const Rect& r) const
	{
		Rect lastBounds = c.bounds;
		c.bounds = r;
		c.onResize(lastBounds);
	}

	void Component::setClean()
	{
		clean = true;
	}

	void Component::setMouseOver()
	{
		hasMouseOver = true;
	}

	void Component::resetMouseOver()
	{
		hasMouseOver = false;
	}

	void Component::setMouseFocus()
	{
		hasMouseFocus = true;
	}

	void Component::resetMouseFocus()
	{
		hasMouseFocus = false;
	}


	void Container::invalidateVisuals()
	{
		Component::invalidateVisuals();
		auto it = firstElement();
		while (!it.end())
		{
			it.current().invalidateVisuals();
			it.loadNext();
		}
	}

	bool Container::handleEvent(const Event& event)
	{
		bool r = Component::handleEvent(event);
		for (Iterator it = firstElement(); !it.end(); it.loadNext())
			handleEventForComponent(event, it.current());
		return r;
	}

	Event Container::adjustEventForComponent(const Event& event, Component& component)
	{
		Event copy(event);
		switch (copy.type)
		{
		case Event::Type::MouseButtonDown:
		case Event::Type::MouseButtonUp:
		case Event::Type::MouseMoved:
		case Event::Type::MouseLeft:
		{
			copy.mouseEvent = Event::MouseEvent(copy.mouseEvent, component.getBounds());
			break;
		}
		}
		return copy;
	}

	void Container::handleEventForComponent(const Event& event, Component& component)
	{
		Event copy(event);
		bool shouldHandle = true;
		switch (copy.type)
		{
		case Event::Type::MouseButtonDown:
		case Event::Type::MouseButtonUp:
		case Event::Type::MouseMoved:
		{
			if (component.hasMouseButtonFocus() && copy.type == Event::Type::MouseButtonUp)
				break;
			if (!component.getBounds().contains(Vec2(copy.mouseEvent.x, copy.mouseEvent.y)))
			{
				if (component.isMouseOver())
				{
					copy.type = Event::Type::MouseLeft;
				}
				else
					shouldHandle = false;
			}

			break;
		}
		case Event::Type::MouseLeft:
		{
			shouldHandle = component.isMouseOver();
			break;
		}
		}
		if (shouldHandle)
			component.handleEvent(adjustEventForComponent(copy, component));
	}


	void Engine::addChild(const Component::Type& child)
	{
		Rect bounds = getBounds();
		elements.emplace_back(child);
		child->setParent(*this);
		child->invalidateRecursive();
		child->invalidateVisualsRecursive();
		invalidate();

		toRedraw.insert(child.get());
	}
	
	void Engine::removeChild(const Component::Type& child)
	{
		for (auto it = elements.begin(); it != elements.end(); ++it)
		{
			if (*it == child)
			{
				elements.erase(it);
				return;
			}
		}
	}
	
	void Engine::clearChildren()
	{
		elements.clear();
	}
	
	Container::Iterator Engine::firstElement()
	{
		return createIterator<IteratorType>(elements.begin(), elements.end());
	}
	
	void Engine::onChildNeedsRedraw(Component& c)
	{
		toRedraw.insert(&c);
	}
	
	void Engine::resize(const Vec2& size)
	{
		backend.setSize(size);
		Rect bounds(0, 0, size.x, size.y);
		setBounds(*this, bounds);
		invalidateRecursive();
		invalidateVisuals();
	}
	
	void Engine::update()
	{
		Rect bounds = getBounds();
		for (auto element : elements)
		{
			if (!element->isClean())
			{
				Rect oldRect = element->getBounds();
				std::pair<DimensionDesc, DimensionDesc> measurements = element->measure(
					DimensionDesc(bounds.width, DimensionDesc::Max),
					DimensionDesc(bounds.height, DimensionDesc::Max));

				measurements.first.value = std::min(measurements.first.value, bounds.width);
				measurements.second.value = std::min(measurements.second.value, bounds.height);
				
				Rect newRect(0, 0, measurements.first.value, measurements.second.value);

				if (oldRect != newRect)
				{
					setBounds(*element, newRect);
				}
				element->poke();
			}
		}
	}
	
	void Engine::onMaskDraw(Canvas& canvas) const
	{
		if (!toRedraw.empty())
		{
			for (auto element : elements)
			{
				element->drawMask(canvas);
			}
		}
	}
	
	void Engine::onDraw(Canvas& canvas)
	{
		for (auto element : elements)
		{
			if (toRedraw.count(element.get()))
				element->draw(canvas);
		}
		toRedraw.clear();
	}
	
	void Engine::onRedraw(Canvas& canvas)
	{
		for (auto element : elements)
			element->redraw(canvas);
	}
	
	void Engine::draw()
	{
		if (canvas)
		{
			Canvas* c = canvas.get();
			Vec2 size = backend.getSize();
			Rect bounds(0, 0, size.x, size.y);
			backend.setBounds(bounds);

			backend.setupMask();
			backend.clearMask();

			drawMask(*c);

			backend.useMask();
			Component::draw(*c);

			backend.disableMask();
		}
	}
	
	Engine::Engine(Backend& b) : backend(b)
	{
		canvas = b.getCanvas();
		setBackend(b);
	}
	
	Engine::Engine(Backend& b, const std::shared_ptr<Canvas>& c) : backend(b), canvas(c)
	{
		setBackend(b);
	}
}
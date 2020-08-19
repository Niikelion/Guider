#include <guider/base.hpp>

namespace Guider
{
	bool Rect::operator==(const Rect& t) const noexcept
	{
		return left == t.left && width == t.width && top == t.top && height == t.height;
	}
	bool Rect::operator!=(const Rect& t) const noexcept
	{
		return left != t.left || width != t.width || top != t.top || height != t.height;
	}
	bool Rect::intersects(const Rect& rect) const noexcept
	{
		return	(rect.left <= left + width && rect.left + rect.width >= left) &&
			(rect.top <= top + height && rect.top + rect.height >= top);
	}

	bool Rect::contains(const Vec2& pos) const noexcept
	{
		return left <= pos.x && top <= pos.y && left+width >= pos.x && top+height >= pos.y;
	}

	Rect::Rect()
	{
		left = 0;
		top = 0;
		width = 0;
		height = 0;
	}
	Rect::Rect(float l, float t, float w, float h)
	{
		left = l;
		top = t;
		width = w;
		height = h;
	}

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

	void Backend::limitView(const Rect& rect)
	{
		float left = rect.left > bounds.left ? rect.left : bounds.left;
		float right = rect.left + rect.width < bounds.left + bounds.width ? rect.left + rect.width : bounds.left + bounds.width;
		bounds.left = left;
		float width = right - left;
		bounds.width = width > 0 ? width : 0;

		float top = rect.top > bounds.top ? rect.top : bounds.top;
		float bottom = rect.top + rect.height < bounds.top + bounds.height ? rect.top + rect.height : bounds.top + bounds.height;
		bounds.top = top;
		float height = bottom - top;
		bounds.height = height > 0 ? height : 0;
		setViewport(bounds);
	}
	void Backend::setView(const Rect& rect)
	{
		bounds = rect;
		setViewport(bounds);
	}

	void Backend::drawRectangle(const Rect& rect, const Color& color)
	{
		if (!rectangle)
		{
			rectangle = createRectangle(Vec2(0, 0), Color(0, 0, 0));
		}
		if (rectangle)
		{
			rectangle->setColor(color);
			rectangle->setSize(Vec2(rect.width, rect.height));
			rectangle->draw(Vec2(rect.left, rect.top));
		}
	}

	void Backend::pushDrawOffset(const Vec2& offset)
	{
		Vec2 pos = offset;
		if (!offsets.empty())
			pos += offsets.back();
		offsets.emplace_back(pos);
		setDrawOrigin(pos.x, pos.y);
	}
	void Backend::popDrawOffset()
	{
		offsets.pop_back();
	}

	Event Event::createMouseEvent(MouseEvent::Subtype subtype,float  x, float y, uint8_t button)
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
		new (&e.mouseEvent) MouseEvent(x,y,button);
		return e;
	}

	void Event::dispose()
	{
		switch (type)
		{
		case Guider::Event::None:
			break;
		case Guider::Event::Invalidated:
			break;
		case Guider::Event::BackendConnected:
		{
			backendConnected.~backendConnected();
			break;
		}
		case Guider::Event::MouseMoved:
		case Guider::Event::MouseButtonDown:
		case Guider::Event::MouseButtonUp:
		case Guider::Event::MouseLeft:
		{
			mouseEvent.~mouseEvent();
			break;
		}
		default:
			break;
		}
	}

	Event::Event(const Event& t): type(t.type)
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

	void Component::drawMask(Backend& renderer) const
	{
		renderer.addToMask(getGlobalBounds());
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

	void Component::handleEvent(const Event& event)
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
		case Event::Type::MouseButtonDown:
		case Event::Type::MouseButtonUp:
		case Event::Type::MouseMoved:
		{
			if (getBounds().at(Vec2(0,0)).contains(Vec2(event.mouseEvent.x, event.mouseEvent.y)))
			{
				_setMouseOver();
			}
			break;
		}
		default:
			break;
		}
	}

	void Component::invalidate()
	{
		if (clean)
		{
			clean = false;

			Component* p = getParent();
			Component* c = this;

			while (p != nullptr)
			{
				p->onChildStain(*c);
				c = p;
				p = p->getParent();
			}
			invalidateVisuals();
		}
	}

	void Component::invalidateVisuals()
	{
		if (!redraw)
		{
			redraw = true;

			Component* p = getParent();
			Component* c = this;

			while (p != nullptr)
			{
				p->onChildNeedsRedraw(*c);
				c = p;
				p = p->getParent();
			}
		}
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
	void Component::draw(Backend& renderer) const
	{
		redraw = false;
		renderer.pushDrawOffset(Vec2(bounds.left,bounds.top));
		onDraw(renderer);
		renderer.popDrawOffset();
	}


	void Container::handleEvent(const Event& event)
	{
		Component::handleEvent(event);
		propagateEvent(event);
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
			if (!component.getBounds().contains(Vec2(copy.mouseEvent.x, copy.mouseEvent.y)))
			{
				if (component._getMouseOver())
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
			shouldHandle = component._getMouseOver();
			break;
		}
		}
		if (shouldHandle)
			component.handleEvent(adjustEventForComponent(copy,component));
	}

	void AbsoluteContainer::drawMask(Backend& renderer) const
	{
		for (auto i : toUpdate)
		{
			i->drawMask(renderer);
		}
	}

	void AbsoluteContainer::addChild(const Component::Type& child)
	{
		addChild(child, 0, 0);
	}

	void AbsoluteContainer::addChild(const Component::Type& child, float x, float y)
	{
		Rect bounds = getBounds();
		children.emplace_back(child, x, y);
		child->setParent(*this);
		child->poke();
		toUpdate.insert(child.get());
		std::pair<Component::DimensionDesc, Component::DimensionDesc> measurements = child->measure(
			Component::DimensionDesc(bounds.width, Component::DimensionDesc::Mode::Max),
			Component::DimensionDesc(bounds.height, Component::DimensionDesc::Mode::Max)
		);
		bounds.left = x;
		bounds.top = y;
		bounds.width = measurements.first.value;
		bounds.height = measurements.second.value;
		setBounds(*child, bounds);
	}
	void AbsoluteContainer::removeChild(const Component::Type& child)
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			if (it->component.get() == child.get())
			{
				toUpdate.erase(it->component.get());
				children.erase(it);
			}
		}
	}
	void AbsoluteContainer::clearChildren()
	{
		toUpdate.clear();
		children.clear();
	}
	void AbsoluteContainer::poke()
	{
		Component::poke();
		for (auto& i : children)
		{
			//TODO: move to another method
			i.component->poke();
			Rect bounds = getBounds();
			std::pair<DimensionDesc, DimensionDesc> measurements = i.component->measure(DimensionDesc(bounds.width, DimensionDesc::Max),
				DimensionDesc(bounds.height, DimensionDesc::Max)
			);

			bounds.left = i.x;
			bounds.top = i.y;
			bounds.width = measurements.first.value;
			bounds.height = measurements.second.value;
			setBounds(*i.component.get(), bounds);
			toUpdate.insert(i.component.get());

			for (auto& j : children)
			{
				if (i.component != j.component && bounds.intersects(j.component->getBounds()))
				{
					toUpdate.insert(j.component.get());
				}
			}
		}
	}
	void AbsoluteContainer::onResize(const Rect& last)
	{
		if (last != getBounds())
		{
			for (auto& i : children)
				toUpdate.insert(i.component.get());
		}
	}
	void AbsoluteContainer::onChildStain(Component& c)
	{
		for (auto& i : children)
		{
			if (i.component.get() == &c)
			{
				Rect bounds = getBounds();
				std::pair<DimensionDesc, DimensionDesc> measurements = c.measure(DimensionDesc(bounds.width, DimensionDesc::Max),
					DimensionDesc(bounds.height, DimensionDesc::Max)
				);

				bounds.left = i.x;
				bounds.top = i.y;
				bounds.width = measurements.first.value;
				bounds.height = measurements.second.value;
				setBounds(c, bounds);
				toUpdate.insert(&c);

				for (auto& j : children)
				{
					if (i.component != j.component && bounds.intersects(j.component->getBounds()))
					{
						toUpdate.insert(j.component.get());
					}
				}

				break;
			}
		}
	}
	void AbsoluteContainer::onChildNeedsRedraw(Component& c)
	{
		toUpdate.insert(&c);
	}
	void AbsoluteContainer::propagateEvent(const Event& event)
	{
		for (auto& i : children)
		{
			handleEventForComponent(event, *i.component);
		}
	}
	void AbsoluteContainer::onDraw(Backend& renderer) const
	{
		if (toUpdate.size() > 0)
		{
			Rect bounds = getGlobalBounds();
			std::vector<Rect> base;
			base.reserve(toUpdate.size());
			for (auto i : toUpdate)
			{
				base.emplace_back(i->getBounds());
			}
			for (const auto& i : children)
			{
				Rect localBounds = i.component->getBounds();
				for (const auto& rect : base)
				{
					if (localBounds.intersects(rect))
					{
						i.component->draw(renderer);
						break;
					}
				}
			}
		}
	}
	AbsoluteContainer::Element::Element(const Component::Type& c, float x, float y)
	{
		component = c;
		this->x = x;
		this->y = y;
	}
	void Engine::resize(const Vec2& size)
	{
		backend.setSize(size);
		Rect bounds(0, 0, size.x, size.y);
		setBounds(container, bounds);
	}
	void Engine::update()
	{
		container.poke();
	}
	void Engine::onDraw(Backend& renderer) const
	{
		container.onDraw(renderer);
	}
	void Engine::draw() const
	{
		Vec2 size = backend.getSize();
		Rect bounds(0, 0, size.x, size.y);
		backend.setView(bounds);

		backend.setupMask();
		backend.clearMask();
		
		container.drawMask(backend);

		backend.useMask();
		
		container.onDraw(backend);

		backend.disableMask();
	}
}
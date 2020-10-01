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
	void Backend::popDrawOffset()
	{
		offsets.pop_back();
	}

	namespace Resources
	{
		void CompositeDrawable::draw(Canvas& canvas, const Rect& bounds)
		{
			for (auto& sub : elements)
			{
				Rect b = sub.padding.calcContentArea(bounds);
				//TODO: apply gravity
				sub.drawable->draw(canvas, bounds);
			}
		}
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
	void Component::draw(Canvas& canvas) const
	{
		redraw = false;
		getBackend()->pushDrawOffset(Vec2(bounds.left,bounds.top));
		onDraw(canvas);
		getBackend()->popDrawOffset();
	}


	void Container::handleEvent(const Event& event)
	{
		Component::handleEvent(event);
		Iterator it = firstElement();
		while (!it.end())
		{
			it.current().handleEvent(adjustEventForComponent(event,it.current()));
			it.loadNext();
		}
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
	Container::Iterator AbsoluteContainer::firstElement()
	{
		return createIterator<IteratorType>(children.begin(),children.end());
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

	void AbsoluteContainer::onDraw(Canvas& canvas) const
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
						i.component->draw(canvas);
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
	void Engine::addChild(const Component::Type& child)
	{
		Rect bounds = getBounds();
		elements.emplace_back(child);
		child->setParent(*this);
		child->poke();
		std::pair<Component::DimensionDesc, Component::DimensionDesc> measurements = child->measure(
			Component::DimensionDesc(bounds.width, Component::DimensionDesc::Mode::Max),
			Component::DimensionDesc(bounds.height, Component::DimensionDesc::Mode::Max)
		);
		bounds.left = 0;
		bounds.top = 0;
		bounds.width = measurements.first.value;
		bounds.height = measurements.second.value;
		setBounds(*child, bounds);
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
		return createIterator<IteratorType>(elements.begin(),elements.end());
	}
	void Engine::resize(const Vec2& size)
	{
		backend.setSize(size);
		Rect bounds(0, 0, size.x, size.y);
		setBounds(*this, bounds);
		for (auto element : elements)
		{
			Rect oldRect = element->getBounds();
			element->poke();
			std::pair<DimensionDesc, DimensionDesc> measurements = element->measure(
				DimensionDesc(bounds.width, DimensionDesc::Max),
				DimensionDesc(bounds.height, DimensionDesc::Max));

			measurements.first.value = std::min(measurements.first.value, bounds.width);
			measurements.second.value = std::min(measurements.second.value, bounds.height);

			if (measurements.first.value != oldRect.width || measurements.second.value != oldRect.height)
			{
				setBounds(*element, Rect(0, 0, measurements.first.value, measurements.second.value));
			}

		}
	}
	void Engine::update()
	{
		Rect bounds = getBounds();
		for (auto element : elements)
		{
			if (!element->isClean())
			{
				Rect oldRect = element->getBounds();
				element->poke();
				std::pair<DimensionDesc, DimensionDesc> measurements = element->measure(
					DimensionDesc(bounds.width, DimensionDesc::Max),
					DimensionDesc(bounds.height, DimensionDesc::Max));

				measurements.first.value = std::min(measurements.first.value,bounds.width);
				measurements.second.value = std::min(measurements.second.value, bounds.height);

				if (measurements.first.value != oldRect.width || measurements.second.value != oldRect.height)
				{
					setBounds(*element, Rect(0, 0, measurements.first.value, measurements.second.value));
				}
			}
		}
	}
	void Engine::onDraw(Canvas& canvas) const
	{
		//TODO: redraw only visually invalidated elements
		for (auto element : elements)
		{
			element->draw(canvas);
		}
	}
	void Engine::draw() const
	{
		if (canvas)
		{
			Canvas* c = canvas.get();
			Vec2 size = backend.getSize();
			Rect bounds(0, 0, size.x, size.y);
			backend.setBounds(bounds);

			backend.setupMask();
			backend.clearMask();

			Container::drawMask(backend);

			backend.useMask();
			Component::draw(*c);

			backend.disableMask();
		}
	}
}
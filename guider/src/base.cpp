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

	void RenderBackend::limitView(const Rect& rect)
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
	void RenderBackend::setView(const Rect& rect)
	{
		bounds = rect;
		setViewport(bounds);
	}


	Component::DimensionDesc::DimensionDesc(float value, Mode mode)
	{
		this->value = value;
		this->mode = mode;
	}

	void Component::poke()
	{
	}

	void Component::onResize(const Rect& bounds)
	{
	}

	void Component::onChildStain(Component& c)
	{
	}

	void Component::onChildNeedsRedraw(Component& c)
	{
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

			if (p != nullptr)
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
	void Component::draw(RenderBackend& renderer) const
	{
		redraw = false;
		onDraw(renderer);
	}

	void AbsoluteContainer::addChild(const Component::Type& child, float x, float y)
	{
		Rect bounds = getBounds();
		children.emplace_back(child, x, y);
		child->setParent(*this);
		toUpdate.insert(child.get());
		std::pair<Component::DimensionDesc, Component::DimensionDesc> measurements = child->measure(
			Component::DimensionDesc(bounds.width, Component::DimensionDesc::Mode::Max),
			Component::DimensionDesc(bounds.height, Component::DimensionDesc::Mode::Max)
		);
		bounds.left = x;
		bounds.top = y;
		bounds.width = measurements.first.value;
		bounds.height = measurements.second.value;
		setClean(*child);
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
	void AbsoluteContainer::onDraw(RenderBackend& renderer) const
	{
		if (toUpdate.size() > 0)
		{
			Rect bounds = getGlobalBounds();
			std::vector<Rect> base;
			base.reserve(toUpdate.size());
			renderer.pushMaskLayer();
			renderer.setupMask();
			for (auto i : toUpdate)
			{
				Rect globalBounds = i->getGlobalBounds();
				renderer.addToMask(globalBounds);
				base.emplace_back(globalBounds);
			}
			renderer.useMask();
			for (const auto& i : children)
			{
				Rect localBounds = i.component->getBounds();
				for (const auto& rect : base)
				{
					if (localBounds.intersects(rect))
					{
						renderer.setDrawOrigin(i.x, i.y);
						i.component->draw(renderer);
						break;
					}
				}
			}
			renderer.popMaskLayer();
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
	void Engine::onDraw(RenderBackend& renderer) const
	{
		container.onDraw(renderer);
	}
	void Engine::draw() const
	{
		Vec2 size = backend.getSize();
		Rect bounds(0, 0, size.x, size.y);
		backend.setView(bounds);
		backend.clearMask();
		backend.disableMask();
		container.onDraw(backend);
	}
}
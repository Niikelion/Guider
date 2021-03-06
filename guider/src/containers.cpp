#include <guider/containers.hpp>
#include <queue>
#include <cstdlib>
#include <limits>
#include <cassert>
#include <map>

namespace Guider
{
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
		return createIterator<IteratorType>(children.begin(), children.end());
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

	void AbsoluteContainer::onMaskDraw(Canvas& canvas) const
	{
		for (auto i : toUpdate)
		{
			i->drawMask(canvas);
		}
	}
	void AbsoluteContainer::onDraw(Canvas& canvas)
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

	bool ListContainer::updateElementRect(Element& element, bool needsMeasure, float localOffset, const DimensionDesc& w, const DimensionDesc& h, const Rect& bounds)
	{
		Rect lb = element.component->getBounds();
		Rect original = lb;

		element.offset = localOffset;

		if (horizontal)
		{
			lb.left = element.offset + offset;
		}
		else
		{
			lb.top = element.offset + offset;
		}

		if (needsMeasure)
		{
			std::pair<DimensionDesc, DimensionDesc> measurements = element.component->measure(w, h);

			float width = measurements.first.value;
			float height = measurements.second.value;

			if (horizontal)
			{
				if (height > bounds.height)
					height = bounds.height;
			}
			else
			{
				if (width > bounds.width)
					width = bounds.width;
			}
			lb.width = width;
			lb.height = height;
			if (horizontal)
			{
				lb.top = (bounds.height - height) / 2;
				element.size = lb.width;
			}
			else
			{
				lb.left = (bounds.width - width) / 2;
				element.size = lb.height;
			}
		}
		if (lb != original)
		{
			if (std::abs(lb.left - original.left) > std::numeric_limits<float>::epsilon() ||
				std::abs(lb.top - original.top) > std::numeric_limits<float>::epsilon() ||
				lb.width != original.width || lb.height != original.height)
			{
				setBounds(*element.component, lb);
				return true;
			}
		}
		return false;
	}
	void ListContainer::setOrientation(bool horizontal)
	{
		this->horizontal = horizontal;
		//TODO: recalc offsets
		invalidate();
	}
	void ListContainer::setBackgroundColor(const Color& color)
	{
		backgroundColor = color;
		if (backgroundColor.a > 0)
			backgroundColor.a = 255;
		invalidateVisuals();
		firstDraw = true;
	}
	void ListContainer::addChild(const Component::Type& child)
	{
		child->setParent(*this);
		child->poke();
		float off;
		Rect bounds = getBounds();

		DimensionDesc w(bounds.width, DimensionDesc::Max);
		DimensionDesc h(bounds.height, DimensionDesc::Max);

		if (horizontal)
		{
			w.value = 0;
			w.mode = DimensionDesc::Min;
		}
		else
		{
			h.value = 0;
			h.mode = DimensionDesc::Min;
		}

		std::pair<DimensionDesc, DimensionDesc> measurements = child->measure(w, h);

		Rect lb;

		if (horizontal)
		{
			off = measurements.first.value;
			float height = measurements.second.value;
			if (height > bounds.height)
				height = bounds.height;
			lb = Rect(size,(bounds.height-height)/2,off,height);
		}
		else
		{
			off = measurements.second.value;
			float width = measurements.first.value;
			if (width > bounds.width)
				width = bounds.width;
			lb = Rect((bounds.width - width) / 2,size, width, off);
		}

		children.emplace_back(child,off,size);
		toUpdate.insert(child.get());
		toRedraw.insert(child.get());

		size += off;

		invalidate();
	}
	void ListContainer::removeChild(const Component::Type& child)
	{
		auto it = children.begin();
		while (it != children.end())
		{
			if (it->component.get() == child.get())
			{
				auto cp = it;
				cp++;
				while (cp != children.end())//todo: stop after reaching end of parent frame
				{
					toRedraw.insert(cp->component.get());
					toOffset.insert(cp->component.get());
					++cp;
				}
				children.erase(it);
				invalidate();
				return;
			}
			++it;
		}
	}
	void ListContainer::removeChild(unsigned n)
	{
		removeChild(std::next(children.begin(),n)->component);
	}
	void ListContainer::clearChildren()
	{
		children.clear();
		toUpdate.clear();
		toRedraw.clear();
		toOffset.clear();
	}
	size_t ListContainer::getChildrenCount() const
	{
		return children.size();
	}
	Component::Type ListContainer::getChild(unsigned i)
	{
		return std::next(children.begin(),i)->component;
	}
	void ListContainer::onMaskDraw(Canvas& canvas) const
	{
		if (backgroundColor.a > 0 && firstDraw)
		{
			Component::onMaskDraw(canvas);
		}
		else
		{
			for (auto i : toRedraw)
			{
				i->drawMask(canvas);
			}
			Rect bounds = getBounds().at(Vec2(0.f, 0.f));
			float d = 0;
			if (horizontal)
			{
				d = bounds.width;
				bounds.left += offset + size;
			}
			else
			{
				d = bounds.height;
				bounds.top += offset + size;
			}
			if (offset + size < d)
			{
				getBackend()->addToMask(bounds);
			}
		}
	}

	void ListContainer::onDraw(Canvas& canvas)
	{
		onRedraw(canvas);
		return;

		if (backgroundColor.a > 0 && firstDraw)
			canvas.drawRectangle(getBounds().at(Vec2(0.f, 0.0f)), backgroundColor);
		for (const auto& i : children)
		{
			if (backgroundColor.a > 0 || toRedraw.count(i.component.get()))
			{
				i.component->draw(canvas);
			}
		}
		toRedraw.clear();
	}

	void ListContainer::onRedraw(Canvas& canvas)
	{
		if (backgroundColor.a > 0)
			canvas.drawRectangle(getBounds().at(Vec2(0.f, 0.0f)), backgroundColor);
		for (const auto& i : children)
			i.component->redraw(canvas);
		toRedraw.clear();
		firstDraw = false;
	}

	void ListContainer::poke()
	{
		Component::poke();
		float off = 0;
		bool offsetting = false;

		Rect bounds = getBounds();
		DimensionDesc w(bounds.width, DimensionDesc::Max);
		DimensionDesc h(bounds.height, DimensionDesc::Max);

		if (horizontal)
		{
			w.value = 0;
			w.mode = DimensionDesc::Min;
		}
		else
		{
			h.value = 0;
			h.mode = DimensionDesc::Min;
		}

		for (auto& i:children)
		{
			bool needsMeasureing = !i.component->isClean();
			if (!i.component->isClean())
				i.component->poke();

			if (offsetting)
			{
				updateElementRect(i, needsMeasureing, off, w, h, bounds);
			}
			else if (needsMeasureing)
			{
				updateElementRect(i, true, off, w, h, bounds);
				offsetting = true;
			}
			off += i.size;
		}
		size = off;
		toUpdate.clear();
	}

	void ListContainer::onResize(const Rect& lastBounds)
	{
		Rect bounds = getBounds();
		
		if (bounds != lastBounds)
		{//TODO: remeasure only if logical width changes
			float off = 0;

			DimensionDesc w(bounds.width, DimensionDesc::Max);
			DimensionDesc h(bounds.height, DimensionDesc::Max);

			if (horizontal)
			{
				w.value = 0;
				w.mode = DimensionDesc::Min;
			}
			else
			{
				h.value = 0;
				h.mode = DimensionDesc::Min;
			}
			for (auto& i : children)
			{
				updateElementRect(i, true, off, w, h, bounds);
				off += i.size;
			}
			size = off;
			invalidate();

			toOffset.clear();
			for (auto& i : children)
				i.component->invalidateVisuals();

			firstDraw = true;
		}
	}

	void ListContainer::onChildStain(Component& c)
	{
		toUpdate.insert(&c);
	}

	void ListContainer::onChildNeedsRedraw(Component& c)
	{
		toRedraw.insert(&c);
	}

	ListContainer::Iterator ListContainer::firstElement()
	{
		return createIterator<IteratorType>(children.begin(),children.end());
	}

	std::pair<Component::DimensionDesc, Component::DimensionDesc> ListContainer::measure(const DimensionDesc& w, const DimensionDesc& h)
	{
		//TODO: move to function i guess
		float off = 0;
		bool offsetting = false;

		Rect bounds = getBounds();
		DimensionDesc ww(bounds.width, DimensionDesc::Max);
		DimensionDesc hh(bounds.height, DimensionDesc::Max);

		if (horizontal)
		{
			ww.value = 0;
			ww.mode = DimensionDesc::Min;
		}
		else
		{
			hh.value = 0;
			hh.mode = DimensionDesc::Min;
		}

		for (auto& i : children)
		{
			bool needsMeasureing = !i.component->isClean();
			if (!i.component->isClean())
				i.component->poke();

			if (offsetting)
			{
				updateElementRect(i, needsMeasureing, off, ww, hh, bounds);
			}
			else if (needsMeasureing)
			{
				updateElementRect(i, true, off, ww, hh, bounds);
				offsetting = true;
			}
			off += i.size;
		}
		size = off;

		std::pair<DimensionDesc, DimensionDesc> measurements = Component::measure(w,h);
		if (getSizingModeHorizontal() == SizingMode::WrapContent && horizontal)
		{
			measurements.first = DimensionDesc(size, DimensionDesc::Exact);
		}
		else if (getSizingModeVertical() == SizingMode::WrapContent && !horizontal)
		{
			measurements.second = DimensionDesc(size, DimensionDesc::Exact);
		}
		return measurements;
	}

	ListContainer::ListContainer() : horizontal(false), size(0), offset(0), backgroundColor(0, 0, 0, 0), firstDraw(true)
	{
	}

	ListContainer::ListContainer(Manager& manager, const XML::Tag& config, const StylingPack& style) : ListContainer()
	{
		Manager::handleDefaultArguments(*this, config, style.style);

		XML::Value tmp = config.getAttribute("orientation");
		if (tmp.exists())
		{
			if (tmp.val == "horizontal")
				setOrientation(true);
			else if (tmp.val == "vertical")
				setOrientation(false);
		}

		for (const auto& child : config.children)
		{
			if (!child->isTextNode())
			{
				XML::Tag& tag = static_cast<XML::Tag&>(*child);
				Component::Type t = manager.instantiate(tag, style.theme);

				addChild(t);
			}
		}
	}

	std::vector<Component*> ConstraintsContainer::Constraint::getDeps() const
	{
		std::vector<Component*> ret;

		switch (getType())
		{
		case Type::Regular:
		{
			if (regular.left != nullptr)
				ret.push_back(regular.left);
			if (regular.right != nullptr)
				ret.push_back(regular.right);
			break;
		}
		case Type::Chain:
		{
			if (chain.left != nullptr)
				ret.push_back(chain.left);
			if (chain.right != nullptr)
				ret.push_back(chain.right);
			break;
		}
		}

		return std::move(ret);
	}
	bool ConstraintsContainer::Constraint::isFor(const Component& c) const
	{
		switch (getType())
		{
		case Type::Regular:
			return regular.target == &c;
		case Type::Chain:
		{
			for (unsigned i = 2; i < chain.targets.size(); ++i)
			{
				if (chain.targets[i].first == &c)
					return true;
			}
			return false;
		}
		default:
			return false;
		}
	}
	ConstraintsContainer::Constraint::Constraint(Type type, Orientation o)
	{
		flags = 0;
		setOrientation(o);
		setType(type);
		switch (type)
		{
		case Type::Regular:
		{
			new (&regular) RegularConstraintData();
			break;
		}
		case Type::Chain:
		{
			new (&chain) ChainConstraintData();
			break;
		}
		}
	}

	ConstraintsContainer::Constraint::Constraint(Constraint&& t) noexcept
	{
		switch (getType())
		{
		case Type::Regular:
		{
			regular.~RegularConstraintData();
			break;
		}
		case Type::Chain:
		{
			chain.~ChainConstraintData();
			break;
		}
		}

		switch (t.getType())
		{
		case Type::Regular:
		{
			new (&regular) RegularConstraintData(std::move(t.regular));
			break;
		}
		case Type::Chain:
		{
			new (&chain) ChainConstraintData(std::move(t.chain));
			break;
		}
		}

		constOffset = t.constOffset;
		flags = t.flags;
		t.flags = 0;
	}
	ConstraintsContainer::Constraint::~Constraint()
	{
		switch (getType())
		{
		case Type::Regular:
		{
			regular.~RegularConstraintData();
			break;
		}
		case Type::Chain:
		{
			chain.~ChainConstraintData();
			break;
		}
		}
	}

	void ConstraintsContainer::RegularConstraintBuilder::setSize(float size)
	{
		constraint.regular.size = size;
	}

	void ConstraintsContainer::RegularConstraintBuilder::setFlow(float flow)
	{
		constraint.regular.flow = flow;
	}

	void ConstraintsContainer::RegularConstraintBuilder::attachStartTo(const Component::Type& target, bool toStart, float offset)
	{
		constraint.regular.leftOffset = offset;
		constraint.setFirstEdge(toStart);

		Component* prevLeft = constraint.regular.left;

		constraint.regular.left = target.get();
		cluster->dependencies[constraint.regular.left]++;

		if (prevLeft != nullptr)
		{
			cluster->dependencies.at(prevLeft)--;
			if (cluster->dependencies.at(prevLeft) == 0)
				cluster->dependencies.erase(prevLeft);
		}
	}

	void ConstraintsContainer::RegularConstraintBuilder::attachEndTo(const Component::Type& target, bool toStart, float offset)
	{
		constraint.regular.rightOffset = offset;
		constraint.setSecondEdge(toStart);

		Component* prevRight = constraint.regular.right;

		constraint.regular.right = target.get();
		cluster->dependencies[constraint.regular.right]++;

		if (prevRight != nullptr)
		{
			cluster->dependencies.at(prevRight)--;
			if (cluster->dependencies.at(prevRight) == 0)
				cluster->dependencies.erase(prevRight);
		}
	}

	void ConstraintsContainer::ChainConstraintBuilder::attachStartTo(const Component::Type& target, bool toStart, float offset)
	{
		constraint.chain.leftOffset = offset;
		constraint.setFirstEdge(toStart);

		Component* prevLeft = constraint.chain.left;

		constraint.chain.left = target.get();
		cluster->dependencies[constraint.chain.left]++;

		if (prevLeft != nullptr)
		{
			cluster->dependencies.at(prevLeft)--;
			if (cluster->dependencies.at(prevLeft) == 0)
				cluster->dependencies.erase(prevLeft);
		}
	}

	void ConstraintsContainer::ChainConstraintBuilder::attachEndTo(const Component::Type& target, bool toStart, float offset)
	{
		constraint.chain.rightOffset = offset;
		constraint.setSecondEdge(toStart);

		Component* prevRight = constraint.chain.right;

		constraint.chain.right = target.get();
		cluster->dependencies[constraint.chain.right]++;

		if (prevRight != nullptr)
		{
			cluster->dependencies.at(prevRight)--;
			if (cluster->dependencies.at(prevRight) == 0)
				cluster->dependencies.erase(prevRight);
		}
	}

	float ConstraintsContainer::getEdge(Component* c, Constraint::Edge e)
	{
		Rect rect;
		if (c == nullptr)
		{
			rect = Rect(0, 0, 0, 0);
		}
		else
		{
			rect = boundaries[c];
		}
		if (c == this)
		{
			rect.left = 0;
			rect.top = 0;
		}

		switch (e)
		{
		case Constraint::Edge::Left: return rect.left;
			break;
		case Constraint::Edge::Right: return rect.left + rect.width;
			break;
		case Constraint::Edge::Top: return rect.top;
			break;
		case Constraint::Edge::Bottom: return rect.top + rect.height;
			break;
		}
		return 0;
	}
	bool ConstraintsContainer::reorderClusters()
	{
		updated.clear();
		size_t n = clusters.size();
		//number of references pointing to me
		std::unordered_map<std::list<Cluster>::iterator, unsigned, ClusterHash> ndeps;

		std::unordered_map<std::list<Cluster>::iterator, std::unordered_set<std::list<Cluster>::iterator, ClusterHash>, ClusterHash> clusterDependencies;

		for (auto it = clusters.begin(); it != clusters.end(); ++it)
		{
			ndeps[it] = 0;
		}

		for (auto cluster = clusters.begin(); cluster != clusters.end(); ++cluster)
		{
			for (const auto& dep : cluster->dependencies)
			{
				auto mapping = clusterMapping.find(dep.first);
				
				if (mapping != clusterMapping.end())
				{
					ndeps[mapping->second]++;
					clusterDependencies[cluster].insert(mapping->second);
				}
				else if (dep.first != this)
					throw std::runtime_error("Internal error");
			}
		}

		// Create an queue and enqueue all vertices with 
		// depth 0 
		std::queue<std::list<Cluster>::iterator> q;

		for (auto it = clusters.begin(); it != clusters.end(); ++it)
		{
			if (ndeps[it] == 0)
				q.push(it);
		}

		unsigned cnt = 0;

		std::unordered_set<std::list<Cluster>::iterator, ClusterHash> visited;
		std::vector<Cluster*> ret;

		ret.reserve(n);

		while (!q.empty())
		{
			auto front = q.front();
			q.pop();
			ret.emplace_back(&(*front));
			auto& clusterDependency = clusterDependencies[front];
			for (auto it : clusterDependency)
				if (--ndeps[it] == 0)
					q.push(it);

			cnt++;
		}

		if (cnt != n)
		{
			return false;
		}

		std::list<Cluster> tmp;

		for (auto i : ret)
		{
			for (auto j : i->components)
			{
				updated.insert(j);
			}
			tmp.emplace_back(std::move(const_cast<Cluster&>(*i)));
		}

		clusters = std::move(tmp);

		return true;
	}
	void ConstraintsContainer::solveConstraint(const Constraint& c, Constraint::PassType pass)
	{
		switch (c.getType())
		{
		case Constraint::Type::Regular:
		{
			Component* start = c.regular.left;
			float soff = c.regular.leftOffset;
			Component* end = c.regular.right;
			float eoff = c.regular.rightOffset;

			float s = getEdge(start, c.getFirstEdge()) + soff;
			float e = getEdge(end, c.getSecondEdge()) - eoff;

			if (s > e)
			{
				float tmp = s;
				s = e;
				e = tmp;
			}

			Rect& out = boundaries[c.regular.target];

			switch (pass)
			{
			case Constraint::PassType::First:
			{
				float size = 0;
				if (c.constOffset)
				{
					size = e - s;
				}
				else
				{
					size = c.regular.size;
					s = s * (1 - c.regular.flow) + (e - size) * c.regular.flow;
				}
				if (c.getOrientation() == Constraint::Orientation::Horizontal)
				{
					out.left = s;
					out.width = size;
				}
				else
				{
					out.top = s;
					out.height = size;
				}
				break;
			}
			case Constraint::PassType::Final:
			{
				if (c.getOrientation() == Constraint::Orientation::Horizontal)
				{
					out.left = s * (1 - c.regular.flow) + (e - out.width) * c.regular.flow;
				}
				else
				{
					out.top = s * (1 - c.regular.flow) + (e - out.height) * c.regular.flow;
				}
				break;
			}
			}
			break;
		}
		case Constraint::Type::Chain:
		{
			if (c.chain.targets.size() < 3)
				return;
			Component* start = c.chain.left;
			float soff = c.chain.leftOffset;
			Component* end = c.chain.right;
			float eoff = c.chain.rightOffset;

			float s = getEdge(start, c.getFirstEdge()) + soff;
			float e = getEdge(end, c.getSecondEdge()) - eoff;

			if (s > e)
			{
				float tmp = s;
				s = e;
				e = tmp;
			}

			float width = 0;
			for (unsigned j = 0; j < c.chain.targets.size(); ++j)
			{
				switch (pass)
				{
				case Constraint::PassType::First:
				{
					width += c.chain.targets[j].second;
					break;
				}
				case Constraint::PassType::Final:
				{
					Rect& bounds = boundaries[c.chain.targets[j].first];
					if (c.getOrientation() == Constraint::Orientation::Horizontal)
					{
						width += bounds.width;
					}
					else
					{
						width += bounds.height;
					}
					break;
				}
				}
			}

			size_t n = c.chain.targets.size();
			float spacing = c.chain.spacing;
			float totalSpacing = spacing * (n - 1);

			if (c.constOffset)
			{
				float d = e - s;


				float ratio = 0;


				if (width < 0.000001f)
				{
					width = 0.000001f;
				}
				if (totalSpacing < d && n > 1)
				{
					spacing = d / (n - 1);
				}
				else
				{
					ratio = (d - totalSpacing) / width;
				}

				float sp = s;

				for (unsigned i = 0; i < c.chain.targets.size(); ++i)
				{
					float size = 0;

					switch (pass)
					{
					case Constraint::PassType::First:
					{
						size = c.chain.targets[i].second * ratio;
						break;
					}
					case Constraint::PassType::Final:
					{
						Rect& bounds = boundaries[c.chain.targets[i].first];
						if (c.getOrientation() == Constraint::Orientation::Horizontal)
						{
							size = bounds.width;
						}
						else
						{
							size = bounds.height;
						}
						break;
					}
					}

					if (c.getOrientation() == Constraint::Orientation::Horizontal)
					{
						boundaries[c.chain.targets[i].first].left = sp;
						boundaries[c.chain.targets[i].first].width = size;
					}
					else
					{
						boundaries[c.chain.targets[i].first].top = sp;
						boundaries[c.chain.targets[i].first].height = size;
					}
					sp += spacing + size;
				}
			}
			else
			{
				float center = (s + e) / 2;

				float sp = center - (width + totalSpacing) / 2;

				for (unsigned i = 0; i < c.chain.targets.size(); ++i)
				{
					float size = 0;

					switch (pass)
					{
					case Guider::ConstraintsContainer::Constraint::PassType::First:
					{
						size = c.chain.targets[i].second;
						break;
					}
					case Guider::ConstraintsContainer::Constraint::PassType::Final:
					{
						Rect& bounds = boundaries[c.chain.targets[i].first];
						if (c.getOrientation() == Constraint::Orientation::Horizontal)
						{
							size = bounds.width;
						}
						else
						{
							size = bounds.height;
						}
						break;
					}
					}

					if (c.getOrientation() == Constraint::Orientation::Horizontal)
					{
						boundaries[c.chain.targets[i].first].left = sp;
						boundaries[c.chain.targets[i].first].width = size;
					}
					else
					{
						boundaries[c.chain.targets[i].first].top = sp;
						boundaries[c.chain.targets[i].first].height = size;
					}

					sp += spacing + size;
				}
			}
			break;
		}
		}
	}
	void ConstraintsContainer::solveCluster(const Cluster& cluster)
	{
		if (cluster.constraints.size() == 0)
			return;

		for (const auto constraint : cluster.constraints) // calculate maximal bounds
		{
			solveConstraint(*constraint, Constraint::PassType::First);
		}
		for (const auto i : cluster.components) // ask elements for their bounds
		{
			if (i != this)
			{
				Rect& bounds = boundaries[i];
				std::pair<Component::DimensionDesc, Component::DimensionDesc> measures = i->measure(
					Component::DimensionDesc(bounds.width, Component::DimensionDesc::Mode::Max),
					Component::DimensionDesc(bounds.height, Component::DimensionDesc::Mode::Max)
				);
				if (measures.first.value < bounds.width)
					bounds.width = measures.first.value;
				if (measures.second.value < bounds.height)
					bounds.height = measures.second.value;
			}
		}
		for (const auto constraint : cluster.constraints) // calculate final bounds
		{
			solveConstraint(*constraint, Constraint::PassType::Final);
		}
	}
	void ConstraintsContainer::solveConstraints()
	{
		Rect bounds = getBounds();
		solveConstraints(DimensionDesc(bounds.width, DimensionDesc::Exact), DimensionDesc(bounds.height, DimensionDesc::Exact));
	}
	void ConstraintsContainer::solveConstraints(const DimensionDesc& w, const DimensionDesc& h)
	{
		if (messyClusters)
		{
			if (!reorderClusters())
				return;
			messyClusters = false;
		}
		std::unordered_map<Component*, Rect> lastBoundaries = boundaries;

		Rect bounds;
		Rect parentBounds;

		if (getParent() != nullptr)
		{
			parentBounds = getParent()->getBounds();
			bounds.width = parentBounds.width;
			bounds.height = parentBounds.height;
		}

		switch (getSizingModeHorizontal())
		{
		case SizingMode::MatchParent:
		{
			bounds.width = parentBounds.width;
			break;
		}
		case SizingMode::OwnSize:
		{
			bounds.width = getWidth();
			break;
		}
		default:
			break;
		}

		switch (getSizingModeVertical())
		{
		case SizingMode::MatchParent:
		{
			bounds.height = parentBounds.height;
			break;
		}
		case SizingMode::OwnSize:
		{
			bounds.height = getHeight();
			break;
		}
		default:
			break;
		}

		switch (w.mode)
		{
		case DimensionDesc::Mode::Exact:
			bounds.width = w.value; break;
		case DimensionDesc::Mode::Max:
			bounds.width = bounds.width <= w.value ? bounds.width : w.value; break;
		case DimensionDesc::Mode::Min:
			bounds.width = bounds.width >= w.value ? bounds.width : w.value; break;
		}

		switch (h.mode)
		{
		case DimensionDesc::Mode::Exact:
			bounds.height = h.value; break;
		case DimensionDesc::Mode::Max:
			bounds.height = bounds.height <= h.value ? bounds.height : h.value; break;
		case DimensionDesc::Mode::Min:
			bounds.height = bounds.height >= h.value ? bounds.height : h.value; break;
		}

		boundaries[this] = bounds;

		for (const auto& cluster : clusters)
		{
			solveCluster(cluster);
		}
	}
	void ConstraintsContainer::applyConstraints()
	{
		for (auto it = children.begin(); it != children.end(); ++it)
		{
			auto child = *it;
			Component* p = child.get();
			if (p->getBounds() != boundaries[p])
			{
				drawnLastFrame[p] = p->getBounds();
				setBounds(*p, boundaries[p]);
			}

		}
		invalidLayout = false;
	}
	void ConstraintsContainer::registerProperties(Manager& m, const std::string& name)
	{
		m.registerPropertyForComponent<Color>(name,"backgroundColor", (Color(*)(const std::string&))Styles::strToColor);
	}
	void ConstraintsContainer::setBackgroundColor(const Color& color)
	{
		backgroundColor = color;
		if (backgroundColor.a > 0)
			backgroundColor.a = 255;
		firstDraw = true;
		invalidateVisuals();
	}
	void ConstraintsContainer::poke()
	{
		Component::poke();
		for (auto& i : children)
		{
			if (!i->isClean())
				i->poke();
		}
		if (invalidLayout)
		{
			solveConstraints();
			applyConstraints();
		}
	}
	void ConstraintsContainer::onResize(const Rect& bounds)
	{
		if (getBounds() != bounds || invalidLayout)
		{
			firstDraw = true;
			solveConstraints();
			invalidLayout = false;
			applyConstraints();
			for (auto& i : children)
				i->invalidateVisuals();
			invalidateVisuals();
		}
	}
	void ConstraintsContainer::onChildStain(Component& c)
	{
		Rect bounds = boundaries[&c];
		std::pair<DimensionDesc, DimensionDesc> measurements = c.measure(DimensionDesc(bounds.width, DimensionDesc::Exact), DimensionDesc(bounds.height, DimensionDesc::Exact));
		if (measurements.first.value != bounds.width || measurements.second.value != bounds.height)
		{
			invalidLayout = true;
		}
	}
	void ConstraintsContainer::onChildNeedsRedraw(Component& c)
	{
		needsRedraw.insert(&c);
	}

	void ConstraintsContainer::handleEvent(const Event& event)
	{
		if (event.type == Event::Type::BackendConnected)
			invalidLayout = true;
		Container::handleEvent(event);
	}

	std::pair<Component::DimensionDesc, Component::DimensionDesc> ConstraintsContainer::measure(const DimensionDesc& w, const DimensionDesc& h)
	{
		Rect bounds = boundaries[this];
		float ws = bounds.width, hs = bounds.height;
		auto measurements = Component::measure(w, h);
		bool recalcLayout = false;
		if (getSizingModeVertical() == SizingMode::WrapContent)
		{
			if (canWrapW)
			{
				recalcLayout = true;
			}
			else
			{
				ws = w.value;
			}
		}
		else
		{
			ws = measurements.first.value;
		}
		if (getSizingModeHorizontal() == SizingMode::WrapContent)
		{
			if (canWrapH)
			{
				recalcLayout = true;
			}
			else
			{
				hs = h.value;
			}
		}
		else
		{
			hs = measurements.second.value;
		}

		if (recalcLayout && (invalidLayout ||
			std::abs(bounds.width - w.value) > 0.000001f ||
			std::abs(bounds.height - h.value) > 0.000001f
			))
		{
			solveConstraints(w, h);
			invalidLayout = false;
			bounds = boundaries[this];
			if (getSizingModeHorizontal() == SizingMode::WrapContent)
			{
				ws = bounds.width;
			}
			if (getSizingModeVertical() == SizingMode::WrapContent)
			{
				hs = bounds.height;
			}
		}
		return std::make_pair<DimensionDesc, DimensionDesc>(
			DimensionDesc(ws, DimensionDesc::Mode::Exact),
			DimensionDesc(hs, DimensionDesc::Mode::Exact)
			);
	}
	void ConstraintsContainer::removeChild(const Component::Type& child)
	{
		auto it = std::find(children.begin(), children.end(), child);
		Component* p = child.get();
		if (it != children.end())
		{
			//find and delete cluster
			auto cit = clusterMapping.find(p);
			if (cit != clusterMapping.end())
			{
				if (!cit->second->components.count(p))
					throw std::runtime_error("Internal ConstraintsContainer error(cluster mapping desync)");
				cit->second->components.erase(p);
				std::vector<Constraint*> constraintsToRemove;
				for (auto constraint : cit->second->constraints)
				{
					if (constraint->isFor(*p))
					{
						switch (constraint->getType())
						{
						case Constraint::Type::Regular:
						{
							//delete immidiately, regular constraints have only one target
							constraintsToRemove.push_back(constraint);
							constraints.erase(constraintMapping.at(constraint));
							break;
						}
						case Constraint::Type::Chain:
						{
							std::remove_if(constraint->chain.targets.begin(), constraint->chain.targets.end(), [p](const std::pair<Component*,float>& a) {
								return a.first == p;
							});
							if (constraint->chain.targets.empty())
							{
								//delete empty chain constraint
								constraintsToRemove.push_back(constraint);
								constraints.erase(constraintMapping.at(constraint));
							}
							break;
						}
						default:
							break;
						}
					}
				}

				for (auto constraint : constraintsToRemove)
				{
					//remove dependency reference from cluster
					std::vector<Component*> deps = constraint->getDeps();
					for (auto dep : deps)
						cit->second->dependencies.at(dep)--;
					cit->second->constraints.erase(constraint);
				}

				//remove cluster if empty
				if (cit->second->constraints.empty())
				{
					clusters.erase(cit->second);
				}
				else
				{
					//otherwise remove handing dependencies
					std::vector<Component*> dependenciesToRemove;
					for (auto i : cit->second->dependencies)
						if (i.second == 0)
							dependenciesToRemove.push_back(i.first);
					for (auto i : dependenciesToRemove)
						cit->second->dependencies.erase(i);
				}

				invalidate();
			}
			children.erase(it);
		}

	}
	void ConstraintsContainer::clearChildren()
	{
		//basics
		Rect bounds = boundaries[this];
		children.clear();
		boundaries.clear();
		boundaries[this] = bounds;

		//constraints&clusters
		constraints.clear();
		constraintMapping.clear();
		clusters.clear();
		clusterMapping.clear();
		messyClusters = true;
		invalidLayout = true;
		canWrapH = false;
		canWrapW = false;

		//drawing caches
		drawnLastFrame.clear();
		needsRedraw.clear();
		firstDraw = true;
		updated.clear();
	}
	void ConstraintsContainer::addChild(const Component::Type& child)
	{
		children.emplace_back(child);
		child->setParent(*this);
		child->poke();
		needsRedraw.insert(child.get());
	}
	std::unique_ptr<ConstraintsContainer::RegularConstraintBuilder> ConstraintsContainer::addConstraint(Constraint::Orientation orientation, const Component::Type& target, bool constOffset)
	{
		if (!target)
			return std::unique_ptr<RegularConstraintBuilder>();
		Constraint c(Constraint::Type::Regular, orientation);

		c.regular.target = target.get();
		c.constOffset = constOffset;

		if (target.get() == this)
		{
			c.setFirstEdge(false);
			c.setSecondEdge(true);
		}
		else
		{
			c.setFirstEdge(true);
			c.setSecondEdge(false);
		}

		auto it = clusterMapping.find(target.get());

		auto cluster = clusters.end();

		if (it != clusterMapping.end())
		{
			for (const auto& i : it->second->constraints)
			{
				if (i->getOrientation() == orientation && i->isFor(*target)) //there is already constraint for that orientation
				{
					return std::unique_ptr<RegularConstraintBuilder>();
				}
			}
			cluster = it->second;
		}
		else
		{
			clusters.emplace_back();
			cluster = std::prev(clusters.end());
		}

		if (cluster != clusters.end())//TODO: potential lack of redraw, fix needed
		{
			if (target.get() != this)
				needsRedraw.insert(target.get());
			cluster->components.insert(target.get());
			constraints.emplace_back(std::move(c));
			constraintMapping[&constraints.back()] = std::prev(constraints.end());
			cluster->constraints.insert(&constraints.back());
			clusterMapping[target.get()] = cluster;

			invalidLayout = true;
			messyClusters = true;
			return std::make_unique<RegularConstraintBuilder>(constraints.back(), cluster);
		}

		return std::unique_ptr<RegularConstraintBuilder>();
	}
	std::unique_ptr<ConstraintsContainer::ChainConstraintBuilder> ConstraintsContainer::addChainConstraint(Constraint::Orientation orientation, const std::vector<Component::Type>& targets, bool constOffset)
	{
		//TODO: potentially merge some for loops?

		if (targets.empty())
			return std::unique_ptr<ChainConstraintBuilder>();

		Constraint c(Constraint::Type::Chain, orientation);

		c.chain.targets.reserve(targets.size());

		std::unordered_set<std::list<Cluster>::iterator, ClusterHash> clustersToMerge;

		for (const auto& target : targets)
		{
			Component* t = target.get();
			if (t == this)
				return std::unique_ptr<ChainConstraintBuilder>();
			c.chain.targets.emplace_back(t, 0.f);

			auto it = clusterMapping.find(target.get());
			if (it != clusterMapping.end())
			{
				for (const auto& i : it->second->constraints)
				{
					if (i->getType() == Constraint::Type::Chain) //there is another chain constraint on one target
					{
						return std::unique_ptr<ChainConstraintBuilder>();
					}
					else if (i->getOrientation() == orientation && i->isFor(*target)) //there is already constraint for that orientation on one target
					{
						return std::unique_ptr<ChainConstraintBuilder>();
					}
				}
				clustersToMerge.emplace(it->second);
			}
		}

		c.constOffset = constOffset;

		c.chain.spacing = 0;

		c.setFirstEdge(true);
		c.setSecondEdge(false);

		for (const auto& target : targets)
		{
			needsRedraw.insert(target.get());
		}

		if (clustersToMerge.empty()) //elements are not in clusters, create new one
		{
			clusters.emplace_back();
			auto cluster = std::prev(clusters.end());

			//create cluster
			constraints.emplace_back(std::move(c));
			constraintMapping[&constraints.back()] = std::prev(constraints.end());
			cluster->constraints.insert(&constraints.back());
			for (const auto& target : targets)
			{
				clusterMapping[target.get()] = cluster;
				cluster->components.insert(target.get());
			}

			invalidLayout = true;
			messyClusters = true;

			return std::make_unique<ChainConstraintBuilder>(constraints.back(), cluster);
		}
		else //move constraints to one cluster
		{
			clusters.emplace_back();
			auto cluster = std::prev(clusters.end());
			//steel constraints
			for (auto clusterToMerge : clustersToMerge)
			{
				for (auto constraint : clusterToMerge->constraints)
				{
					cluster->constraints.insert(constraint);
				}
				for (auto dependency : clusterToMerge->dependencies)
				{
					cluster->dependencies.insert(dependency);
				}
				//remove unwanted clusters
				clusters.erase(clusterToMerge);
			}
			//add new one
			constraints.emplace_back(std::move(c));
			cluster->constraints.insert(&constraints.back());
			for (const auto& target : targets)
			{
				clusterMapping[target.get()] = cluster;
				cluster->components.insert(target.get());
			}

			invalidLayout = true;
			messyClusters = true;

			return std::make_unique<ChainConstraintBuilder>(constraints.back(), cluster);
		}

		return std::unique_ptr<ChainConstraintBuilder>();
		
	}
	
	void ConstraintsContainer::onMaskDraw(Canvas& canvas) const
	{
		if (backgroundColor.a > 0 && firstDraw)
		{
			Component::onMaskDraw(canvas);
		}
		else
		{
			for (const auto& i : drawnLastFrame)
				getBackend()->addToMask(i.second);
			for (auto element : needsRedraw)
			{
				element->drawMask(canvas);
			}
		}
	}
	void ConstraintsContainer::onDraw(Canvas& canvas)
	{
		if (needsRedraw.size() > 0)
		{
			if (backgroundColor.a > 0 && firstDraw)
			{
				Rect bounds = getBounds().at(Vec2(0.f, 0.f));
				canvas.drawRectangle(bounds, backgroundColor);
				for (const auto& i : children)
				{
					i->redraw(canvas);
				}
			}
			else
			{
				std::unordered_set<Component*> forceRedraw;

				std::vector<std::pair<Rect, Component*>> base;
				std::vector<std::pair<Rect, Component*>> lastBase;
				base.reserve(needsRedraw.size());
				for (auto i : needsRedraw)
				{
					Rect localBounds = i->getBounds();
					base.emplace_back(localBounds, i);
				}

				for (auto& i : drawnLastFrame)
				{
					Rect localBounds = i.second;
					lastBase.emplace_back(localBounds, i.first);
				}

				for (const auto& i : children)
				{
					Component* p = i.get();
					//all elements that were moved should redraw all its content internally,
					//so we only need to worry about not updated ones
					if (!drawnLastFrame.count(p))
					{
						//check if element intersects with any object moved during this frame
						//and force redraw if it does
						Rect localBounds = p->getBounds();
						for (const auto& j : lastBase)
						{
							if (j.first.intersects(localBounds))
							{
								forceRedraw.insert(p);
							}
						}
					}
					//check if element intersects with any object currently being drawn
					//and force redraw if it does
					Rect localBounds = i->getBounds();
					for (const auto& j : base)
					{
						if (j.first.intersects(localBounds))
						{
							forceRedraw.insert(j.second);
							forceRedraw.insert(p);
						}
					}

				}
				if (backgroundColor.a > 0)
					canvas.drawRectangle(getBounds().at(Vec2(0.f, 0.f)), backgroundColor);
				for (const auto& i : children)
				{
					Rect localBounds = i->getBounds();
					Component* c = i.get();
					if (forceRedraw.count(c))
					{
						c->redraw(canvas);
					}
					else if (needsRedraw.count(c))
					{
						c->draw(canvas);
					}
				}
			}
			needsRedraw.clear();
			drawnLastFrame.clear();
		}
		else if (backgroundColor.a > 0 && firstDraw)
		{
			onRedraw(canvas);
		}
		firstDraw = false;
	}
	void ConstraintsContainer::onRedraw(Canvas& canvas)
	{
		Rect bounds = getBounds();
		bounds.left = 0;
		bounds.top = 0;

		bool hasBackground = backgroundColor.a > 0;

		if (hasBackground)
		{
			canvas.drawRectangle(bounds, backgroundColor);
		}

		for (const auto& i : children)
		{
			i->redraw(canvas);
		}
		needsRedraw.clear();
		drawnLastFrame.clear();
		firstDraw = false;
	}

	void ConstraintsContainer::postXmlConstruction(Manager& manager, const XML::Tag& config, const StylingPack& pack)
	{
		std::unordered_map<std::string, Component::Type> nameMapping;
		Manager::handleDefaultArguments(*this, config, pack.style);

		std::vector<Component::Type> childMapping;

		XML::Value tmp;
		{
			auto background = pack.style.getAttribute("backgroundColor");
			if (background && background->checkType<Color>())
			{
				setBackgroundColor(background->as<Color>());
			}
		}
		
		for (const auto& child : config.children)
		{
			if (!child->isTextNode())
			{
				XML::Tag& tag = static_cast<XML::Tag&>(*child);
				Component::Type t = manager.instantiate(tag, pack.theme);
				auto it = tag.attributes.find("name");
				if (it != tag.attributes.end())
				{
					nameMapping.emplace(it->second.val, t);
				}
				addChild(t);
				childMapping.push_back(t);
			}
		}
		unsigned i = 0;
		for (const auto& child : config.children)
		{
			if (!child->isTextNode())
			{
				XML::Tag& c = static_cast<XML::Tag&>(*child);

				tmp = c.getAttribute("horizontalConstraint");
				if (tmp.exists())
				{
					//creating horizontal constraint
					if (tmp.val == "regular")
					{
						bool constOffset = true;

						tmp = c.getAttribute("horizontalSizing");
						if (tmp.exists())
						{
							if (tmp.val == "const_offset")
							{
								constOffset = true;
							}
							else if (tmp.val == "const_size")
							{
								constOffset = false;
							}
						}

						auto builder = addConstraint(Constraint::Orientation::Horizontal, childMapping[i], constOffset);

						tmp = c.getAttribute("attachLeftTo");
						if (tmp.exists())
						{
							bool toLeft = false;
							float offset = 0;
							Component::Type target;

							if (tmp.val.find(" ") != tmp.val.npos)
							{
								std::vector<std::string> options = Styles::splitString(tmp.val);
								if (options.size() > 0)
								{
									if (options[0] == "parent")
									{
										target = shared_from_this();
									}
									else
									{
										auto it = nameMapping.find(options[0]);
										if (it != nameMapping.end())
										{
											target = it->second;
										}
									}
									if (options.size() > 1)
									{
										if (options[1] == "left")
											toLeft = true;
										else if (options[1] == "right")
											toLeft = false;

										if (options.size() > 2)
										{
											bool failed = false;
											float v = Styles::strToFloat(options[2], failed);
											if (!failed)
												offset = v;
										}
									}
								}
							}
							else
							{
								if (tmp.val == "parent")
								{
									target = shared_from_this();
								}
								else
								{
									auto it = nameMapping.find(tmp.val);
									if (it != nameMapping.end())
									{
										target = it->second;
									}
								}

								tmp = c.getAttribute("leftAttachmentSide");
								if (tmp.exists())
								{
									if (tmp.val == "left")
										toLeft = true;
									else if (tmp.val == "right")
										toLeft = false;
								}

								tmp = c.getAttribute("leftAttachmentOffset");
								if (tmp.exists())
								{
									bool failed = false;
									float v = Styles::strToFloat(tmp.val, failed);
									if (!failed)
										offset = v;
								}
							}

							builder->attachLeftTo(target, toLeft, offset);
						}

						tmp = c.getAttribute("attachRightTo");
						if (tmp.exists())
						{
							bool toLeft = false;
							float offset = 0;
							Component::Type target;

							if (tmp.val.find(" ") != tmp.val.npos)
							{
								std::vector<std::string> options = Styles::splitString(tmp.val);
								if (options.size() > 0)
								{
									if (options[0] == "parent")
									{
										target = shared_from_this();
									}
									else
									{
										auto it = nameMapping.find(options[0]);
										if (it != nameMapping.end())
										{
											target = it->second;
										}
									}
									if (options.size() > 1)
									{
										if (options[1] == "left")
											toLeft = true;
										else if (options[1] == "right")
											toLeft = false;

										if (options.size() > 2)
										{
											bool failed = false;
											float v = Styles::strToFloat(options[2], failed);
											if (!failed)
												offset = v;
										}
									}
								}
							}
							else
							{
								if (tmp.val == "parent")
								{
									target = shared_from_this();
								}
								else
								{
									auto it = nameMapping.find(tmp.val);
									if (it != nameMapping.end())
									{
										target = it->second;
									}
								}

								tmp = c.getAttribute("rightAttachmentSide");
								if (tmp.exists())
								{
									if (tmp.val == "left")
										toLeft = true;
									else if (tmp.val == "right")
										toLeft = false;
								}

								tmp = c.getAttribute("rightAttachmentOffset");
								if (tmp.exists())
								{
									bool failed = false;
									float v = Styles::strToFloat(tmp.val, failed);
									if (!failed)
										offset = v;
								}
							}

							builder->attachRightTo(target, toLeft, offset);
						}

						tmp = c.getAttribute("horizontalFlow");
						if (tmp.exists())
						{
							bool failed = false;
							float flow = Styles::strToFloat(tmp.val, failed);
							if (!failed)
								builder->setFlow(flow);
						}

						tmp = c.getAttribute("width");
						if (tmp.exists())
						{
							bool failed = false;
							float size = Styles::strToFloat(tmp.val, failed);
							if (!failed)
								builder->setSize(size);
						}
					}
					else if (tmp.val == "chain")
					{
						//
					}
					else
					{
						//error i guess
					}
				}

				tmp = c.getAttribute("verticalConstraint");
				if (tmp.exists())
				{
					//creating horizontal constraint
					if (tmp.val == "regular")
					{
						bool constOffset = true;

						tmp = c.getAttribute("verticalSizing");
						if (tmp.exists())
						{
							if (tmp.val == "const_offset")
							{
								constOffset = true;
							}
							else if (tmp.val == "const_size")
							{
								constOffset = false;
							}
						}

						auto builder = addConstraint(Constraint::Orientation::Vertical, childMapping[i], constOffset);

						tmp = c.getAttribute("attachTopTo");
						if (tmp.exists())
						{
							bool toTop = false;
							float offset = 0;
							Component::Type target;

							if (tmp.val.find(" ") != tmp.val.npos)
							{
								std::vector<std::string> options = Styles::splitString(tmp.val);
								if (options.size() > 0)
								{
									if (options[0] == "parent")
									{
										target = shared_from_this();
									}
									else
									{
										auto it = nameMapping.find(options[0]);
										if (it != nameMapping.end())
										{
											target = it->second;
										}
									}
									if (options.size() > 1)
									{
										if (options[1] == "top")
											toTop = true;
										else if (options[1] == "bottom")
											toTop = false;

										if (options.size() > 2)
										{
											bool failed = false;
											float v = Styles::strToFloat(options[2], failed);
											if (!failed)
												offset = v;
										}
									}
								}
							}
							else
							{
								if (tmp.val == "parent")
								{
									target = shared_from_this();
								}
								else
								{
									auto it = nameMapping.find(tmp.val);
									if (it != nameMapping.end())
									{
										target = it->second;
									}
								}

								tmp = c.getAttribute("topAttachmentSide");
								if (tmp.exists())
								{
									if (tmp.val == "top")
										toTop = true;
									else if (tmp.val == "bottom")
										toTop = false;
								}

								tmp = c.getAttribute("topAttachmentOffset");
								if (tmp.exists())
								{
									bool failed = false;
									float v = Styles::strToFloat(tmp.val, failed);
									if (!failed)
										offset = v;
								}
							}

							builder->attachTopTo(target, toTop, offset);
						}

						tmp = c.getAttribute("attachBottomTo");
						if (tmp.exists())
						{
							bool toTop = false;
							float offset = 0;
							Component::Type target;

							if (tmp.val.find(" ") != tmp.val.npos)
							{
								std::vector<std::string> options = Styles::splitString(tmp.val);
								if (options.size() > 0)
								{
									if (options[0] == "parent")
									{
										target = shared_from_this();
									}
									else
									{
										auto it = nameMapping.find(options[0]);
										if (it != nameMapping.end())
										{
											target = it->second;
										}
									}
									if (options.size() > 1)
									{
										if (options[1] == "top")
											toTop = true;
										else if (options[1] == "bottom")
											toTop = false;

										if (options.size() > 2)
										{
											bool failed = false;
											float v = Styles::strToFloat(options[2], failed);
											if (!failed)
												offset = v;
										}
									}
								}
							}
							else
							{
								if (tmp.val == "parent")
								{
									target = shared_from_this();
								}
								else
								{
									auto it = nameMapping.find(tmp.val);
									if (it != nameMapping.end())
									{
										target = it->second;
									}
								}

								tmp = c.getAttribute("bottomAttachmentSide");
								if (tmp.exists())
								{
									if (tmp.val == "top")
										toTop = true;
									else if (tmp.val == "bottom")
										toTop = false;
								}

								tmp = c.getAttribute("bottomAttachmentOffset");
								if (tmp.exists())
								{
									bool failed = false;
									float v = Styles::strToFloat(tmp.val, failed);
									if (!failed)
										offset = v;
								}
							}

							builder->attachRightTo(target, toTop, offset);
						}

						tmp = c.getAttribute("verticalFlow");
						if (tmp.exists())
						{
							bool failed = false;
							float flow = Styles::strToFloat(tmp.val, failed);
							if (!failed)
								builder->setFlow(flow);
						}

						tmp = c.getAttribute("height");
						if (tmp.exists())
						{
							bool failed = false;
							float size = Styles::strToFloat(tmp.val, failed);
							if (!failed)
								builder->setSize(size);
						}
					}
					else if (tmp.val == "chain")
					{
						//
					}
					else
					{
						//error i guess
					}
				}

				childMapping[i]->invalidate();

				++i;
			}
		}
	}
	ConstraintsContainer::ConstraintsContainer() : firstDraw(true), messyClusters(true), invalidLayout(true), canWrapW(false), canWrapH(false), backgroundColor(0)
	{
	}
	ConstraintsContainer::Iterator ConstraintsContainer::firstElement()
	{
		return createIterator<IteratorType>(children.begin(),children.end());
	}
}
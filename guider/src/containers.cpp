#include <guider/containers.hpp>
#include <queue>

namespace Guider
{
	void ListContainer::addChild(const Component::Type& child)
	{
		child->setParent(*this);
		float off;
		Rect bounds = getBounds();

		DimensionDesc w(bounds.width, DimensionDesc::Max);
		DimensionDesc h(bounds.height, DimensionDesc::Max);

		if (horizontal)
		{
			w.value = 0;
			w.mode = DimensionDesc::Exact;
		}
		else
		{
			h.value = 0;
			h.mode = DimensionDesc::Exact;
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

		children.emplace_back(child,size);
		toUpdate.insert(child.get());
		toRedraw.insert(child.get());

		size += off;
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
				while (cp != children.end())
				{
					toRedraw.insert(cp->component.get());
					//todo: adjust offset or move offsetting to onDraw
					++cp;
				}
				children.erase(it);
				break;
			}
			++it;
		}
	}
	void ListContainer::clearChildren()
	{
		children.clear();
		toUpdate.clear();
		toRedraw.clear();
	}
	void ListContainer::drawMask(RenderBackend& backend) const
	{
		for (const auto& i : children)
		{
			if (toRedraw.count(i.component.get()))
			{
				i.component->drawMask(backend);
			}
		}
	}

	void ListContainer::onDraw(RenderBackend& backend) const
	{
		for (const auto& i : children)
		{
			if (toRedraw.count(i.component.get()))
			{
				i.component->draw(backend);
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
			if (chain.targets[0].first != nullptr)
				ret.push_back(chain.targets[0].first);
			if (chain.targets[1].first != nullptr)
				ret.push_back(chain.targets[1].first);
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

		cluster.dependencies.erase(constraint.regular.left);
		constraint.regular.left = target.get();
		cluster.dependencies.insert(constraint.regular.left);
	}

	void ConstraintsContainer::RegularConstraintBuilder::attachEndTo(const Component::Type& target, bool toStart, float offset)
	{
		constraint.regular.rightOffset = offset;
		constraint.setSecondEdge(toStart);

		cluster.dependencies.erase(constraint.regular.right);
		constraint.regular.right = target.get();
		cluster.dependencies.insert(constraint.regular.right);
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
		std::unordered_map<const Cluster*, unsigned> ndeps;
		for (const auto& i : clusters)
		{
			ndeps[&i] = 0;
		}

		for (const auto& i : clusters)
		{
			auto& clusterDependency = clusterDependencies[&i];
			for (auto it = clusterDependency.begin(); it != clusterDependency.end(); it++)
				ndeps[*it]++;
		}

		// Create an queue and enqueue all vertices with 
		// depth 0 
		std::queue<const Cluster*> q;
		for (const auto& i : clusters)
			if (ndeps[&i] == 0)
				q.push(&i);

		unsigned cnt = 0;

		std::unordered_set<const Cluster*> visited;
		std::vector<const Cluster*> ret;

		ret.reserve(n);

		while (!q.empty())
		{
			const Cluster* front = q.front();
			q.pop();
			ret.emplace_back(front);
			auto& clusterDependency = clusterDependencies[front];
			for (auto it = clusterDependency.begin(); it != clusterDependency.end(); it++)
				if (--ndeps[*it] == 0)
					q.push(*it);

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
			Component* start = c.chain.targets[0].first;
			float soff = c.chain.targets[0].second;
			Component* end = c.chain.targets[1].first;
			float eoff = c.chain.targets[1].second;

			float s = getEdge(start, c.getFirstEdge()) + soff;
			float e = getEdge(end, c.getSecondEdge()) - eoff;

			if (s > e)
			{
				float tmp = s;
				s = e;
				e = tmp;
			}

			float width = 0;
			for (unsigned j = 2; j < c.chain.targets.size(); ++j)
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

			size_t n = c.chain.targets.size() - 2;
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

				for (unsigned i = 2; i < c.chain.targets.size(); ++i)
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

				for (unsigned i = 2; i < c.chain.targets.size(); ++i)
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
		for (auto& child : children)
		{
			if (child->getBounds() != boundaries[child.get()])
				setBounds(*child, boundaries[child.get()]);
		}
		invalidLayout = false;
	}
	void ConstraintsContainer::drawMask(RenderBackend& renderer) const
	{
		for (auto element : needsRedraw)
		{
			element->drawMask(renderer);
		}
	}
	void ConstraintsContainer::poke()
	{
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
		if (bounds != boundaries[this] || invalidLayout)
		{
			solveConstraints();
			invalidLayout = false;
			applyConstraints();
			for (auto& i : children)
				needsRedraw.insert(i.get());
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
		//todo
	}
	void ConstraintsContainer::clearChildren()
	{
		Rect bounds = boundaries[this];
		children.clear();
		boundaries.clear();
		boundaries[this] = bounds;
		constraints.clear();
		clusters.clear();
		clusterDependencies.clear();
		clusterMapping.clear();
		updated.clear();
		messyClusters = true;
		invalidLayout = true;
		canWrapH = false;
		canWrapW = false;
	}
	void ConstraintsContainer::addChild(const Component::Type& child)
	{
		children.emplace_back(child);
		child->setParent(*this);
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

		Cluster* cluster = nullptr;

		if (it != clusterMapping.end())
		{
			for (const auto& i : it->second->constraints)
			{
				if (i->getOrientation() == orientation && i->isFor(*target)) //there is already constraint for that orientation
				{
					return std::unique_ptr<RegularConstraintBuilder>();
				}
			}
			cluster = &const_cast<Cluster&>(*it->second);
		}
		else
		{
			clusters.emplace_back();
			cluster = &clusters.back();
		}

		if (cluster != nullptr)
		{
			if (target.get() != this)
				needsRedraw.insert(target.get());
			cluster->components.insert(target.get());
			constraints.emplace_back(std::move(c));
			cluster->constraints.insert(&constraints.back());
			clusterMapping[target.get()] = cluster;

			invalidLayout = true;
			messyClusters = true;
			return std::make_unique<RegularConstraintBuilder>(constraints.back(), *cluster);
		}

		return std::unique_ptr<RegularConstraintBuilder>();
	}
	std::unique_ptr<ConstraintsContainer::ChainConstraintBuilder> ConstraintsContainer::addChainConstraint(Constraint::Orientation orientation, const std::vector<Component::Type>& targets, bool constOffset)
	{
		return std::unique_ptr<ChainConstraintBuilder>();
	}
	void ConstraintsContainer::onDraw(RenderBackend& renderer) const
	{
		if (needsRedraw.size() > 0)
		{
			Rect bounds = getGlobalBounds();
			std::vector<Rect> base;
			base.reserve(needsRedraw.size());
			for (auto i : needsRedraw)
			{
				Rect globalBounds = i->getGlobalBounds();
				base.emplace_back(globalBounds);
			}
			for (const auto& i : children)
			{
				Rect localBounds = i->getBounds();
				for (const auto& rect : base)
				{
					if (localBounds.intersects(rect))
					{
						Rect local = i->getBounds();
						i->draw(renderer);
						break;
					}
				}
			}
			needsRedraw.clear();
		}
	}
}
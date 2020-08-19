#pragma once

#include <guider/base.hpp>
#include <guider/manager.hpp>
#include <stdexcept>
#include <list>

namespace Guider
{
	class ListContainer : public Container
	{
	private:
		bool horizontal;
		float size, offset;
		class Element
		{
		public:
			std::shared_ptr<Component> component;
			float size, offset;

			Element(const std::shared_ptr<Component>& c, float s, float o) : component(c), size(s), offset(o) {}
			Element(Element&&) noexcept = default;
		};
		std::list<Element> children;
		std::unordered_set<Component*> toUpdate, toOffset;
		mutable std::unordered_set<Component*> toRedraw;

		Color backgroundColor;

		bool updateElementRect(Element& element, bool needsMeasure, float localOffset, const DimensionDesc& w, const DimensionDesc& h, const Rect& bounds);
	public:
		void setOrientation(bool horizontal);

		void setBackgroundColor(const Color& color);
		virtual void addChild(const Component::Type& child) override;
		virtual void removeChild(const Component::Type& child) override;
		void removeChild(unsigned n);
		virtual void clearChildren() override;
		size_t getChildrenCount() const;
		Component::Type getChild(unsigned i);

		virtual void drawMask(Backend& backend) const override;
		virtual void onDraw(Backend& backend) const override;

		virtual void poke() override;
		virtual void onResize(const Rect& lastBounds) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual void propagateEvent(const Event& event) override;

		std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& w, const DimensionDesc& h) override;

		ListContainer() : horizontal(false), size(0), offset(0) {}

		ListContainer(Manager& manager, const XML::Tag& config) : ListContainer()
		{
			Manager::handleDefaultArguments(*this, config);

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
					Component::Type t = manager.instantiate(tag);

					addChild(t);
				}
			}
		}
	};

	class ConstraintsContainer : public Container, public std::enable_shared_from_this<ConstraintsContainer>
	{
	private:
		class Cluster;
	public:
		class RegularConstraintData
		{
		public:
			Component* left, * right, * target;
			float leftOffset, rightOffset, flow, size;

			RegularConstraintData() : left(nullptr), right(nullptr), target(nullptr), leftOffset(0), rightOffset(0), flow(0), size(0) {}
			RegularConstraintData(RegularConstraintData&& t) noexcept : left(t.left), right(t.right), target(t.target), leftOffset(t.leftOffset), rightOffset(t.rightOffset), flow(t.flow), size(t.size) {}
		};

		class ChainConstraintData
		{
		public:
			std::vector<std::pair<Component*, float>> targets;
			float spacing;

			ChainConstraintData() : spacing(0) {}
			ChainConstraintData(ChainConstraintData&& t) noexcept : targets(std::move(t.targets)), spacing(t.spacing) {}
		};
		class Constraint
		{
		private:
			static constexpr uint8_t OrientationMask = 1 << 1;
			static constexpr uint8_t TypeMask = 1 << 2 | 1 << 3;
			static constexpr uint8_t TypeOffset = 2;
			static constexpr uint8_t EdgeFirstMask = 1 << 4;
			static constexpr uint8_t EdgeSecondMask = 1 << 5;
			static constexpr uint8_t ConstOffsetMask = 1 << 6;

			uint8_t flags;

		public:
			enum class Edge
			{
				Left = 0,
				Right = 1,
				Top = 2,
				Bottom = 3
			};

			enum class PassType
			{
				First,
				Final
			};

			enum class Orientation
			{
				Horizontal = 0,
				Vertical = 1
			};

			enum class Type
			{
				None = 0,
				Regular = 1,
				Chain = 2
			};


			union
			{
				RegularConstraintData regular;
				ChainConstraintData chain;
			};
			bool constOffset;

		private:
			inline void setOrientation(Orientation o)
			{
				flags = flags & (~OrientationMask) | (o == Orientation::Vertical ? OrientationMask : 0);
			}
			inline void setType(Type type)
			{
				flags = flags & (~TypeMask) | ((type == Type::Chain ? 2 : 1) << TypeOffset);
			}
		public:

			inline Orientation getOrientation() const noexcept
			{
				return (flags & OrientationMask) ? Orientation::Vertical : Orientation::Horizontal;
			}
			inline Type getType() const noexcept
			{
				uint8_t v = (flags & TypeMask) >> TypeOffset;
				switch (v)
				{
				case 1:
					return Type::Regular;
				case 2:
					return Type::Chain;
				default:
					return Type::None;
				}
			}

			inline bool isOffsetContant() const noexcept
			{
				return (flags & ConstOffsetMask) > 0;
			}

			inline Edge getFirstEdge() const noexcept
			{
				bool e = flags & EdgeFirstMask;
				return getOrientation() == Orientation::Horizontal ? (e ? Edge::Right : Edge::Left) : (e ? Edge::Bottom : Edge::Top);
			}

			inline Edge getSecondEdge() const noexcept
			{
				bool e = flags & EdgeSecondMask;
				return getOrientation() == Orientation::Horizontal ? (e ? Edge::Right : Edge::Left) : (e ? Edge::Bottom : Edge::Top);
			}

			inline void setFirstEdge(bool start)
			{
				flags = (flags & ~EdgeFirstMask) | (start ? 0 : EdgeFirstMask);
			}

			inline void setSecondEdge(bool start)
			{
				flags = (flags & ~EdgeSecondMask) | (start ? 0 : EdgeSecondMask);
			}

			std::vector<Component*> getDeps() const;
			bool isFor(const Component& c) const;

			Constraint(Type type, Orientation o);

			Constraint(Constraint&&) noexcept;
			~Constraint();
		};
		class RegularConstraintBuilder
		{
		private:
			Constraint& constraint;
			Cluster& cluster;
		public:
			void setSize(float size);
			void setFlow(float flow);
			void attachStartTo(const Component::Type& target, bool toStart, float offset);
			inline void attachLeftTo(const Component::Type& target, bool toLeft, float offset)
			{
				attachStartTo(target, toLeft, offset);
			}
			inline void attachTopTo(const Component::Type& target, bool toTop, float offset)
			{
				attachStartTo(target, toTop, offset);
			}
			void attachEndTo(const Component::Type& target, bool toStart, float offset);
			inline void attachRightTo(const Component::Type& target, bool toLeft, float offset)
			{
				attachEndTo(target, toLeft, offset);
			}
			inline void attachBottomTo(const Component::Type& target, bool toTop, float offset)
			{
				attachEndTo(target, toTop, offset);
			}
			inline void attachBetween(const Component::Type& start, bool startToStart, float startOffset, const Component::Type& end, bool endToStart, float endOffset)
			{
				attachStartTo(start, startToStart, startOffset);
				attachEndTo(end, endToStart, endOffset);
			}
			inline void attachBetween(const Component::Type& start, bool startToStart, const Component::Type& end, bool endToStart, float offset)
			{
				attachBetween(start, startToStart, offset, end, endToStart, offset);
			}
			//TODO: add notifying parent if container edge is attached to child
			RegularConstraintBuilder(Constraint& c, Cluster& cc) : constraint(c), cluster(cc)
			{
				if (c.getType() != Constraint::Type::Regular)
					throw std::logic_error("Wrong constraint type");
			}
		};
		class ChainConstraintBuilder
		{
		private:
			Constraint& constraint;
			Cluster& cluster;
		public:
			ChainConstraintBuilder(Constraint& c, Cluster& cc) : constraint(c), cluster(cc)
			{
				if (c.getType() != Constraint::Type::Chain)
					throw std::logic_error("Wrong constraint type");
			}
		};
	private:
		class Cluster
		{
		public:
			std::unordered_set<Constraint*> constraints;
			std::unordered_set<Component*> dependencies, components;

			Cluster() = default;
			Cluster(Cluster&& t) noexcept : constraints(std::move(t.constraints)), dependencies(std::move(t.dependencies)), components(std::move(t.components)) {}
		};

		std::vector<std::shared_ptr<Component>> children;
		std::unordered_map<Component*, Rect> boundaries;
		std::list<Constraint> constraints;
		std::list<Cluster> clusters;
		std::unordered_map<const Cluster*, std::unordered_set<const Cluster*>> clusterDependencies;
		std::unordered_map<const Component*, const Cluster*> clusterMapping;
		std::unordered_set<Component*> updated;

		mutable std::unordered_set<Component*> needsRedraw;

		bool messyClusters;
		bool invalidLayout;
		bool canWrapW, canWrapH;//TODO: add updating this values;

		Color backgroundColor;

		float getEdge(Component* c, Constraint::Edge e);

		bool reorderClusters();

		void solveConstraint(const Constraint& c, Constraint::PassType pass);

		void solveCluster(const Cluster& cluster);

		void solveConstraints();
		void solveConstraints(const DimensionDesc& w, const DimensionDesc& h);

		void applyConstraints();

	public:
		void setBackgroundColor(const Color& color);

		virtual void drawMask(Backend& renderer) const override;

		virtual void poke() override;
		virtual void onResize(const Rect& bounds) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;
		
		virtual void propagateEvent(const Event& event) override;
		
		std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& w, const DimensionDesc& h) override;

		virtual void removeChild(const Component::Type& child) override;
		virtual void clearChildren() override;
		virtual void addChild(const Component::Type& child);

		std::unique_ptr<RegularConstraintBuilder> addConstraint(Constraint::Orientation orientation, const Component::Type& target, bool constOffset);
		std::unique_ptr<ChainConstraintBuilder> addChainConstraint(Constraint::Orientation orientation, const std::vector<Component::Type>& targets, bool constOffset);

		virtual void onDraw(Backend& renderer) const override;

		virtual void postXmlConstruction(Manager& m, const XML::Tag& config);

		ConstraintsContainer() : messyClusters(true), invalidLayout(true), canWrapW(false), canWrapH(false), backgroundColor(0) {}
		ConstraintsContainer(Manager& manager, const XML::Tag& config) : ConstraintsContainer() {}
	};

}
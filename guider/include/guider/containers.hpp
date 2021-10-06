#pragma once

#include <guider/manager.hpp>
#include <guider/base.hpp>
#include <stdexcept>
#include <list>

namespace Guider
{
	//TODO: rewrite
	class AbsoluteContainer : public Container
	{
	public:
		static void registerProperties(Manager& manager, const String& name);

		virtual void addChild(const Component::Ptr& child) override;
		void addChild(const Component::Ptr& child, float x, float y);
		virtual void removeChild(const Component::Ptr& child) override;
		virtual void clearChildren() override;

		virtual Iterator begin() override;
		virtual Iterator end() override;

		virtual void poke() override;

		virtual void onResize(const Rect& last) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual void onMaskDraw(Canvas& canvas) const override;
		virtual void onDraw(Canvas& canvas) override;
	private:
		struct Element
		{
			Component::Ptr component;
			float x, y;

			Element(const Component::Ptr& c, float x, float y);
			Element(const Element&) = default;
		};

		using iterator = Vector<Element>::iterator;

		struct IteratorType : public PackedIteratorTemplate<IteratorType, iterator>
		{
			virtual Component& unpack(const iterator& i) const override
			{
				return *i->component;
			}

			using PackedIteratorTemplate::PackedIteratorTemplate;
		};
		Vector<Element> children;

		std::unordered_set<Component*> toUpdate;
	};

	//TODO: rewrite
	class ListContainer : public Container
	{
	public:
		static void registerProperties(Manager& manager, const String& name);

		void setOrientation(Orientation orientation);
		Orientation getOrientation() const noexcept;

		void setBackgroundColor(const Color& color);
		Color getBackgroundColor() const noexcept;

		void setOffset(float offset);
		float getOffset() const noexcept;

		virtual void addChild(const Component::Ptr& child) override;
		virtual void removeChild(const Component::Ptr& child) override;
		void removeChild(unsigned n);
		virtual void clearChildren() override;
		size_t getChildrenCount() const;
		Component::Ptr getChild(unsigned i);

		virtual void onMaskDraw(Canvas& canvas) const override;
		virtual void onDraw(Canvas& canvas) override;
		virtual void onRedraw(Canvas& canvas) override;

		virtual void poke() override;
		virtual void onResize(const Rect& lastBounds) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual Iterator begin() override;
		virtual Iterator end() override;

		std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& w, const DimensionDesc& h) override;

		ListContainer();

		ListContainer(Manager& manager, const XML::Tag& tag, const StylingPack& pack);
	private:
		class Element
		{
		public:
			Component::Ptr component;
			float size, newSize, offset;

			Element(const Component::Ptr& component, float size, float offset);
			Element(Element&&) noexcept = default;
		};

		using iterator = std::list<Element>::iterator;

		struct IteratorType : public PackedIteratorTemplate<IteratorType, iterator>
		{
			virtual Component& unpack(const iterator& i) const override
			{
				return *i->component;
			}

			using PackedIteratorTemplate::PackedIteratorTemplate;
		};

		Orientation orientation;
		float size, offset, newOffset;

		std::list<Element> children;
		std::unordered_set<Component*> toUpdate, toOffset;
		std::unordered_set<Component*> toRedraw;

		std::unordered_map<Component*, iterator> childMapping;

		std::unordered_set<Component*> toMeasure, updated;

		iterator visibleBegin;
		iterator visibleEnd;
		std::unordered_set<Component*> visible, beforeVisible;

		Color backgroundColor;

		bool firstDraw;

		void adjustVisibleElements();
		void recalculateVisibleElements();
	};

	class ConstraintsContainer : public Container, public std::enable_shared_from_this<ConstraintsContainer>
	{
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
			Component* left, * right;
			float leftOffset, rightOffset;
			std::vector<std::pair<Component*, float>> targets;
			float spacing;

			ChainConstraintData() : left(nullptr), right(nullptr), leftOffset(0), rightOffset(0), spacing(0) {}
			ChainConstraintData(ChainConstraintData&& t) noexcept : left(t.left), right(t.right), leftOffset(t.leftOffset), rightOffset(t.rightOffset), targets(std::move(t.targets)), spacing(t.spacing) {}
		};
		class Constraint
		{
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

			Orientation getOrientation() const noexcept;
			Type getType() const noexcept;

			bool isOffsetContant() const noexcept;

			Edge getFirstEdge() const noexcept;

			Edge getSecondEdge() const noexcept;

			void setFirstEdge(bool start);

			void setSecondEdge(bool start);

			Vector<Component*> getDeps() const;
			bool isFor(const Component& c) const;

			Constraint(Type type, Orientation o);

			Constraint(Constraint&&) noexcept;
			~Constraint();
		private:
			static constexpr uint8_t OrientationMask = 1 << 1;
			static constexpr uint8_t TypeMask = 1 << 2 | 1 << 3;
			static constexpr uint8_t TypeOffset = 2;
			static constexpr uint8_t EdgeFirstMask = 1 << 4;
			static constexpr uint8_t EdgeSecondMask = 1 << 5;
			static constexpr uint8_t ConstOffsetMask = 1 << 6;

			uint8_t flags;

			inline void setOrientation(Orientation o)
			{
				flags = flags & (~OrientationMask) | (o == Orientation::Vertical ? OrientationMask : 0);
			}
			inline void setType(Type type)
			{
				flags = flags & (~TypeMask) | ((type == Type::Chain ? 2 : 1) << TypeOffset);
			}
		};

		class Cluster
		{
		public:
			std::unordered_set<Constraint*> constraints;
			std::unordered_set<Component*> components;
			std::unordered_map<Component*, size_t> dependencies;

			Cluster() = default;
			Cluster(Cluster&& t) noexcept : constraints(std::move(t.constraints)), components(std::move(t.components)), dependencies(std::move(t.dependencies)) {}
		};

		class ClusterHash
		{
		public:
			std::size_t operator() (const std::list<Guider::ConstraintsContainer::Cluster>::iterator& it) const
			{
				return std::hash<const Guider::ConstraintsContainer::Cluster*>{}(&(*it));
			}
		};

		class RegularConstraintBuilder
		{
		private:
			Constraint& constraint;
			std::list<Cluster>::iterator cluster;
		public:
			void setSize(float size);
			void setFlow(float flow);
			
			void attachStartTo(const Component::Ptr& target, bool toStart, float offset);
			void attachEndTo(const Component::Ptr& target, bool toStart, float offset);
			
			void attachLeftTo(const Component::Ptr& target, bool toLeft, float offset);
			void attachRightTo(const Component::Ptr& target, bool toLeft, float offset);
			
			void attachTopTo(const Component::Ptr& target, bool toTop, float offset);
			void attachBottomTo(const Component::Ptr& target, bool toTop, float offset);
			
			void attachBetween(const Component::Ptr& start, bool startToStart, float startOffset, const Component::Ptr& end, bool endToStart, float endOffset);
			void attachBetween(const Component::Ptr& start, bool startToStart, const Component::Ptr& end, bool endToStart, float offset);

			void applyChanges();

			RegularConstraintBuilder(Constraint& c, std::list<Cluster>::iterator cc) : constraint(c), cluster(cc)
			{
				if (c.getType() != Constraint::Type::Regular)
					throw std::logic_error("Wrong constraint type");
			}
		};
		class ChainConstraintBuilder
		{
		private:
			Constraint& constraint;
			std::list<Cluster>::iterator cluster;
		public:
			//TODO: add methods for element sizing/flow

			void attachStartTo(const Component::Ptr& target, bool toStart, float offset);
			inline void attachLeftTo(const Component::Ptr& target, bool toLeft, float offset)
			{
				attachStartTo(target, toLeft, offset);
			}
			inline void attachTopTo(const Component::Ptr& target, bool toTop, float offset)
			{
				attachStartTo(target, toTop, offset);
			}
			void attachEndTo(const Component::Ptr& target, bool toStart, float offset);
			inline void attachRightTo(const Component::Ptr& target, bool toLeft, float offset)
			{
				attachEndTo(target, toLeft, offset);
			}
			inline void attachBottomTo(const Component::Ptr& target, bool toTop, float offset)
			{
				attachEndTo(target, toTop, offset);
			}
			inline void attachBetween(const Component::Ptr& start, bool startToStart, float startOffset, const Component::Ptr& end, bool endToStart, float endOffset)
			{
				attachStartTo(start, startToStart, startOffset);
				attachEndTo(end, endToStart, endOffset);
			}
			inline void attachBetween(const Component::Ptr& start, bool startToStart, const Component::Ptr& end, bool endToStart, float offset)
			{
				attachBetween(start, startToStart, offset, end, endToStart, offset);
			}
			ChainConstraintBuilder(Constraint& c, std::list<Cluster>::iterator cc) : constraint(c), cluster(cc)
			{
				if (c.getType() != Constraint::Type::Chain)
					throw std::logic_error("Wrong constraint type");
			}
		};

		static void registerProperties(Manager& m, const std::string& name);

		/// @brief Sets background color and invalidates visuals.
		/// All colors except that are not fully transparent are converted to fully opaque.
		/// Forces visual invalidation.
		/// @param color new background color.
		void setBackgroundColor(const Color& color);

		virtual void poke() override;

		virtual void onResize(const Rect& bounds) override;
		virtual void onChildStain(Component& c) override;
		virtual void onChildNeedsRedraw(Component& c) override;

		virtual bool handleEvent(const Event& event) override;

		std::pair<DimensionDesc, DimensionDesc> measure(const DimensionDesc& w, const DimensionDesc& h) override;

		virtual void removeChild(const Component::Ptr& child) override;
		virtual void clearChildren() override;
		virtual void addChild(const Component::Ptr& child) override;

		virtual Iterator begin() override;
		virtual Iterator end() override;

		/// @brief Creates regural constraint for given element.
		/// @param orientation either Vertical or Horizontal,
		/// @param target target,
		/// @param constOffset if true, size of target is calculated based on offset from edges, otherwise offset is calculated based on size,
		/// @return Returns wrapper for created constraint.
		std::unique_ptr<RegularConstraintBuilder> addConstraint(Orientation orientation, const Component::Ptr& target, bool constOffset);
		/// @brief Creates chain constraint for given element.
		/// @param orientation either Vertical or Horizontal,
		/// @param targets targets,
		/// @param constOffset if true, size of target is calculated based on offset from edges, otherwise offset is calculated based on size.
		/// @return Returns wrapper for created constraint.
		std::unique_ptr<ChainConstraintBuilder> addChainConstraint(Orientation orientation, const std::vector<Component::Ptr>& targets, bool constOffset);

		virtual void onMaskDraw(Canvas& canvas) const override;
		virtual void onDraw(Canvas& canvas) override;
		virtual void onRedraw(Canvas& canvas) override;

		virtual void postXmlConstruction(Manager& m, const XML::Tag& config, const StylingPack& pack);

		ConstraintsContainer();
	private:
		using IteratorType = CommonIteratorTemplate<Vector < std::shared_ptr<Component> >::iterator>;

		std::vector<Component::Ptr> children;
		std::unordered_map<Component*, Rect> boundaries;
		std::list<Constraint> constraints;
		std::list<Cluster> clusters;

		std::unordered_map<const Component*, std::list<Cluster>::iterator> clusterMapping;
		std::unordered_set<Component*> updated;

		std::unordered_set<Component*> needsRedraw;
		std::unordered_map<Component*, Rect> drawnLastFrame;

		bool firstDraw;

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
	};
}
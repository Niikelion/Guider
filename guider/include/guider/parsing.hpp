#include <string>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <memory>

namespace Guider::Parsing
{
	/*
	* General data layout:
	* <size:32>[size] - size of object, including this field, hash and children
	* <size:8>[type]
	* <size:8>[attrs]
	* (<size:8><size:8>[attr][value]){attrs}
	* <size:32>[hash] - hash of object data preceeding hash field excluding hash field and size
	* <size:16>[childCount]
	* [child]{childCount}
	*/

	class DataObject
	{
	public:
		template<typename T> class IteratorBase
		{
		public:
			virtual T current() const abstract;
			virtual void next() abstract;
			virtual bool equals(const IteratorBase<T>&) const abstract;
			virtual std::unique_ptr<IteratorBase<T>> clone() const abstract;

			virtual ~IteratorBase() = default;
		};

		template<typename T, typename iterator, typename Base> class IteratorTemplate : public IteratorBase<T>
		{
		public:
			virtual T current() const override
			{
				return convert(it);
			}
			virtual T convert(const iterator&) const abstract;
			virtual void next() override
			{
				++it;
			}
			virtual bool equals(const IteratorBase<T>& t) const override
			{
				return static_cast<const IteratorTemplate<T, iterator, Base>&>(t).it == it;
			}
			virtual std::unique_ptr<IteratorBase<T>> clone() const override
			{
				return std::make_unique<Base>(*static_cast<const Base*>(this));
			}

			IteratorTemplate(const iterator& i) : it(i) {}
		private:
			iterator it;
		};

		template<typename T> class Iterator : std::forward_iterator_tag
		{
		public:
			T operator * () const
			{
				return iterator->current();
			}

			Iterator<T>& operator ++ ()
			{
				iterator->next();
				return *this;
			}
			bool operator == (const Iterator<T>& t) const
			{
				return iterator->equals(*t.iterator);
			}

			bool operator != (const Iterator<T>& t) const
			{
				return !iterator->equals(*t.iterator);
			}

			Iterator(std::unique_ptr<IteratorBase<T>>&& i) : iterator(std::move(i)) {}
			Iterator(const Iterator<T>& t) : iterator(t.iterator ? t.iterator->clone() : nullptr) { }
			Iterator(Iterator<T>&& t) noexcept : iterator(std::move(t.iterator)) {}
		private:
			std::unique_ptr<IteratorBase<T>> iterator;
		};

		template<typename T> class Iterable
		{
		public:
			Iterator<T> begin()
			{
				return b;
			}
			Iterator<T> end()
			{
				return e;
			}

			Iterable(const Iterator<T>& bb, const Iterator<T>& ee) : b(bb), e(ee) {}
			Iterable(const Iterable& t) : b(t.b), e(t.e) {}
			~Iterable() {}
		private:
			Iterator<T> b, e;
		};

		virtual std::string_view getType() const abstract;

		virtual Iterable<std::shared_ptr<DataObject>> childrenIt() const abstract;
		virtual Iterable<std::pair<const std::string_view, std::string_view>> attributesIt() const abstract;

		virtual std::string_view getAttribute(const std::string_view& key) const abstract;
	};

	class ParseException : std::exception
	{
	public:
		const char* what() const noexcept override;

		ParseException(const std::string& string);
	private:
		std::string source;
	};

	class MutableObject : public DataObject
	{
	public:
		virtual std::string_view getType() const override;

		virtual Iterable<std::shared_ptr<DataObject>> childrenIt() const override;
		virtual Iterable<std::pair<const std::string_view, std::string_view>> attributesIt() const override;

		virtual std::string_view getAttribute(const std::string_view& key) const override;

		void setType(const std::string& s);
		std::vector<std::shared_ptr<MutableObject>>& getChildren();
		std::unordered_map<std::string, std::string>& getProperties();

		void serialize(std::string& ret) const;
		std::string serialize() const;
	private:
		using Vector = std::vector<std::shared_ptr<MutableObject>>;
		using VectorIterator = Vector::const_iterator;

		using Map = std::unordered_map<std::string, std::string>;
		using MapIterator = Map::const_iterator;

		class VectorIteratorImpl : public IteratorTemplate<std::shared_ptr<DataObject>, VectorIterator, VectorIteratorImpl>
		{
		public:
			virtual std::shared_ptr<DataObject> convert(const VectorIterator& iterator) const override;
			
			using IteratorTemplate::IteratorTemplate;
		};

		class MapIteratorImpl : public IteratorBase<std::pair<const std::string_view, std::string_view>>
		{
		public:
			virtual std::pair<const std::string_view, std::string_view> current() const override;
			virtual void next() override;
			virtual bool equals(const IteratorBase<std::pair<const std::string_view, std::string_view>>&) const override;
			virtual std::unique_ptr<IteratorBase<std::pair<const std::string_view, std::string_view>>> clone() const override;

			MapIteratorImpl(MapIterator iterator) : iterator(iterator) {}
		private:
			MapIterator iterator;
		};

		std::string type;
		Vector children;
		Map properties;
	};

	class ObjectView : public DataObject
	{
	public:
		virtual std::string_view getType() const override;

		virtual Iterable<std::shared_ptr<DataObject>> childrenIt() const override;
		virtual Iterable<std::pair<const std::string_view, std::string_view>> attributesIt() const override;

		virtual std::string_view getAttribute(const std::string_view& key) const override;

		std::string_view getSourceView() const noexcept;

		ObjectView(const std::string& source);
		ObjectView(const std::string_view& source);
		ObjectView(ObjectView&&) noexcept = delete;

		~ObjectView();
	private:
		using ViewMap = std::unordered_map<std::string_view, std::string_view>;
		using ViewMapIterator = ViewMap::const_iterator;

		class ViewMapIteratorImpl : public IteratorBase<std::pair<const std::string_view, std::string_view>>
		{
		public:
			virtual std::pair<const std::string_view, std::string_view> current() const override;
			virtual void next() override;
			virtual bool equals(const IteratorBase<std::pair<const std::string_view, std::string_view>>&) const override;
			virtual std::unique_ptr<IteratorBase<std::pair<const std::string_view, std::string_view>>> clone() const override;

			ViewMapIteratorImpl(ViewMapIterator iterator) : iterator(iterator) {}
			~ViewMapIteratorImpl() {}
		private:
			ViewMapIterator iterator;
		};

		using ViewVector = std::vector<std::shared_ptr<ObjectView>>;
		using ViewVectorIterator = ViewVector::const_iterator;

		class ViewVectorIteratorImpl : public IteratorBase<std::shared_ptr<DataObject>>
		{
		public:
			virtual std::shared_ptr<DataObject> current() const override;
			virtual void next() override;
			virtual bool equals(const IteratorBase<std::shared_ptr<DataObject>>&) const override;
			virtual std::unique_ptr<IteratorBase<std::shared_ptr<DataObject>>> clone() const override;

			ViewVectorIteratorImpl(ViewVectorIterator iterator) : iterator(iterator) {}
		private:
			ViewVectorIterator iterator;
		};

		union
		{
			std::string storage;
			std::string_view storageView;
		};

		std::string_view type;
		ViewMap properties;
		std::vector<std::shared_ptr<ObjectView>> children;

		bool owner;

		void parse();
	};
}
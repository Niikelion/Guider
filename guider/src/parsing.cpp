#include <guider/parsing.hpp>
#include <type_traits>
#include <cassert>

namespace Guider::Parsing
{
	template<typename T> struct pure_mem {};
	template<typename T> using pure_mem_t = typename pure_mem<T>::type;
	template<> struct pure_mem<uint8_t> { using type = uint8_t; };
	template<> struct pure_mem<uint16_t> { using type = uint16_t; };
	template<> struct pure_mem<uint32_t> { using type = uint32_t; };
	template<> struct pure_mem<uint64_t> { using type = uint64_t; };

	bool checkLittleEndian()
	{
		uint16_t a = 1;
		return (*reinterpret_cast<char*>(&a)) != 0;
	}

	static const bool isLittleEndian = checkLittleEndian();

	template<typename T>T mts(T x)
	{
		if (isLittleEndian)
			return x;
		using ST = pure_mem_t<T>;
		using uT = std::make_unsigned_t<T>;
		T ret;
		uT& uX = reinterpret_cast<uT&>(x);
		uint8_t* buff = reinterpret_cast<uint8_t*>(&ret);
		uint64_t off = 0xFF;
		for (size_t i = 0; i < sizeof(uT); ++i)
		{
			buff[i] = uint8_t((uX & off) >> (i*8));
			off <<= 8;
		}
		return ret;
	}

	template<typename T>T stm(T x)
	{
		return mts<T>(x); //yes, effectively, mts and stm just swap bytes on big-endian systems, do nothing otherwise
	}

	class ParseWrapper
	{
	public:
		ParseWrapper(const std::string_view& v, size_t off = 0, size_t size = 0) : source(v.substr(off, size == 0 ? v.size() - off : size)), offset(0) { }

		const std::string_view peek(size_t size)
		{
			return source.substr(offset, size);
		}

		template<typename T>T peek()
		{
			return stm<T>(*reinterpret_cast<const T*>(peek(sizeof(T)).data()));
		}

		const std::string_view get(size_t size)
		{
			offset += size;
			return source.substr(offset - size, size);
		}

		void put(const char* str, size_t size)
		{
			memcpy(const_cast<char*>(source.data() + offset), str, size);
			offset += size;
		}

		template<typename T>void put(T v)
		{
			size_t size = sizeof(T);
			//translate v to storage form
			v = mts<T>(v);
			put(reinterpret_cast<char*>(&v), size);
		}

		template<typename T>T get()
		{
			size_t size = sizeof(T);
			return stm<T>(*reinterpret_cast<const T*>(get(size).data()));
		}

		size_t remainingSize()
		{
			return (source.size() > offset ? source.size() - offset : 0);
		}

		bool canGet(size_t size)
		{
			return remainingSize() >= size;
		}

		template<typename T> bool canGet()
		{
			return canGet(sizeof(T));
		}

		void resetCounter()
		{
			offset = 0;
		}
	private:
		std::string_view source;
		size_t offset;
	};

	const char* ParseException::what() const noexcept
	{
		return source.c_str();
	}

	ParseException::ParseException(const std::string& source) : source(source) {}

	std::string_view MutableObject::getType() const
	{
		return std::string_view(type);
	}

	MutableObject::Iterable<std::shared_ptr<DataObject>> MutableObject::childrenIt()
	{
		return Iterable<std::shared_ptr<DataObject>>(
			Iterator<std::shared_ptr<DataObject>>(std::make_unique<VectorIteratorImpl>(children.begin())),
			Iterator<std::shared_ptr<DataObject>>(std::make_unique<VectorIteratorImpl>(children.end()))
		);
	}

	MutableObject::Iterable<std::pair<const std::string_view, std::string_view>> MutableObject::attributesIt()
	{
		return Iterable<std::pair<const std::string_view, std::string_view>>(
			Iterator<std::pair<const std::string_view, std::string_view>>(std::make_unique<MapIteratorImpl>(properties.begin())),
			Iterator<std::pair<const std::string_view, std::string_view>>(std::make_unique<MapIteratorImpl>(properties.end()))
		);
	}

	std::string_view MutableObject::getAttribute(const std::string_view& key) const
	{
		auto it = properties.find(std::string(key));
		if (it != properties.end())
			return std::string_view(it->second);
		return std::string_view();
	}


	void MutableObject::setType(const std::string& s)
	{
		type = s;
	}

	std::vector<std::shared_ptr<MutableObject>>& MutableObject::getChildren()
	{
		return children;
	}

	std::unordered_map<std::string, std::string>& MutableObject::getProperties()
	{
		return properties;
	}

	void MutableObject::serialize(std::string& ret) const
	{
		size_t sizeNeeded = 2 * sizeof(uint32_t) + (3 + 2 * properties.size()) * sizeof(uint8_t) + sizeof(uint16_t);
		for (const auto& i : properties)
			sizeNeeded += i.first.size() + i.second.size();

		size_t off = ret.size();
		ret.resize(off + sizeNeeded);
		for (const auto& child : children)
		{
			child->serialize(ret);
		}
		//write values
		size_t size = ret.size() - off;
		ParseWrapper p = ParseWrapper(std::string_view(ret));
		
		assert(p.canGet<uint32_t>());
		p.put<uint32_t>(size);
		//if 
	}

	std::string MutableObject::serialize() const
	{
		std::string ret;
		serialize(ret);
		return ret;
	}

	std::shared_ptr<DataObject> MutableObject::VectorIteratorImpl::current() const
	{
		return std::static_pointer_cast<DataObject>(*iterator);
	}

	void MutableObject::VectorIteratorImpl::next()
	{
		++iterator;
	}

	bool MutableObject::VectorIteratorImpl::equals(const IteratorBase<std::shared_ptr<DataObject>>& t) const
	{
		return static_cast<const VectorIteratorImpl&>(t).iterator == iterator;
	}

	std::pair<const std::string_view, std::string_view> MutableObject::MapIteratorImpl::current() const
	{
		return std::pair<const std::string_view, std::string_view>(iterator->first, iterator->second);
	}

	void MutableObject::MapIteratorImpl::next()
	{
		++iterator;
	}

	bool MutableObject::MapIteratorImpl::equals(const IteratorBase<std::pair<const std::string_view, std::string_view>>& t) const
	{
		return static_cast<const MapIteratorImpl&>(t).iterator == iterator;
	}

	std::unique_ptr<MutableObject::IteratorBase<std::pair<const std::string_view, std::string_view>>> MutableObject::MapIteratorImpl::clone() const
	{
		return std::make_unique<MapIteratorImpl>(iterator);
	}

	std::unique_ptr<MutableObject::IteratorBase<std::shared_ptr<DataObject>>> MutableObject::VectorIteratorImpl::clone() const
	{
		return std::make_unique<VectorIteratorImpl>(iterator);
	}

	std::string_view ObjectView::getType() const
	{
		return type;
	}

	ObjectView::Iterable<std::shared_ptr<DataObject>> ObjectView::childrenIt()
	{
		return Iterable<std::shared_ptr<DataObject>>(
			Iterator<std::shared_ptr<DataObject>>(std::make_unique<ViewVectorIteratorImpl>(children.begin())),
			Iterator<std::shared_ptr<DataObject>>(std::make_unique<ViewVectorIteratorImpl>(children.end()))
		);
	}

	ObjectView::Iterable<std::pair<const std::string_view, std::string_view>> ObjectView::attributesIt()
	{
		using T = std::pair<const std::string_view, std::string_view>;
		
		auto b = Iterator<T>(std::make_unique<ViewMapIteratorImpl>(properties.begin()));
		auto e = Iterator<T>(std::make_unique<ViewMapIteratorImpl>(properties.end()));

		return Iterable<T>(std::move(b),std::move(e));
	}

	std::string_view ObjectView::getAttribute(const std::string_view& key) const
	{
		auto it = properties.find(key);
		if (it != properties.end())
			return it->second;
		return std::string_view();
	}

	std::string_view ObjectView::getSourceView() const noexcept
	{
		return owner ? std::string_view(storage) : storageView;
	}

	ObjectView::ObjectView(const std::string& source) : storage(source), owner(true)
	{
		parse();
	}
	ObjectView::ObjectView(const std::string_view& source) : storageView(source), owner(false)
	{
		parse();
	}
	
	ObjectView::~ObjectView()
	{
		properties.clear();
		children.clear();
		if (owner)
		{
			storage.~basic_string();
		}
		else
		{
			storageView.~basic_string_view();
		}
	}
	void ObjectView::parse()
	{
		ParseWrapper wrapper(getSourceView());
		//header
		{
			if (!wrapper.canGet<uint32_t>())
				throw ParseException("Missing node size");
			uint32_t size = wrapper.get<uint32_t>();
			if (!wrapper.canGet(size - sizeof(uint32_t)))
				throw ParseException("Corrupted node size");
		}
		//type
		{
			uint8_t typeSize = wrapper.get<uint8_t>();
			if (!wrapper.canGet(typeSize))
				throw ParseException("Corrupted type name");
			type = wrapper.get(typeSize);
		}
		//attributes
		{
			if (!wrapper.canGet<uint8_t>())
				throw ParseException("Corrupted attributes count");
			uint8_t attrs = wrapper.get<uint8_t>();
			for (uint8_t i = 0; i < attrs; ++i)
			{
				if (!wrapper.canGet(sizeof(uint8_t) * 2))
					throw ParseException("Missing attribute header");
				uint8_t keySize = wrapper.get<uint8_t>();
				uint8_t valueSize = wrapper.get<uint8_t>();

				if (!wrapper.canGet(keySize + valueSize))
					throw ParseException("Missing attribute data");

				auto key = wrapper.get(keySize);
				auto value = wrapper.get(valueSize);
				properties.emplace(key, value);
			}
		}
		//hash
		{
			//TODO: calc and check hash
			wrapper.get<uint32_t>();
		}
		//children
		{
			if (!wrapper.canGet<uint16_t>())
				throw ParseException("Missing child count");
			uint16_t childCount = wrapper.get<uint16_t>();
			for (uint16_t i = 0; i < childCount; ++i)
			{
				if (!wrapper.canGet<uint32_t>())
					throw ParseException("Corrupted child header");
				uint32_t childSize = wrapper.peek<uint32_t>();
				if (!wrapper.canGet(childSize))
					throw ParseException("Corrupted child data");
				children.emplace_back(std::make_shared<ObjectView>(wrapper.get(childSize)));
			}
		}
	}
	std::pair<const std::string_view, std::string_view> ObjectView::ViewMapIteratorImpl::current() const
	{
		return *iterator;
	}
	void ObjectView::ViewMapIteratorImpl::next()
	{
		++iterator;
	}
	bool ObjectView::ViewMapIteratorImpl::equals(const IteratorBase<std::pair<const std::string_view, std::string_view>>& t) const
	{
		return static_cast<const ViewMapIteratorImpl&>(t).iterator == iterator;
	}
	std::unique_ptr<ObjectView::IteratorBase<std::pair<const std::string_view,std::string_view>>> ObjectView::ViewMapIteratorImpl::clone() const
	{
		return std::make_unique<ViewMapIteratorImpl>(iterator);
	}
	std::shared_ptr<DataObject> ObjectView::ViewVectorIteratorImpl::current() const
	{
		return std::static_pointer_cast<DataObject>(*iterator);
	}
	void ObjectView::ViewVectorIteratorImpl::next()
	{
		++iterator;
	}
	bool ObjectView::ViewVectorIteratorImpl::equals(const IteratorBase<std::shared_ptr<DataObject>>& t) const
	{
		return static_cast<const ViewVectorIteratorImpl&>(t).iterator == iterator;
	}
	std::unique_ptr<ObjectView::IteratorBase<std::shared_ptr<DataObject>>> ObjectView::ViewVectorIteratorImpl::clone() const
	{
		return std::make_unique<ViewVectorIteratorImpl>(iterator);
	}
}
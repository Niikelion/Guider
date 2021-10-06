#include <guider/parsing.hpp>
#include <type_traits>

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
		uT off = 0xFF;
		for (size_t i = 0; i < sizeof(uT); ++i)
		{
			buff[i] = uX & off;
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
			return stm<T>(*reinterpret_cast<const T*>(get(sizeof(T)).data()));
		}

		const std::string_view get(size_t size)
		{
			offset += size;
			return source.substr(offset - size, size);
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

	std::string ObjectView::getType() const
	{
		return std::string(type);
	}

	size_t ObjectView::getChildCount() const
	{
		return children.size();
	}

	std::shared_ptr<DataObject> ObjectView::getChild(size_t i) const
	{
		return children.at(i);
	}

	size_t ObjectView::getAttributesCount() const
	{
		return properties.size();
	}

	std::pair<std::string, std::string> ObjectView::getAttribute(size_t i) const
	{
		auto r = props.at(i);
		return {std::string(r->first), std::string(r->second)};
	}

	std::string ObjectView::getAttribute(const std::string& key) const
	{
		return std::string(properties.at(key));
	}

	std::string_view ObjectView::getSourceView() const noexcept
	{
		return owner ? std::string_view(storage) : storageView;
	}
	const std::vector<std::shared_ptr<ObjectView>>& ObjectView::getChildren() const noexcept
	{
		return children;
	}
	ObjectView::ObjectView(const std::string& source) : storage(source), owner(true)
	{
		parse();
	}
	ObjectView::ObjectView(const std::string_view& source) : storageView(source), owner(false)
	{
		parse();
	}
	ObjectView::ObjectView(ObjectView&&) noexcept
	{
		//TODO: do
	}
	ObjectView::~ObjectView()
	{
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
			if (!wrapper.canGet(size))
				throw ParseException("Corrupted node size");
		}
		//type
		{
			uint8_t typeSize = wrapper.get<uint8_t>();
			if (!wrapper.canGet(typeSize))
			{
				type = wrapper.get(typeSize);
			}
		}
		//attributes
		{
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
				auto r = properties.emplace(key, value);
				if (r.second)
				{
					props.emplace_back(r.first);
				}
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
			uint16_t childCount = wrapper.peek<uint16_t>();
			for (uint16_t i = 0; i < childCount; ++i)
			{
				if (!wrapper.canGet<uint32_t>())
					throw ParseException("Corrupted child header");
				uint32_t childSize = wrapper.get<uint16_t>();
				if (!wrapper.canGet(childSize))
					throw ParseException("Corrupted child data");
				children.emplace_back(std::make_shared<ObjectView>(wrapper.get(childSize)));
			}
		}
	}
}
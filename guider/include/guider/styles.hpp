#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <typeindex>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <Guider/base.hpp>

namespace Guider
{
	namespace Styles
	{
		static bool isDigitInBase(char c, unsigned base);
		static unsigned digitFromChar(char c);

		std::string trim(const std::string& s);

		std::vector<std::string> splitString(const std::string& str);

		uint64_t strToInt(const std::string& str, bool& failed, unsigned base = 10);
		inline uint64_t strToInt(const std::string& str, unsigned base = 10)
		{
			bool a;
			return strToInt(str, a, base);
		}
		float strToFloat(const std::string& str, bool& failed);
		inline float strToFloat(const std::string& str)
		{
			bool failed = false;
			return strToFloat(str, failed);
		}
		bool strToBool(const std::string& str, bool& failed);
		inline bool strToBool(const std::string& str)
		{
			bool failed = false;
			return strToBool(str, failed);
		}
		Color strToColor(const std::string& str, bool& failed);
		inline Color strToColor(const std::string& str)
		{
			bool failed = false;
			return strToColor(str, failed);
		}
	}

	namespace
	{
		template<class...>struct types { using type = types; };
		template<class T>struct tag { using type = T; };
		template<class Tag>using type_t = typename Tag::type;
	}

	class Style
	{
	public:
		class Value
		{
		public:
			template<typename T> bool checkType()
			{
				return typeid(T) == type;
			}

			inline std::type_index getType()
			{
				return type;
			}

			template<typename T> T& as()
			{
				if (type != typeid(T))
					throw std::logic_error("Invalid type");
				if (pointer == nullptr)
				{
					prepare<T>();
				}
				return *static_cast<T*>(pointer);
			}

			template<typename T,typename ...Args> void prepare(Args... args)
			{
				if (pointer == nullptr && type == typeid(T))
				{
					data.resize(sizeof(T));
					pointer = new(data.data()) T(std::forward<Args>(args)...);
					deleter = deleterFunc<T>;
					cloner = clonerFunc<T>;
				}
			}

			template<typename T> Value(tag<T>) : Value(typeid(T), sizeof(T))
			{
				prepare<T>();
			}

			Value(const std::type_index& t, size_t size) : deleter(nullptr), cloner(nullptr), data(size), pointer(nullptr), type(t) {}
			Value(const Value& t) : deleter(t.deleter), cloner(t.cloner), pointer(nullptr), type(t.type)
			{
				if (t.pointer != nullptr && cloner != nullptr)
				{
					data.resize(t.data.size());
					pointer = cloner(t.pointer);
				}
			}
			Value(Value&& t) noexcept : deleter(t.deleter), cloner(t.cloner), data(std::move(t.data)), pointer(t.pointer), type(std::move(t.type))
			{
				t.pointer = nullptr;
				t.data.clear();
				t.deleter = nullptr;
				t.cloner = nullptr;
			}
			~Value()
			{
				if (pointer != nullptr && deleter != nullptr)
					deleter(pointer);
			}
		private:
			template<typename T> static void deleterFunc(void* p)
			{
				(*static_cast<T*>(p)).~T();
			}
			template<typename T> static void* clonerFunc(void* p)
			{
				return new T(*static_cast<T*>(p));
			}
			void(*deleter)(void*);
			void*(*cloner)(void*);
			std::vector<uint8_t> data;
			void* pointer;
			std::type_index type;
		};

		class ValueDefinition
		{
		public:
			template<typename T> static ValueDefinition create()
			{
				return ValueDefinition(creatorFunc<T>);
			}
			template<typename T> static ValueDefinition createEmpty()
			{
				return ValueDefinition(genericCreatorFunc<T>);
			}

			Value value(const std::string& str)
			{
				if (creator)
					return creator(str);
				throw std::logic_error("Creator not specified");
			}
			ValueDefinition(const std::function<Value(const std::string&)>& f) : creator(f) {}
			ValueDefinition(const ValueDefinition&) = default;
			ValueDefinition(ValueDefinition&&) noexcept = default;
		private:
			std::function<Value(const std::string&)> creator;

			template<typename T> static Value creatorFunc(const std::string&)
			{
				return Value(tag<T>{});
			}
			template<typename T> static Value genericCreatorFunc(const std::string&)
			{
				return Value(typeid(T), sizeof(T));
			}
		};

		void inherit(const Style& parentStyle);
		void inheritValues(const Style& parentStyle);
		void inheritAliases(const Style& parentStyle);

		std::shared_ptr<Value> getValue(const std::string& name) const;
		void setValue(const std::string& name,const std::shared_ptr<Value>& value);
		void setAlias(const std::string& name, const std::string& alias);
	private:
		std::unordered_map<std::string, std::shared_ptr<Value>> values;
		std::unordered_map<std::string, std::string> aliases;
	};
}
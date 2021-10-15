#pragma once

#include <guider/definitions.hpp>

#include <functional>
#include <typeindex>
#include <stdexcept>
#include <algorithm>
#include <memory>

namespace Guider
{
	namespace Styles
	{
		namespace
		{
			template<class...> struct types { using type = types; };
			template<class T> struct tag { using type = T; };
			template<class Tag> using type_t = typename Tag::type;
		}

		class Value
		{
		public:
			using Ptr = std::shared_ptr<Value>;

			template<typename T, typename ...Args> static Value::Ptr ofType(Args&&... args)
			{
				Value::Ptr ret = std::make_shared<Value>(typeid(T), sizeof(T));
				ret->prepare<T>(std::forward<Args>(args)...);
				return ret;
			}

			template<typename T> bool checkType()
			{
				return type == typeid(T);
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
					throw std::runtime_error("Missing value");
				}
				return *static_cast<T*>(pointer);
			}

			template<typename T, typename ...Args> void prepare(Args&&... args)
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
					pointer = cloner(t.pointer, data.data());
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
				static_cast<T*>(p)->~T();
			}
			template<typename T> static void* clonerFunc(void* p, uint8_t* buffer)
			{
				return new(buffer) T(*static_cast<T*>(p));
			}
			void(*deleter)(void*);
			void* (*cloner)(void*, uint8_t*);
			Vector<uint8_t> data;
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

			Value value(const String& str)
			{
				if (creator)
					return creator(str);
				throw std::logic_error("Creator not specified");
			}
			ValueDefinition(const std::function<Value(const String&)>& f) : creator(f) {}
			ValueDefinition(const ValueDefinition&) = default;
			ValueDefinition(ValueDefinition&&) noexcept = default;
		private:
			std::function<Value(const String&)> creator;

			template<typename T> static Value creatorFunc(const String&)
			{
				return Value(tag<T>{});
			}
			template<typename T> static Value genericCreatorFunc(const String&)
			{
				return Value(typeid(T), sizeof(T));
			}
		};

		class Variable
		{
		public:
			bool isReference() const;

			String getValue() const;

			Variable(const String& value, bool reference);
		private:
			bool reference;
			String value;
		};

		class VariableReference
		{
		public:
			bool detached() const noexcept;
			void attach(const Value::Ptr& value);

			String getName() const;
			std::shared_ptr<Value> getValue() const;

			VariableReference() : name() {}
			VariableReference(const String& n) : name(n) {}
			VariableReference(const String& n, const Value::Ptr& c) : name(n), cache(c) {};
			VariableReference(const VariableReference&) = default;
			VariableReference(VariableReference&&) noexcept = default;
		private:
			String name;
			Value::Ptr cache;
		};

		class UnresolvedValue
		{
		public:
			String getValue();

			UnresolvedValue(const String& value);
		private:
			String value;
		};

		static bool isDigitInBase(char c, unsigned base);
		static unsigned digitFromChar(char c);

		String trim(const String& s);

		Vector<String> splitString(const String& str);

		uint64_t strToInt(const String& str, bool& failed, unsigned base = 10);
		inline uint64_t strToInt(const String& str, unsigned base = 10)
		{
			bool a;
			return strToInt(str, a, base);
		}
		float strToFloat(const String& str, bool& failed);
		inline float strToFloat(const String& str)
		{
			bool failed = false;
			return strToFloat(str, failed);
		}
		bool strToBool(const String& str, bool& failed);
		inline bool strToBool(const String& str)
		{
			bool failed = false;
			return strToBool(str, failed);
		}
		Color strToColor(const String& str, bool& failed);
		inline Color strToColor(const String& str)
		{
			bool failed = false;
			return strToColor(str, failed);
		}
	}

	class Style
	{
	public:
		void inheritAttributes(const Style& parentStyle);

		Styles::Value::Ptr getAttribute(const String& name) const;

		void setAttribute(const String& name, const Styles::Value::Ptr& value);
		void setAttribute(const String& name, const String& variable);
		void removeAttribute(const String& name);

		HashMap<String, Styles::Value::Ptr> attributes;
	private:
	};

	class Theme
	{
	public:
		void setVariable(const String& name, const std::shared_ptr<Styles::Variable>& value);
		std::shared_ptr<Styles::Variable> getVariable(const String& name) const;

		std::shared_ptr<Styles::Variable> dereferenceVariable(const String& name) const;

		void inheritVariables(const Theme& parentTheme);
	private:
		HashMap<String, std::shared_ptr<Styles::Variable>> variables;
	};

	struct StylingPack
	{
		Style style;
		Theme theme;
	};
}
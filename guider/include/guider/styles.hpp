#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <typeindex>
#include <stdexcept>
#include <Guider/base.hpp>

namespace Guider
{
	namespace Parsing
	{
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

	class Style
	{
	public:
		class Value
		{
		public:
			template<typename T> T& as()
			{
				if (pointer != nullptr && typeid(T) == type)
				{
					return *static_cast<T*>(pointer);
				}
				throw std::logic_error("Invalid type");
			}
		private:
			std::vector<uint8_t> data;
			void* data;
			std::type_index type;
		};

		void inherit(const Style& parentStyle);

		std::shared_ptr<Value> getValue(const std::string& name);
	private:
		std::unordered_map<std::string, std::shared_ptr<Value>> values;
	};
}
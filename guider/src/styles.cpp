#include <guider/styles.hpp>

namespace Guider
{
	void Style::inherit(const Style& parentStyle)
	{
		for (const auto& value : parentStyle.values)
			if (!values.count(value.first))
				values.emplace(value);
	}
	std::shared_ptr<Style::Value> Style::getValue(const std::string& name)
	{
		auto it = values.find(name);
		if (it != values.end())
			return it->second;
		return std::shared_ptr<Value>();
	}
}
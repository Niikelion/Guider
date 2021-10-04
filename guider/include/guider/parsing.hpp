#include <string>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <memory>

namespace Guider::Parsing
{
	class MutableObject
	{
		//
	};

	class ObjectView
	{
	public:
		std::string_view getSourceView() const noexcept;
		const std::vector<std::shared_ptr<ObjectView>>& getChildren() const noexcept;

		ObjectView(const std::string& source);
		ObjectView(const std::string_view& source);
		ObjectView(ObjectView&&) noexcept;
	private:
		union
		{
			std::string storage;
			std::string_view storageView;
		};

		std::unordered_map<std::string_view, std::string_view> properties;
		std::vector<std::shared_ptr<ObjectView>> children;

		bool owner;

		void parse();
	};
}
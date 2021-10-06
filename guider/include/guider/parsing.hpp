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
		//TODO: change getting size & alements at index to iterators
		virtual std::string getType() const abstract;
		virtual size_t getChildCount() const abstract;
		virtual std::shared_ptr<DataObject> getChild(size_t i) const abstract;
		virtual size_t getAttributesCount() const abstract;
		virtual std::pair<std::string, std::string> getAttribute(size_t i) const abstract;
		virtual std::string getAttribute(const std::string& key) const abstract;
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
		//TODO: implement
		virtual std::string getType() const override;
		virtual size_t getChildCount() const override;
		virtual std::shared_ptr<DataObject> getChild(size_t i) const override;
		virtual size_t getAttributesCount() const override;
		virtual std::pair<std::string, std::string> getAttribute(size_t i) const override;
		virtual std::string getAttribute(const std::string& key) const override;

		std::vector<std::shared_ptr<MutableObject>>& getChildren() const;

	private:
		std::vector<std::shared_ptr<MutableObject>> children;
		std::unordered_map<std::string, std::string> properties;
	};

	class ObjectView : public DataObject
	{
	public:
		virtual std::string getType() const override;
		virtual size_t getChildCount() const override;
		virtual std::shared_ptr<DataObject> getChild(size_t i) const override;
		virtual size_t getAttributesCount() const override;
		virtual std::pair<std::string, std::string> getAttribute(size_t i) const override;
		virtual std::string getAttribute(const std::string& key) const override;

		std::string_view getSourceView() const noexcept;
		const std::vector<std::shared_ptr<ObjectView>>& getChildren() const noexcept;

		ObjectView(const std::string& source);
		ObjectView(const std::string_view& source);
		ObjectView(ObjectView&&) noexcept;

		~ObjectView();
	private:
		union
		{
			std::string storage;
			std::string_view storageView;
		};

		std::string_view type;
		std::unordered_map<std::string_view, std::string_view> properties;
		std::vector<std::unordered_map<std::string_view, std::string_view>::const_iterator> props;
		std::vector<std::shared_ptr<ObjectView>> children;

		bool owner;

		void parse();
	};
}
#include <guider/parsing.hpp>

namespace Guider::Parsing
{
	//TODO: prepare functions to save bytes in one endiannes
	/*
	uint16_t mtdu16(uint16_t x)
	{
		//return x & 0xFF00;
	}
	*/

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
	void ObjectView::parse()
	{
	}
}
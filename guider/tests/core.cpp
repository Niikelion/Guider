#include <gtest/gtest.h>
#include <guider/parsing.hpp>

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

constexpr const char* sampleFile = "\x3F\x0\x0\x0"\
"\x8template"/*type of parent*/\
"\x2"/*number of attributes*/\
	"\x4\x4" "hide" "true"/*first attribute*/\
	"\x5\x7" "theme" "default"/*second attribute*/\
"\x0\x0\x0\x0"/*hash*/\
"\x1\x0"/*number of children*/\
	"\x13\x0\x0\x0"\
	"\x3" "div"/*first child*/\
	"\x1"/*number of attributes*/\
		"\x1\x1" "a" "b"/*first attribute*/\
	"\x0\x0\x0\x0"/*hash of child*/\
	"\x0\x0"/*number of children*/
;

constexpr size_t sampleFileSize = 63;

TEST(Parsing, MutableObject_BasicUnit)
{
	//
}

TEST(Parsing, ObjectView_BasicUnit)
{
	std::string source = std::string(sampleFile, sampleFileSize);
	std::shared_ptr<Guider::Parsing::ObjectView> obj;

	//parse source
	ASSERT_NO_THROW(obj = std::make_shared<Guider::Parsing::ObjectView>(std::string_view(source)));
	//check type
	ASSERT_EQ(obj->getType(), "template");
	//check properties
	{
		std::unordered_map<std::string, std::string> pA = {
			{"hide", "true"},
			{"theme", "default"}
		};
		for (const auto& i : obj->attributesIt())
		{
			auto it = pA.find(std::string(i.first));
			ASSERT_TRUE(it != pA.end()); //must exist
			ASSERT_EQ(it->second, i.second);
			pA.erase(it);
		}
		ASSERT_TRUE(pA.empty());
	}
	//check children
	{
		std::unordered_map<std::string, std::string> pA = {
			{"a", "b"}
		};
		size_t i = 0;
		for (const auto& child : obj->childrenIt())
		{
			ASSERT_EQ(i, 0);
			ASSERT_EQ(child->getType(), "div");
			//check child properties
			{
				for (const auto& p : child->attributesIt())
				{
					auto it = pA.find(std::string(p.first));
					ASSERT_TRUE(it != pA.end()); //must exist
					ASSERT_EQ(it->second, p.second);
					pA.erase(it);
				}
			}
			//check child children
			for (const auto& cc : child->childrenIt())
				FAIL();
			++i;
		}
	}
}

TEST(Parsing, ObjectViewAndMutableObject_BasicIntegration)
{
	Guider::Parsing::MutableObject s;
	std::shared_ptr<Guider::Parsing::ObjectView> obj;
	//construct object

	ASSERT_NO_THROW(obj = std::make_shared<Guider::Parsing::ObjectView>(s.serialize()));
	//compare two objects
}
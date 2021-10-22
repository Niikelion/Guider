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

constexpr const char* sampleFile = "\x3F\x00\x00\x00"\
"\x08" "template"/*type of parent*/\
"\x02"/*number of attributes*/\
	"\x04\x04" "hide" "true"/*first attribute*/\
	"\x05\x07" "theme" "default"/*second attribute*/\
"\x0F\x72\x3D\xD1"/*hash*/\
"\x01\x00"/*number of children*/\
	"\x13\x00\x00\x00"\
	"\x03" "div"/*first child*/\
	"\x01"/*number of attributes*/\
		"\x01\x01" "a" "b"/*first attribute*/\
	"\x88\x87\x84\xF3"/*hash of child*/\
	"\x00\x00"/*number of children*/
;

constexpr size_t sampleFileSize = 63;

bool RecursiveCompare(Guider::Parsing::DataObject& a, Guider::Parsing::DataObject& b)
{
	if (a.getType() != b.getType()) return false;
	std::unordered_map<std::string_view, std::string_view> cache;
	for (const auto& prop : a.attributesIt())
	{
		cache.emplace(prop.first, prop.second);
	}
	for (const auto& prop : b.attributesIt())
	{
		auto it = cache.find(prop.first);
		if (it == cache.end()) return false;
		if (it->second != prop.second) return false;
		cache.erase(it);
	}
	auto ca = a.childrenIt(), cb = b.childrenIt();
	auto cae = ca.end(), cbe = cb.end();
	auto cab = ca.begin(), cbb = cb.begin();
	for (; cab != cae && cbb != cbe; ++cab, ++cbb)
	{
		if (!RecursiveCompare(**cab, **cbb))
			return false;
	}

	return cab == cae && cbb == cbe;
}

TEST(Parsing, MutableObject_BasicUnit)
{
	Guider::Parsing::MutableObject obj;
	obj.setType("template");
	obj.getProperties().emplace("hide", "true");
	obj.getProperties().emplace("theme", "default");
	auto child = obj.getChildren().emplace_back(std::make_shared<Guider::Parsing::MutableObject>());
	child->setType("div");
	child->getProperties().emplace("a", "b");
	auto s = obj.serialize();
	ASSERT_EQ(s.size(), sampleFileSize);

	for (unsigned i = 0; i < s.size(); ++i)
	{
		EXPECT_EQ(s.data()[i], sampleFile[i]) << "at i =" << i;
	}
	ASSERT_EQ(memcmp(s.data(),sampleFile, sampleFileSize), 0);
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
	s.setType("Themes");

	ASSERT_NO_THROW(obj = std::make_shared<Guider::Parsing::ObjectView>(s.serialize()));
	//compare two objects
	ASSERT_TRUE(RecursiveCompare(*obj, s));
}
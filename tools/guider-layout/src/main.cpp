#include <iostream>
#include <cstdlib>

#include <app.hpp>

#include <string>
#include <fstream>

using namespace sf;
using namespace std;

using Orientation = Gui::ConstraintsContainer::Constraint::Orientation;
using SizingMode = Gui::Component::SizingMode;

int main(int argc, char* argv[])
{
	unsigned width = 800;
	unsigned height = 450;

	App app;
	app.createWindow(width, height, "Guider layout creator");
	app.showWindow(false);

	app.initManager();

	Guider::BasicButtonComponent::registerProperties(*app.getManager(),"common.Button");

	app.addDefinitions();
	app.loadResources();

	auto enginePointer = app.getEngine();
	Gui::Engine &engine = *enginePointer;
	engine.resize(Gui::Vec2((float)width, (float)height));

	std::string mainLayout;
	{
		std::ifstream t(RESOURCE_FOLDER"/layouts/main_layout.xml");

		if (t.is_open())
		{
			t.seekg(0, std::ios::end);
			mainLayout.reserve(t.tellg());
			t.seekg(0, std::ios::beg);

			mainLayout.assign((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
		}
		else
		{
			std::cout << argv[0] << std::endl;
		}
	}

	auto xmlRoot = Guider::XML::parse(mainLayout);

	Guider::Component::Type root = app.getManager()->instantiate(*static_cast<Guider::XML::Tag*>(xmlRoot->children[0].get()), app.getManager()->getTheme("app.dark").theme);

	std::shared_ptr<Guider::BasicButtonComponent> branchButton = static_pointer_cast<Guider::BasicButtonComponent>(app.getManager()->getElementById("branch_button"));
	branchButton->setText("none");
	
	app.getEngine()->addChild(root);

	app.showWindow(true);

	app.run();
}
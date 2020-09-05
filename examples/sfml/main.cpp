#include <iostream>
#include <cstdlib>

#include <backend.hpp>
#include <guider/base.hpp>
#include <guider/containers.hpp>
#include <guider/components.hpp>
#include <SFML/OpenGL.hpp>

using namespace sf;
using namespace std;

namespace Gui
{
	using namespace Guider;
}

using Orientation = Gui::ConstraintsContainer::Constraint::Orientation;
using SizingMode = Gui::Component::SizingMode;

int main(int argc, char* argv[])
{
	unsigned width = 800;
	unsigned height = 450;

	RenderWindow window(VideoMode(width, height), "Noder", Style::Default, ContextSettings(0, 8, 2));
	/*
	Font font;
	font.loadFromFile("resources/Arimo-Regular.ttf");

	Text text;
	text.setCharacterSize(20);
	text.setString("test");
	text.setFillColor(Color::White);
	text.setFont(font);
	*/

	Gui::SfmlBackend renderer;
	Gui::Engine engine(renderer);
	engine.resize(Gui::Vec2((float)width, (float)height));

	std::shared_ptr<Gui::ConstraintsContainer> container = make_shared<Gui::ConstraintsContainer>();

	engine.container.addChild(container, 0, 0);

	Gui::Component::Type rects[] = {
		make_shared<Gui::RectangleShapeComponent>(SizingMode::MatchParent,Gui::Color(255,0,0)),
		make_shared<Gui::RectangleShapeComponent>(SizingMode::MatchParent,Gui::Color(0,255,0)),
		make_shared<Gui::RectangleShapeComponent>(SizingMode::MatchParent,Gui::Color(0,0,255)),
		make_shared<Gui::RectangleShapeComponent>(SizingMode::MatchParent,Gui::Color(255,255,0)),
		make_shared<Gui::RectangleShapeComponent>(SizingMode::MatchParent,Gui::Color(0,255,255))
	};

	Gui::Component::Type dividers[] = {
		make_shared<Gui::RectangleShapeComponent>(0,0,Gui::Color(0)),
		make_shared<Gui::RectangleShapeComponent>(0,0,Gui::Color(0))
	};

	for (unsigned i = 0; i < 5; ++i)
		container->addChild(rects[i]);

	for (unsigned i = 0; i < 2; ++i)
		container->addChild(dividers[i]);

	std::shared_ptr<Gui::ListContainer> list = std::make_shared<Gui::ListContainer>();
	list->setSizingMode(SizingMode::GivenSize, SizingMode::GivenSize);
	list->addChild(std::make_shared<Gui::RectangleShapeComponent>(200, 50, Gui::Color(255, 255, 255)));
	container->addChild(list);


	auto r1v = container->addConstraint(Orientation::Vertical, rects[0], false);
	auto r1h = container->addConstraint(Orientation::Horizontal, rects[0], true);

	r1v->attachBetween(container, true, nullptr, false, 0);
	r1v->setFlow(0);
	r1v->setSize(25);
	r1h->attachBetween(container, true, container, false, 0);

	auto r2v = container->addConstraint(Orientation::Vertical, rects[1], true);
	auto r2h = container->addConstraint(Orientation::Horizontal, rects[1], false);

	r2v->attachBetween(rects[0], false, container, false, 0);
	r2h->attachBetween(container, true, nullptr, false, 0);
	r2h->setFlow(0);
	r2h->setSize(200);

	auto l1v = container->addConstraint(Orientation::Vertical, list, true);
	auto l1h = container->addConstraint(Orientation::Horizontal, list, false);

	l1v->attachBetween(rects[0], false, container, false, 0);
	l1h->attachBetween(container, true, nullptr, false, 0);
	l1h->setFlow(0);
	l1h->setSize(200);

	auto d1v = container->addConstraint(Orientation::Vertical, dividers[0], false);
	auto d1h = container->addConstraint(Orientation::Horizontal, dividers[0], true);

	d1v->attachBetween(rects[0], false, container, false, 0);
	d1v->setFlow(0.5f);
	d1v->setSize(0);
	d1h->attachBetween(rects[1], false, container, false, 0);

	auto r3v = container->addConstraint(Orientation::Vertical, rects[2], true);
	auto r3h = container->addConstraint(Orientation::Horizontal, rects[2], true);

	r3v->attachBetween(rects[0], false, dividers[0], false, 0);
	r3h->attachBetween(rects[1], false, container, false, 0);

	auto d2v = container->addConstraint(Orientation::Vertical, dividers[1], true);
	auto d2h = container->addConstraint(Orientation::Horizontal, dividers[1], false);

	d2v->attachBetween(dividers[0], false, container, false, 0);
	d2h->attachBetween(rects[1], false, container, false, 0);
	d2h->setFlow(0.5f);
	d2h->setSize(0);

	auto r4v = container->addConstraint(Orientation::Vertical, rects[3], true);
	auto r4h = container->addConstraint(Orientation::Horizontal, rects[3], true);

	r4v->attachBetween(dividers[0], false, container, false, 0);
	r4h->attachBetween(rects[1], false, dividers[1], false, 0);

	auto r5v = container->addConstraint(Orientation::Vertical, rects[4], true);
	auto r5h = container->addConstraint(Orientation::Horizontal, rects[4], true);

	r5v->attachBetween(dividers[0], false, container, false, 0);
	r5h->attachBetween(dividers[1], false, container, false, 0);

	container->setSizingMode(SizingMode::MatchParent, SizingMode::MatchParent);
	//engine.update();

	View view = window.getView();

	view.setCenter(0, 0);
	//view.set
	window.setView(view);

	Cursor cursor;

	Sprite gui;
	gui.setTexture(renderer.target.getTexture());
	gui.setOrigin(width / 2.0f, height / 2.0f);

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case Event::Closed:
			{
				window.close();
				break;
			}
			case Event::Resized:
			{
				sf::FloatRect visibleArea(0, 0, (float)event.size.width, (float)event.size.height);

				view.reset(visibleArea);
				view.setCenter(0, 0);

				Vector2u size(event.size.width, event.size.height);
				window.setView(view);

				engine.resize(Gui::Vec2((float)size.x, (float)size.y));
				gui.setTexture(renderer.target.getTexture(), true);
				gui.setOrigin(event.size.width / 2.0f, event.size.height / 2.0f);
				engine.update();
				break;
			}
			case Event::KeyPressed:
			{
				if (event.key.code == Keyboard::Escape)
					window.close();
				else if (event.key.code == Keyboard::Add)
				{
					int c = 255 / (list->getChildrenCount() + 1);
					list->addChild(std::make_shared<Gui::RectangleShapeComponent>(200, 50, Gui::Color(c, c, c)));
				}
				else if (event.key.code == Keyboard::Subtract)
				{
					if (list->getChildrenCount() > 0)
						list->removeChild(list->getChildrenCount() - 1);
				}
				break;
			}
			default:
				break;
			}
		}

		window.clear(Color(200, 200, 200));


		renderer.target.setActive();
		engine.update();
		engine.draw();

		renderer.target.display();

		window.setActive();
		window.draw(gui);
		window.display();
	}

}
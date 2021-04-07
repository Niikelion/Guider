#include <backend.hpp>
#include <guider/base.hpp>
#include <guider/containers.hpp>
#include <guider/components.hpp>

namespace Gui
{
	using namespace Guider;
}

class App
{
public:
	void createWindow(size_t width, size_t height, const std::string& title);
	void initManager();
	void loadResources();
	void addDefinitions();

	void showWindow(bool show);

	void run();

	inline std::shared_ptr<sf::RenderWindow> getWindow()
	{
		return window;
	}
	inline std::shared_ptr<sf::RenderTexture> getBuffer()
	{
		return buffer;
	}
	inline std::shared_ptr<Gui::Manager> getManager()
	{
		return guiManager;
	}
	inline std::shared_ptr<Gui::Engine> getEngine()
	{
		return engine;
	}
private:
	std::shared_ptr<sf::RenderWindow> window;
	std::shared_ptr<Gui::Manager> guiManager;
	std::shared_ptr<Gui::Engine> engine;
	std::shared_ptr<Gui::Backend> backend;
	std::shared_ptr<sf::RenderTexture> buffer;
	std::shared_ptr<sf::Sprite> guiSprite;
};

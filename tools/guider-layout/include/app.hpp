#include <backend.hpp>
#include <guider/base.hpp>
#include <guider/containers.hpp>
#include <guider/components.hpp>

namespace Gui
{
	using namespace Guider;
}

class Timer
{
public:
	void start()
	{
		clock.restart();
	}

	void sample()
	{
		int t = clock.restart().asMilliseconds();
		history.push_back(t);
		if (history.size() > maxSamples)
			history.erase(history.begin());
		if (t > max)
			max = t;
		if (t < min || min == -1)
			min = t;
	}
	int getMin()
	{
		return min;
	}
	int getMax()
	{
		return max;
	}
	int getAverage()
	{
		if (history.size() == 0)
			return 0;
		int sum = 0;
		for (int i : history)
		{
			sum += i;
		}
		return (sum + history.size() - 1) / history.size();
	}
	void reset()
	{
		min = -1;
		max = 0;
		history.clear();
		clock.restart();
	}

	Timer(int maxS) : max(0), min(-1), maxSamples(maxS) {}
private:
	sf::Clock clock;
	int max;
	int min;
	std::vector<int> history;
	int maxSamples;
};

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

	inline void resetTimer()
	{
		timer.reset();
	}

	App() : timer(10) {}
private:
	std::shared_ptr<sf::RenderWindow> window;
	std::shared_ptr<Gui::Manager> guiManager;
	std::shared_ptr<Gui::Engine> engine;
	std::shared_ptr<Gui::Backend> backend;
	std::shared_ptr<sf::RenderTexture> buffer;
	std::shared_ptr<sf::Sprite> guiSprite;

	Timer timer;
};

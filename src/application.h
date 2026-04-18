#pragma once
#include <string>
#include "layer_stack.h"

struct GLFWwindow;

class Application {
private:
	static Application* instance;

private:
	LayerStack* layerStack;
	GLFWwindow* window;
	float deltaTime;
	bool running;

public:
	Application(const std::string& windowTitle, int windowWidth, int windowHeight);
	~Application();

	static Application* current() { return instance; }

	virtual void onUpdate() = 0;
	void run(int fpsLimit = 70);

	bool isRunning() const { return running; }

	float getDeltaTime() const { return deltaTime; }
	inline float delta() const { return getDeltaTime(); }
	GLFWwindow* getWindow() const { return window; }
	LayerStack& layers() { return *layerStack; }
};
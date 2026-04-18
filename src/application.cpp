#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "application.h"

Application* Application::instance = nullptr;

Application::Application(const std::string& windowTitle, int windowWidth, int windowHeight)
	: deltaTime(1.0f), running(false) {
	if (instance != nullptr) {
		throw std::runtime_error("Only one instance of an application can be launched inside the process!");
	}
	instance = this;

	if (!glfwInit()) {
		throw std::runtime_error("Can't initialize GLFW!");
	}

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Window creation error");
	}

	glfwMakeContextCurrent(window);
	layerStack = new LayerStack(window);
}

Application::~Application() {
	delete layerStack;
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::run(int fpsLimit) {
	running = true;
	float frameDuration = 1000.0f / fpsLimit;

	//std::thread fpsCounter(countFps, window, std::ref(elapsedTime));
	auto beg = std::chrono::steady_clock::now();
	auto end = beg;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		end = std::chrono::steady_clock::now();
		auto frameEnd = end + std::chrono::milliseconds(1000 / fpsLimit);
		std::chrono::duration<float> diff = end - beg;
		deltaTime = diff.count();
		beg = end;

		onUpdate();
		layerStack->update();

		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_until(frameEnd);
	}

	running = false;
}
#pragma once
#include <vector>
#include "layer.h"

struct GLFWwindow;

class LayerStack {
private:
	std::vector<ILayer*> layers;
	GLFWwindow* window;
	float time;

public:
	LayerStack(GLFWwindow* window);
	~LayerStack();

	void update();
	ILayer* pop();
	void attach(ILayer* layer);

	template <typename T, typename... Args>
	T* attach(Args&&... args);
};

template <typename T, typename... Args>
T* LayerStack::attach(Args&&... args) {
	T* newAttachment = new T(std::forward<Args>(args)...);
	layers.push_back(newAttachment);
	newAttachment->onAttach();
	return newAttachment;
}
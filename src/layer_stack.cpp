#include "layer_stack.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "GLFW/glfw3.h"
#include "imgui.h"

LayerStack::LayerStack(GLFWwindow* _window) : window(_window), time(1.0f) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
}

LayerStack::~LayerStack() {
	for (ILayer* layer : layers) {
		layer->onDetach();
		delete layer;
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void LayerStack::update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	io.DisplaySize = ImVec2((float)width, (float)height);

	float newTime = glfwGetTime();
	io.DeltaTime = (float)(newTime - time);
	time = newTime;

	for (ILayer* layer : layers) layer->onUpdate();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

ILayer* LayerStack::pop() {
	ILayer* layer = layers.back();
	layer->onDetach();
	return layer;
}

void LayerStack::attach(ILayer* layer) {
	layers.push_back(layer);
	layer->onAttach();
}
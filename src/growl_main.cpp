#include <imgui.h>
#include <imgui_stdlib.h>
#include <imfilebrowser.h>
#include <GLFW/glfw3.h>
#include <print>

#include "string_converting.h"
#include "application.h"
#include "growl.h"

class DemoLayer final : public ILayer {
private:
public:
	DemoLayer() {}

	void onUpdate() override {
		ImGui::ShowDemoWindow();
	}

	void onAttach() override {
		std::println("Demo attached");
	}

	void onDetach() override {
		std::println("Demo detached");
	}
};


int main() {
	sconv::setProperEncoding();
	Application* growlApp = new Growl();
	growlApp->run(64);
	delete growlApp;
	return 0;
}
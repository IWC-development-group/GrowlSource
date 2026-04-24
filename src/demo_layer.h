#pragma once

#include <imgui.h>
#include "layer.h"

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
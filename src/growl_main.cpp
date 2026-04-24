#include <imgui.h>
#include <imgui_stdlib.h>
#include <imfilebrowser.h>
#include <GLFW/glfw3.h>
#include <print>

#include "string_converting.h"
#include "application.h"
#include "growl.h"

int main() {
	sconv::setProperEncoding();
	Application* growlApp = new Growl();
	growlApp->run(64);
	delete growlApp;
	return 0;
}
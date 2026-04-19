#include <imgui.h>
#include <imgui_stdlib.h>
#include <GLFW/glfw3.h>
#include <print>

#include "application.h"
#include "event.h"
#include "session.h"
#include "instances.h"

#ifdef _WIN32
#include <windows.h>
#endif

std::string ConvertCP1251ToUTF8(const std::string& str)
{
	int len = MultiByteToWideChar(1251, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len];
	MultiByteToWideChar(1251, 0, str.c_str(), -1, wstr, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, 0, 0);
	char* utf8 = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, 0, 0);

	std::string result(utf8);
	delete[] wstr;
	delete[] utf8;

	return result;
}

std::string ConvertUTF8ToCP1251(const std::string& str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, len);

	len = WideCharToMultiByte(1251, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* cp1251 = new char[len];
	WideCharToMultiByte(1251, 0, wstr, -1, cp1251, len, NULL, NULL);

	std::string result(cp1251);
	delete[] wstr;
	delete[] cp1251;

	return result;
}

class TrackAddedEvent : public Event<const Track&> {
public:
	TrackAddedEvent() {}
};

class TrackEditedEvent : public Event<int, const Track&> {
public:
	TrackEditedEvent() {}
};

class ConnectionEvent : public Event<const Connection&> {
public:
	ConnectionEvent() {}
};

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

class UiLayer final : public ILayer {
private:
	TrackAddedEvent trackAdded;
	TrackEditedEvent trackEdited;
	ConnectionEvent clientConnects;
	Session& session;
	Track track;
	Connection connection;
	int selectedIndex;

	static void dropCallback(GLFWwindow* window, int pathCount, const char* paths[]) {
		TrackAddedEvent* event = (TrackAddedEvent*)glfwGetWindowUserPointer(window);

		for (int i = 0; i < pathCount; i++) {
			Track track;
			track.path = ConvertUTF8ToCP1251(paths[i]);
			event->fire(track);
		}
	}

public:
	UiLayer(Session& _session, const TrackAddedEvent& added, const ConnectionEvent& connects)
		: session(_session), selectedIndex(-1), trackAdded(added), clientConnects(connects) {

		GLFWwindow* window = Application::current()->getWindow();
		glfwSetWindowUserPointer(window, &trackAdded);
		glfwSetDropCallback(window, dropCallback);

		trackEdited.onEvent([this](int seletedIndex, const Track& track) {
			session.editTrack(seletedIndex, track);
		});

		connection.ip = "26.70.26.159";
		connection.port = 8000;
		connection.mount = "/test.mp3";
		connection.username = "source";
		connection.password = "123456";
	}

	TrackAddedEvent& getTrackAddedEvent() { return trackAdded; }

	void setCurrentTrack(Track& track) {
		this->track = track;
	}

	void showTrackForm(Track& track) {
		ImGui::InputText("Path", &track.path);
		ImGui::InputText("Name", &track.name);
		ImGui::InputText("Author", &track.author);
		ImGui::Separator();
	}

	void showConnectionForm(Connection& connection) {
		ImGui::Text("hostname");
		ImGui::InputText("##hostname", &connection.ip);
		
		ImGui::Text("port");
		ImGui::InputInt("##port", &connection.port);
		
		ImGui::Text("mount");
		ImGui::InputText("##mount", &connection.mount);
		
		ImGui::Text("username");
		ImGui::InputText("##username", &connection.username);
		
		ImGui::Text("password");
		ImGui::InputText("##password", &connection.password, ImGuiInputTextFlags_Password);
		ImGui::Separator();
	}

	void processAdditionModal() {
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Add track", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			showTrackForm(track);

			if (ImGui::Button("OK")) {
				trackAdded.fire(track);
				track.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	void processEditModal() {
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Edit track", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			showTrackForm(track);

			if (ImGui::Button("OK")) {
				trackEdited.fire(selectedIndex, track);
				track.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				track.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void onUpdate() override {
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(io.DisplaySize);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_MenuBar;

		ImGui::Begin("Main panel", nullptr, flags);

		if (ImGui::BeginTable("MainLayout", 2,
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_NoSavedSettings)) {

			ImGui::TableSetupColumn("Playlist", ImGuiTableColumnFlags_WidthStretch, 0.6f);
			ImGui::TableSetupColumn("Connection", ImGuiTableColumnFlags_WidthStretch, 0.4f);
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Playlist");

			ImVec2 avail = ImGui::GetContentRegionAvail();
			float scrollHeight = avail.y - 30.0f;

			ImGui::BeginChild("ScrollArea", ImVec2(avail.x, scrollHeight), true);

			session.eachTrack([this](int index, Track& track) {
				ImGui::PushID(index);

				bool isSelected = (index == selectedIndex);
				if (!isSelected && index == session.getCurrentTrackIndex()) {
					ImGui::Selectable(track.path.c_str(), false, ImGuiSelectableFlags_Highlight);
				}
				else if (ImGui::Selectable(track.path.c_str(), isSelected)) {
					selectedIndex = index;
				}

				ImGui::PopID();
			});

			ImGui::EndChild();

			if (ImGui::Button("Add")) {
				ImGui::OpenPopup("Add track");
			}

			ImGui::SameLine();
			if (ImGui::Button("Edit") && selectedIndex != -1) {
				track = session.getTrack(selectedIndex);
				ImGui::OpenPopup("Edit track");
			}

			//ImGui::SameLine();
			//ImGui::Button("Delete");

			processAdditionModal();
			processEditModal();

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Connection");

			ImGui::BeginChild("ConnectionArea", ImVec2(avail.x, scrollHeight), true);
			showConnectionForm(connection);
			if (ImGui::Button("Connect")) {
				clientConnects.fire(connection);
			}

			ImGui::Separator();
			ImGui::Text("Connection status: %s", connectionStatusString(session.getConnectionStatus()));
			ImGui::Text("Current track: %d", session.getCurrentTrackIndex());

			ImGui::EndChild();
		}

		ImGui::EndTable();
		ImGui::End();
	}

	void onAttach() override {
		std::println("UI attached");
	}

	void onDetach() override {
		std::println("UI detached");
	}
};

class Growl : public Application {
private:
	UiLayer* gui;
	Session session;

public:
	Growl() : Application("Growl", 854, 480) {
		TrackAddedEvent trackAdded;

		trackAdded.onEvent([this](const Track& track) {
			std::println("Adding {}", track.toString());
			session.addTrack(track);
		});

		ConnectionEvent clientConnects;

		clientConnects.onEvent([this](const Connection& connection) {
			std::println("Connecting...");
			session.connect(connection);
		});

		gui = layers().attach<UiLayer>(session, trackAdded, clientConnects);
	}

	void onUpdate() override {
		if (session.isListEmpty()) return;

		ConnectionStatus status = session.getConnectionStatus();
		if (status != ConnectionStatus::STREAMING && status == ConnectionStatus::CONNECTED) {
			session.playCurrent();
			session.nextTrack();
		}
	}
};

int main() {
#ifdef _WIN32
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
#else
	setlocale(LC_ALL, "ru_RU.UTF-8");
#endif

	Application* growlApp = new Growl();
	growlApp->run(64);
	delete growlApp;
	return 0;
}
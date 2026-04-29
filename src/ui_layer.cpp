#include <GLFW/glfw3.h>
#include <print>

#include "ui_layer.h"
#include "application.h"
#include "growl.h"

#define GR_PLAYLIST_EXTENSION		".gtl"

UiLayer::UiLayer(Session& _session, const TrackAddedEvent& added, const ConnectionEvent& connects)
	: events(((Growl*)Application::current())->getEventManager()),
	session(_session), selectedIndex(-1), shufflePlaylist(false) {

	TrackAddedEvent trackAdded;
	TrackEditedEvent trackEdited;
	ConnectionEvent clientConnects;
	PlaylistManageEvent playlistSelected, playlistSaved;

	GLFWwindow* window = Application::current()->getWindow();
	//glfwSetWindowUserPointer(window, &trackAdded);
	glfwSetDropCallback(window, dropCallback);

	trackEdited.onEvent([this](int seletedIndex, const Track& track) {
		session.editTrack(seletedIndex, track);
	});

	playlistSelected.onEvent([this](void* _browser) {
		auto* browser = (ImGui::FileBrowser*)_browser;
		currentPlaylistPath = browser->GetSelected().string();
		bool loaded = session.loadPlaylistFromFile(browser->GetSelected().string());
		if (!loaded) ImGui::OpenPopup("Error");
	});

	playlistSaved.onEvent([this](void* _browser) {
		auto* browser = (ImGui::FileBrowser*)_browser;

		if (browser->HasSelected()) {
			std::println("Current playlist path: {}", currentPlaylistPath);
			currentPlaylistPath = browser->GetSelected().string();
			if (!hasExtension(currentPlaylistPath, GR_PLAYLIST_EXTENSION)) {
				currentPlaylistPath += GR_PLAYLIST_EXTENSION;
			}
		}

		session.savePlaylist(currentPlaylistPath);
	});

	events.add<"EVT_TRACK_EDITED">(trackEdited);
	events.add<"EVT_PLAYLIST_SELECTED">(playlistSelected);
	events.add<"EVT_PLAYLIST_SAVED">(playlistSaved);

	connection.ip = "26.70.26.159";
	connection.port = 8000;
	connection.mount = "/test.mp3";
	connection.username = "source";
	connection.password = "123456";

	loadFont("fonts/terminus.ttf");
	//ImGui::GetIO().Fonts->Build();

	loadBrowser.SetTitle("Load playlist");
	loadBrowser.SetTypeFilters({ GR_PLAYLIST_EXTENSION });

	saveBrowser = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename);
	saveBrowser.SetTitle("Save playlist");
	saveBrowser.SetTypeFilters({ GR_PLAYLIST_EXTENSION });
}

void UiLayer::callPlaylistSave() {
	if (currentPlaylistPath.empty()) saveBrowser.Open();
	else events.fire<PlaylistManageEvent, "EVT_PLAYLIST_SAVED">((void*)&saveBrowser);
}

void UiLayer::callPlaylistSaveAs() {
	saveBrowser.Open();
}

void UiLayer::callPlaylistOpen() {
	loadBrowser.Open();
}

void UiLayer::dropCallback(GLFWwindow* window, int pathCount, const char* paths[]) {
	EventManager& events = ((Growl*)Application::current())->getEventManager();
	auto* event = events.get<TrackAddedEvent, "EVT_TRACK_ADDED">();

	for (int i = 0; i < pathCount; i++) {
		Track track(paths[i]);
		event->fire(track);
	}
}

bool UiLayer::hasExtension(const std::string& path, const std::string_view& extension) {
	if (extension.size() > path.size()) return false;
	int i = extension.size() - 1, j = path.size() - 1;

	for (; i >= 0 && j >= 0; i--, j--) {
		if (std::tolower(path[j]) != std::tolower(extension[i])) return false;
	}

	return true;
}

void UiLayer::loadFont(const std::string& filename) {
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig config;
	config.OversampleH = 1;
	config.OversampleV = 1;
	config.PixelSnapH = true;

	ImFont* font = io.Fonts->AddFontFromFileTTF(
		filename.c_str(),
		14.0f,
		&config,
		io.Fonts->GetGlyphRangesCyrillic()
	);

	if (font) {
		std::println("Loaded font: {}", filename);
	}
	else {
		std::println(stderr, "Can't load font: {}", filename);
	}
}

void UiLayer::processErrorModal() {
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Can't load playlist!");

		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void UiLayer::showMenuBar() {
	bool addTrackOpened = false,
		editTrackOpened = false,
		currentPlaylistSaved = false,
		fileBrowserOpenedToLoad = false,
		fileBrowserOpenedToSave = false;

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open playlist", "Ctrl+O")) fileBrowserOpenedToLoad = true;
			if (ImGui::MenuItem("Save playlist", "Ctrl+S")) currentPlaylistSaved = true;
			if (ImGui::MenuItem("Save playlist as", "Ctrl+Shift+S")) fileBrowserOpenedToSave = true;
			ImGui::Separator();
			if (ImGui::MenuItem("Add track", "Ctrl+N")) addTrackOpened = true;
			if (ImGui::MenuItem("Edit track", "Ctrl+E", false, selectedIndex != -1)) {
				editTrackOpened = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (addTrackOpened || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_N)) {
		ImGui::OpenPopup("Add track");
	}
	if (editTrackOpened ||
		ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_E) &&
		selectedIndex != -1) {
		track = session.getTrack(selectedIndex);
		ImGui::OpenPopup("Edit track");
	}

	if (currentPlaylistSaved || 
		ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S)) callPlaylistSave();
	if (fileBrowserOpenedToLoad ||
		ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O)) callPlaylistOpen();
	if (fileBrowserOpenedToSave ||
		ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S)) callPlaylistSaveAs();

	loadBrowser.Display();
	saveBrowser.Display();

	if (loadBrowser.HasSelected()) {
		events.fire<PlaylistManageEvent, "EVT_PLAYLIST_SELECTED">((void*)&loadBrowser);
		loadBrowser.ClearSelected();
	}
	if (saveBrowser.HasSelected()) {
		events.fire<PlaylistManageEvent, "EVT_PLAYLIST_SAVED">((void*)&saveBrowser);
		saveBrowser.ClearSelected();
	}
}

void UiLayer::showTrackForm(Track& track) {
	ImGui::InputText("Path", &track.path);
	ImGui::InputText("Name", &track.name);
	ImGui::InputText("Author", &track.author);
	ImGui::Separator();
}

void UiLayer::showConnectionForm(Connection& connection) {
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

void UiLayer::processAdditionModal() {
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Add track", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		showTrackForm(track);

		if (ImGui::Button("OK")) {
			events.fire<TrackAddedEvent, "EVT_TRACK_ADDED">(track);
			track.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void UiLayer::processEditModal() {
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Edit track", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		showTrackForm(track);

		if (ImGui::Button("OK")) {
			events.fire<TrackEditedEvent, "EVT_TRACK_EDITED">(selectedIndex, track);
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

void UiLayer::onUpdate() {
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

		session.eachShuffledTrack([this](int index, Track& track) {
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

		ImGui::TableSetColumnIndex(1);
		ImGui::Text("Connection");

		ImGui::BeginChild("ConnectionArea", ImVec2(0, scrollHeight), true);

		avail = ImGui::GetContentRegionAvail();
		ImGui::PushItemWidth(avail.x);
		showConnectionForm(connection);
		ImGui::PopItemWidth();

		if (ImGui::Button("Connect")) {
			events.fire<ConnectionEvent, "EVT_CONNECTION">(connection);
		}

		ImGui::Separator();
		ImGui::Text("Connection status: %s", connectionStatusString(session.getConnectionStatus()));
		ImGui::Text("Current track: %d", session.getCurrentTrackIndex());

		ImGui::EndChild();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		static bool shufflingEnabled = session.isRandomPlaybackEnabled();
		ImGui::Checkbox("Reshuffling", &shufflePlaylist);
		if (shufflingEnabled != shufflePlaylist) {
			events.fire<ShufflingSelectionEvent, "EVT_SHUFFLING_SELECTION">(shufflePlaylist);
			shufflingEnabled = shufflePlaylist;
		}
	}

	showMenuBar();
	processErrorModal();
	processAdditionModal();
	processEditModal();

	ImGui::EndTable();
	ImGui::End();
}

void UiLayer::onAttach() {
	std::println("UI attached");
}

void UiLayer::onDetach() {
	std::println("UI detached");
}

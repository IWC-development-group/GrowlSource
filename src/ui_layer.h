#pragma once
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imfilebrowser.h>
#include <string>

#include "custom_events.h"
#include "session.h"
#include "instances.h"
#include "layer.h"

struct GLFWwindow;

class UiLayer final : public ILayer {
private:
	TrackAddedEvent trackAdded;
	TrackEditedEvent trackEdited;
	ConnectionEvent clientConnects;
	PlaylistManageEvent playlistSelected, playlistSaved;
	Session& session;
	Track track;
	Connection connection;
	std::string currentPlaylistPath;
	ImGui::FileBrowser loadBrowser, saveBrowser;
	int selectedIndex;

	static void dropCallback(GLFWwindow* window, int pathCount, const char* paths[]);
	static void hotkeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void loadFont(const std::string& filename);

public:
	UiLayer(Session& _session, const TrackAddedEvent& added, const ConnectionEvent& connects);

	TrackAddedEvent& getTrackAddedEvent() { return trackAdded; }

	void setCurrentTrack(Track& track) { this->track = track; }
	void callPlaylistSave();
	void callPlaylistSaveAs();

	void showMenuBar();
	void showTrackForm(Track& track);
	void showConnectionForm(Connection& connection);

	void processErrorModal();
	void processAdditionModal();
	void processEditModal();

	void onUpdate() override;
	void onAttach() override;
	void onDetach() override;
};

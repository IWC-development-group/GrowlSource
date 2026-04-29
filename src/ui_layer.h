#pragma once
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imfilebrowser.h>
#include <string>

#include "event_manager.h"
#include "custom_events.h"
#include "session.h"
#include "instances.h"
#include "layer.h"

struct GLFWwindow;

class UiLayer final : public ILayer {
private:
	EventManager& events;
	Session& session;
	Track track;
	Connection connection;
	std::string currentPlaylistPath;
	ImGui::FileBrowser loadBrowser, saveBrowser;
	int selectedIndex;
	bool shufflePlaylist;

	static void dropCallback(GLFWwindow* window, int pathCount, const char* paths[]);
	static bool hasExtension(const std::string& path, const std::string_view& extension);
	void loadFont(const std::string& filename);

public:
	UiLayer(Session& _session, const TrackAddedEvent& added, const ConnectionEvent& connects);

	void setCurrentTrack(Track& track) { this->track = track; }
	void callPlaylistSave();
	void callPlaylistSaveAs();
	void callPlaylistOpen();

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

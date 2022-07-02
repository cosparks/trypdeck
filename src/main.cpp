#include <iostream>
#include <thread>

#include "settings.h"
#include "Clock.h"
#include "Serial.h"
#include "VideoPlayer.h"
#include "LedPlayer.h"
#include "TripdeckMediaManager.h"
#include "TripdeckLeader.h"
#include "TripdeckFollower.h"
#include "MockButton.h"

const char* VideoFolders[] = { VIDEO_CONNECTING_DIRECTORY, VIDEO_CONNECTED_DIRECTORY, VIDEO_WAIT_DIRECTORY, VIDEO_PULLED_DIRECTORY, VIDEO_REVEAL_DIRECTORY };
const char* LedFolders[] = { LED_CONNECTING_DIRECTORY, LED_CONNECTED_DIRECTORY, LED_WAIT_DIRECTORY, LED_PULLED_DIRECTORY, LED_REVEAL_DIRECTORY };

InputManager inputManager;
Serial serial("/dev/ttyAMA0", O_RDWR);

int main(int argc, char** argv) {
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	// Test code:
	// TODO: try to repro the weird video bug
	// const char* VLC_ARGS[] = { "-v", "-I", "dummy", "--aout=adummy", "--fullscreen", "--no-osd", "--no-audio", "--vout", "mmal_vout" };

	// libvlc_instance_t* instance = libvlc_new(9, VLC_ARGS);
	// libvlc_media_list_t* mediaList = libvlc_media_list_new(instance);
	// libvlc_media_list_player_t* mediaListPlayer = libvlc_media_list_player_new(instance);
	// libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);
	// libvlc_media_t* media = libvlc_media_new_path(instance, "/home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/complex-color-test-fast.mp4");
	
	// libvlc_media_list_lock(mediaList);
	// int32_t ret1 = libvlc_media_list_insert_media(mediaList, media, 0);
	// std::cout << "media list count 1: " << libvlc_media_list_count(mediaList) << std::endl;
	// libvlc_media_list_unlock(mediaList);

	// bool isReadOnly = libvlc_media_list_is_readonly(mediaList);

	// libvlc_media_release(media);

	// libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_default);
	// libvlc_media_list_player_play_item_at_index(mediaListPlayer, 0);

	// while (Clock::instance().millis() < 5000) {
	// 	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	// }

	// libvlc_media_list_player_stop(mediaListPlayer);

	// libvlc_media_list_lock(mediaList);
	// libvlc_media_t* mediaToRemove = libvlc_media_list_item_at_index(mediaList, 0);
	// int32_t ret2 = libvlc_media_list_remove_index(mediaList, 0);
	// libvlc_media_release(mediaToRemove);
	// std::cout << "media list count 2: " << libvlc_media_list_count(mediaList) << std::endl;
	// libvlc_media_list_unlock(mediaList);

	// libvlc_media_t* newMedia = libvlc_media_new_path(instance, "/home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/sonic2.mp4");

	// libvlc_media_list_lock(mediaList);
	// int32_t ret3 = libvlc_media_list_insert_media(mediaList, newMedia, 0);
	// std::cout << "media list count 3: " << libvlc_media_list_count(mediaList) << std::endl;
	// libvlc_media_list_unlock(mediaList);

	// libvlc_media_release(newMedia);

	// libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);

	// libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_default);
	// libvlc_media_list_player_play_item_at_index(mediaListPlayer, 0);

	// while (Clock::instance().millis() < 5000) {
	// 	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	// }
	
	// libvlc_media_list_player_stop(mediaListPlayer);

	// libvlc_media_list_player_release(mediaListPlayer);
	// libvlc_media_list_release(mediaList);
	// libvlc_release(instance);

	// std::cout << "ret1: " << ret1 << " -- ret2: " << ret2 <<  std::endl;
	// std::cout << "Test complete" << std::endl;
	// END: test code

	// std::this_thread::sleep_for(std::chrono::milliseconds(10000));

	// set up media
	DataManager dataManager;
	VideoPlayer videoPlayer;
	#if RUN_LEDS
	#if (LED_SETTING == MAIN_LEDS)
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_SPLIT, GRID_AB_ORIENTATION, LED_GRID_CONFIGURATION_OPTION_A, LED_GRID_CONFIGURATION_OPTION_B);
	#else
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, 0, GRID_AB_ORIENTATION, LED_GRID_CONFIGURATION_OPTION, Apa102::GridConfigurationOption(0));
	#endif
	LedPlayer ledPlayer(&ledController);
	TripdeckMediaManager mediaManager(&dataManager, &videoPlayer, &ledPlayer);
	#else
	TripdeckMediaManager mediaManager(&dataManager, &videoPlayer);
	#endif

	for (int32_t state = TripdeckState::Connecting; state <= TripdeckState::Reveal; state++) {
		mediaManager.addVideoFolder(TripdeckState(state), VideoFolders[state]);

		#if RUN_LEDS
		mediaManager.addLedFolder(TripdeckState(state), LedFolders[state]);
		#endif
	}

	// set up application
	#ifdef Leader
	TripdeckLeader tripdeck(&mediaManager, &inputManager, &serial);
	#else
	TripdeckFollower tripdeck(&mediaManager, &inputManager, &serial);
	#endif

	// initialize and run application
	tripdeck.init();
	tripdeck.run();

	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}
// #include <iostream>
// #include <thread>
// #include <stdint.h>
// #include <vlc/vlc.h>

// #include "Clock.h"

// // Test code:

// int main(int argc, char** argv) {
// 	// TODO: try to repro the weird video bug
// 	const char* VLC_ARGS[] = { "-v", "-I", "dummy", "--aout=adummy", "--fullscreen", "--no-osd", "--no-audio", "--vout", "mmal_vout" };

// 	libvlc_instance_t* instance = libvlc_new(9, VLC_ARGS);
// 	libvlc_media_list_t* mediaList = libvlc_media_list_new(instance);
// 	libvlc_media_list_player_t* mediaListPlayer = libvlc_media_list_player_new(instance);
// 	libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);
// 	libvlc_media_t* media = libvlc_media_new_path(instance, "/home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/complex-color-test-fast.mp4");
	
// 	libvlc_media_list_lock(mediaList);
// 	int32_t ret1 = libvlc_media_list_insert_media(mediaList, media, 0);
// 	std::cout << "media list count 1: " << libvlc_media_list_count(mediaList) << std::endl;
// 	libvlc_media_list_unlock(mediaList);

// 	bool isReadOnly = libvlc_media_list_is_readonly(mediaList);

// 	libvlc_media_release(media);

// 	libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_default);
// 	libvlc_media_list_player_play_item_at_index(mediaListPlayer, 0);

// 	while (Clock::instance().millis() < 5000) {
// 		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 	}

// 	libvlc_media_list_player_stop(mediaListPlayer);

// 	libvlc_media_list_lock(mediaList);
// 	libvlc_media_t* mediaToRemove = libvlc_media_list_item_at_index(mediaList, 0);
// 	int32_t ret2 = libvlc_media_list_remove_index(mediaList, 0);
// 	libvlc_media_release(mediaToRemove);
// 	std::cout << "media list count 2: " << libvlc_media_list_count(mediaList) << std::endl;
// 	libvlc_media_list_unlock(mediaList);

// 	libvlc_media_t* newMedia = libvlc_media_new_path(instance, "/home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/sonic2.mp4");

// 	libvlc_media_list_lock(mediaList);
// 	int32_t ret3 = libvlc_media_list_insert_media(mediaList, newMedia, 0);
// 	std::cout << "media list count 3: " << libvlc_media_list_count(mediaList) << std::endl;
// 	libvlc_media_list_unlock(mediaList);

// 	libvlc_media_release(newMedia);

// 	libvlc_media_list_player_set_media_list(mediaListPlayer, mediaList);

// 	libvlc_media_list_player_set_playback_mode(mediaListPlayer, libvlc_playback_mode_default);
// 	libvlc_media_list_player_play_item_at_index(mediaListPlayer, 0);

// 	while (Clock::instance().millis() < 5000) {
// 		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 	}
	
// 	libvlc_media_list_player_stop(mediaListPlayer);

// 	libvlc_media_list_player_release(mediaListPlayer);
// 	libvlc_media_list_release(mediaList);
// 	libvlc_release(instance);

// 	std::cout << "ret1: " << ret1 << " -- ret2: " << ret2 <<  std::endl;
// 	std::cout << "Test complete" << std::endl;

// 	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
// 	return 1;
// }
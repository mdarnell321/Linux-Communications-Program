#ifndef USER_H
#define USER_H

#include "Packet.h"
#include <future>
#include <deque>
#include <AL/al.h>
#include <AL/alc.h>
#include "wxcrafter.h"
#include <wx/statbmp.h>

class User;

typedef struct {
	User* Host;
	std::string filename;
	unsigned long long total_bytes;
	unsigned long long bytes_processed;
	std::vector<char> buff;
	bool pending;
	bool responded;
	bool cant_continue;
} FileStream;

class User
{
public:
	bool destruction = false;
	std::string name = "";
	bool active = false;
	int main_tcp_socket = -1;
	bool voice_active = false, video_active = false;
	bool video_was_active = false;
	//VOICE
	bool passed_listen_init = false;
	ALuint cur_source;
	ALuint *openal_buffer = nullptr; //buffer OpenAL uses. Uses voice_buffer for the data.
	std::deque<audio_buffer_data> voice_buffer; // data sent as bytes from server
	std::thread* voice_thread= nullptr;
	ALCdevice* input_device = nullptr;
	std::deque<std::packaged_task<void()>> ActionsOnUserVoiceThread;
	std::mutex ActionsOnUserVoiceThread_mutex;
	void TaskTo_UserVoiceThread(std::packaged_task<void()>);
	void Voice();
	void StartVoice();
	void StopVoice();
	void ClearVoiceBuff();
	//VIDEO
	std::string cam_path = "";
	wxStaticBitmap* vid = nullptr; //video UI element
	unsigned int video_width = 0,video_height = 0;
	std::thread* video_thread= nullptr;
	std::deque<std::packaged_task<void()>> ActionsOnUserVideoThread;
	std::mutex ActionsOnUserVideoThread_mutex;
	std::vector<fragmented_frame> video_buffer;
	void StartVideo();
	void StopVideo();
	void Video();
	void TaskTo_UserVideoThread(std::packaged_task<void()>);
	void ClearVideoBuff();
	int VideoTimeStamp = 0 /* this is the timestamp for each frame*/;
	float DifferenceAverage = 0.0;
	int vid_send_timer = 0;
	int frames_passed = 0, frames_lost = 0, lost_frames_threshold_reached = 0;
	float percent_frames_lost = 0;
	std::vector<float> VideoandAudioDifferences;
	//FILE
	std::thread* handle_file_thread= nullptr;
	void Handle_File(FileStream*);
	//UDP
	bool jobs_running = false;
	void UDPJobs();
	void TaskTo_UserUDPThread(std::packaged_task<void()>);
	std::deque<std::packaged_task<void()>> ActionsOnUserUDPThread;
	std::mutex ActionsOnUserUDPThread_mutex;
	std::thread* udp_jobs_thread= nullptr;
	std::thread* termination_thread = nullptr;
	User(int, std::string, bool,bool);
	~User();
};
#endif

/*
MIT License

Copyright (c) 2023 mdarnell321

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
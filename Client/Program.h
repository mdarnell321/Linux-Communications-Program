#ifndef PROGRAM_H
#define PROGRAM_H

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <future>
#include <deque>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include "wxcrafter.h"
#include <wx/statbmp.h>

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Form.h"
#include "Packet.h"
#include "V4L2Manager.h"
#include "User.h"


typedef struct {
	wxStaticBitmap* source_ptr;
	int width;
	int height;
} VidElements;


class Program
{
public:
	static bool running;
	static std::string HostAddress;
	static std::string Username;
	static int network_socket, room_socket, room_udp_socket, mysocket;

	static struct sockaddr_in room_address;
	static std::thread* UDPThread,*TCPThread, *Disconnect_Thread;
	static int ms_Elapsed;
	static int Ping_Timer;
	static std::vector<VidElements> Screen_Video_Elements;
	static ALCdevice* output_device;
	static ALCcontext* audio_context;
	static int fd;
	static bool CanContinueFile;
	//Network Objs
	static std::vector<User*> User_List;
	static std::vector<FileStream*> File_Transfers;
	static User* me;
	static User* AsUser(int);
	static FileStream* MyFile;
	static void TaskTo_NetworkObjectThread(std::packaged_task<void()>);
	static void StartThreads();
	static void StartRoomThreads();
private:
	static std::deque<std::packaged_task<void()>> ActionsOnNetObjThread;
	static std::mutex netobj_mutex;
	static void NetObjActions_Update();
	static void One_Second_Update();
	static void Listen_MainTCP();
	static void Listen_RoomTCP();
	static void Listen_RoomUDP();
	static void UpdateMSElapsed();
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

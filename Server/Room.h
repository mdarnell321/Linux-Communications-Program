#ifndef ROOM_H
#define ROOM_H

#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <string>
#include <future>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>

#include "User.h"
#include "MemoryManagement.h"

typedef struct {
	struct sockaddr_in ep;
	std::vector<unsigned char> data;
	std::chrono::system_clock::time_point timestamp;
	int public_id = -1;
} ENDPOINTS;

class Room
{
public:
	std::string name = "";
	bool alive = false;
	int tcp_socket = -1, udp_socket = -1;
	int port = 0;
	std::vector<FileStream*> File_Transfers;
	std::vector<RoomUser*> User_List;
	std::vector<UnassignedUser*> UnassignedUser_List;
	std::thread* EndThread = nullptr;
	std::vector<DeletionObject> ObjsToDelete_A;
	std::thread* tcp_listen_thread = nullptr;
	void TCP_Listen();

	std::vector<ENDPOINTS> endpoints;
	ENDPOINTS* FindUDPEndPoint(struct sockaddr_in);
	std::thread* UDP_task_thread = nullptr, *udp_listen_thread = nullptr;
	void UDP_Listen();
	void TaskTo_UDPThread(std::packaged_task<void()>);
	void ActionsOnUDPThread_Update();
	std::deque<std::packaged_task<void()>> ActionsOnUserUDPThread;
	std::mutex ActionsOnUserUDPThread_mutex;

	std::thread* File_task_thread = nullptr;
	void TaskTo_FileThread(std::packaged_task<void()>);
	void File_Tasks_Update();
	std::deque<std::packaged_task<void()>> file_tasks;
	std::mutex file_tasks_mutex;

	std::vector<void*> Instantiated_Objs;
	Room(int);
	~Room();
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
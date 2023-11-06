#ifndef USER_H
#define USER_H

#include <iostream>
#include <thread>
#include <vector>

class User;
class RoomUser;
class UnassignedUser;

typedef struct {

	RoomUser* Host;
	std::string filename;
	unsigned long long total_bytes;
	int secondsleft;
	unsigned long long processed_bytes;
	std::vector<RoomUser*> Recipients;
	int count_of_tens = 0;
} FileStream;


class User
{
public:
	bool active_on_serv = false;
	int Ping_Timer = 0;
	User(std::string, int);
	~User();
	std::string name = "";
	int tcp_socket = -1;
	std::thread* tcp_listen_thread = nullptr;
	void TCP_Listen();
};
class RoomUser : public User
{
public:
	int room_tcp_socket = -1;
	bool voice_active = false, video_active = false;
	int w = 0, h = 0;
	bool active = false;
	std::thread* room_tcp_listen_thread;
	void Room_TCP_Listen();
	RoomUser(std::string, int);
	~RoomUser();
	std::thread* EndThread = nullptr;
};
class UnassignedUser
{
public:
	int socket = -1;
	bool active = false;
	std::thread* tcp_listen_thread = nullptr;
	void TCP_Listen();
	UnassignedUser(int);
	~UnassignedUser();
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
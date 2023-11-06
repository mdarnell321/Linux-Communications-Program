#include "Program.h"
#include "SendPackets.h"
#include "ReceivePackets.h"

bool Program::running = false;
std::vector<User*> Program::User_List;
int Program::Server_Socket;
std::vector<Room*> Program::Room_List;
std::deque<std::packaged_task<void()>> Program::ActionsOnNetObjThread;
std::mutex Program::netobj_mutex;


void Program::StartThreads()
{
	new std::thread (&NetObjActions_Update);
	new std::thread (&Update);
	new std::thread (&MemoryManagement::Update);
	new std::thread (&TCP_Listen);
}
User* Program::AsUser(int socket)
{
	for(auto &user : User_List) {
		if(user->tcp_socket == socket || ((RoomUser*)user)->room_tcp_socket == socket) {
			return user;
		}
	}
	return nullptr;
}
Room* Program::FindRoomWithSocket(int socket)
{
	for(auto &room : Room_List) {
		for(auto &user : room->User_List) {
			if(user->tcp_socket == socket || user->room_tcp_socket == socket) {
				return room;
			}
		}
		for(auto &undefined_user : room->UnassignedUser_List) {
			if(undefined_user->socket == socket) {
				return room;
			}
		}
	}
	return nullptr;
}
Room* Program::AsRoom(int socket)
{
	for(auto &room : Room_List) {
		if(room->tcp_socket == socket) {
			return room;
		}
	}
	return nullptr;
}

void Program::TaskTo_NetworkObjectThread(std::packaged_task<void()> a) // this is used if your trying to access a network obj (such as a user).
{
	std::lock_guard<std::mutex> lock(netobj_mutex);
	ActionsOnNetObjThread.push_back(std::move(a));
}

void Program::NetObjActions_Update()
{
	while (running == true) {
		std::vector<std::packaged_task<void()>> temp;
		std::unique_lock<std::mutex> lock(netobj_mutex);
		while(ActionsOnNetObjThread.empty() == false) {
			temp.push_back(std::move(ActionsOnNetObjThread.front()));
			ActionsOnNetObjThread.pop_front();
		}
		lock.unlock();
		for(int i =0 ; i < temp.size(); ++i) {
			(temp[i])();
		}
	}
}
void Program::TCP_Listen()
{
	while(running == true) {
		struct sockaddr_in client_address;
		socklen_t address_length;
		int client_socket;
		address_length = sizeof(struct sockaddr_in);
		client_socket = accept(Server_Socket, (struct sockaddr *)&client_address, &address_length);
		if(client_socket == -1) {
			continue;
		} else {
			TaskTo_NetworkObjectThread( std::packaged_task<void()> { [client_socket]()
			{
				RoomUser* new_u = new RoomUser("Undefined", client_socket);
				MemoryManagement::Add_Obj(new_u);
				User_List.push_back(std::move((User*)new_u));
				SendPackets::RequestNameFromUser(client_socket);
			}
			                                                                });
		}
	}
}


void Program::Update()
{
	while (running == true) {

		TaskTo_NetworkObjectThread( std::packaged_task<void()> { []()
		{
			for(unsigned int i = 0; i < User_List.size(); ++i) {
				RoomUser* u = (RoomUser*)User_List[i];
				if(u->tcp_socket == Server_Socket)
					continue;
				u->Ping_Timer ++;
				if(u->Ping_Timer > 6) {
					//lost connection, handle disconnnect
					if(u->tcp_socket != -1) {
						shutdown(u->tcp_socket, SHUT_RDWR);
						close(u->tcp_socket);
					}
					if(u->room_tcp_socket != -1) {
						shutdown(u->room_tcp_socket, SHUT_RDWR);
						close(u->room_tcp_socket);
					}
				}
			}
			for(unsigned int i = 0; i < Room_List.size(); ++i) {

				for(unsigned int ii = 0; ii < Room_List[i]->File_Transfers.size(); ++ii) {
					FileStream *f = Room_List[i]->File_Transfers[ii];
					if(f->secondsleft > 0) {
						f->secondsleft --;
						if(f->secondsleft <= 0) {
							//decision time
							if(f->Recipients.size() <= 1) {
								Room_List[i]->File_Transfers.erase(Room_List[i]->File_Transfers.begin()+ii--);
								if(f->Host != nullptr)
									SendPackets::SendServerMessage(f->Host->tcp_socket, "Server : Unable to stream file due to no receivers.");
								SendPackets::StartFileProcess(false,f);

								Room_List[i]->ObjsToDelete_A.push_back(std::move(DeletionObject{f,0,0,0}));
								break;
							} else {
								SendPackets::StartFileProcess(true,f);
							}
						}
					}
				}
			}
		}
		                                                                });
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

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

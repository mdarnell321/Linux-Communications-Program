#include "ReceivePackets.h"


void ReceivePackets::ReceiveUserName(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	std::string username = packet->GetString(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(sock_id);
	if(u != nullptr) {
		u->name = username;
		std::cout << "Socket " << sock_id << " is named " << username << ".\n";
		SendPackets::SendUserTheirSocket(sock_id);
	}
}
void ReceivePackets::CreateRoom(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	std::string roomname = packet->GetString(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(sock_id);
	if(u != nullptr) {
		Room* room = nullptr;
		for(int i =0; i < (int)Program::Room_List.size(); ++i) {
			if(roomname == Program::Room_List[i]->name) {
				if(Program::Room_List[i]->User_List.size() >= 4) {
					SendPackets::SendServerMessage(sock_id, "Server : This room already has 4 users in it.");
					SendPackets::SendServerResponse(sock_id, 0);
					return;
				}
				room = Program::Room_List[i];
			}
		}
		if(room == nullptr) {
			room = new Room(Program::Room_List.size() > 0 ? Program::Room_List.back()->port + 1 : 5001);
			MemoryManagement::Add_Obj(room);
			room->name = roomname;
			Program::Room_List.push_back(std::move(room));
		}
		SendPackets::SendServerResponse(sock_id, 0);
		SendPackets::SendRoomCreationSuccess(sock_id, room->port, room->tcp_socket,room->udp_socket);
	}
}
void ReceivePackets::ReceiveRoomMessage(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	std::string msg = packet->GetString(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(sock_id);
	if(u != nullptr) {
		std::string new_msg = u->name + ": " + msg;
		Room* r = Program::FindRoomWithSocket(sock_id);
		if(r != nullptr)
			SendPackets::SendMessageToRoom(r->tcp_socket, new_msg);
	}
}
void ReceivePackets::Ping(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(sock_id);
	if(u != nullptr) {
		u->Ping_Timer = 0;
		SendPackets::Pong(sock_id);
	}
}
void ReceivePackets::OpenUserVoice(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	bool open = packet->GetBool(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	RoomUser* u = (RoomUser*)Program::AsUser(sock_id);
	u->voice_active = open;
	Room* r = Program::FindRoomWithSocket(sock_id);
	if(r != nullptr)
		SendPackets::SendUserVoiceStatus(u, r);
}
void ReceivePackets::ClientRequestLeaveRoom(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	if(packet->CheckFinalizers(error) == false) return;
	RoomUser* u = (RoomUser*)Program::AsUser(sock_id);
	Room* r = Program::FindRoomWithSocket(sock_id);
	if(r != nullptr && u != nullptr) {
		u->active = false;
		if(u->room_tcp_socket != -1) {
			shutdown(u->room_tcp_socket,SHUT_RDWR);
			close(u->room_tcp_socket);
		}
		SendPackets::LeaveRoomSuccess(sock_id);
	}

}
void ReceivePackets::OpenUserVideo(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{

	bool open = packet->GetBool(error);
	if(*error == true)return;
	int a = packet->GetInt(error);
	if(*error == true)return;
	int b = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	RoomUser* u = (RoomUser*)Program::AsUser(sock_id);
	if(u == nullptr)
		return;
	u->video_active = open;
	if(open == true) {
		u->w = a;
		u->h = b;
	} else {
		u->w = 0;
		u->h = 0;
	}
	Room* r = Program::FindRoomWithSocket(sock_id);
	if(r != nullptr)
		SendPackets::SendUserVideoStatus(u, r,a,b);
}
void ReceivePackets::TerminateFileStream(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	RoomUser* u = (RoomUser*)Program::AsUser(sock_id);
	Room* r = Program::FindRoomWithSocket(sock_id);
	if(r != nullptr && u != nullptr) {
		for(unsigned int i =0; i < r->File_Transfers.size(); ++i) {
			if(r->File_Transfers[i]->Host == u) {
				FileStream* f = r->File_Transfers[i];
				r->File_Transfers.erase(r->File_Transfers.begin()+i);
				if(f->Host != nullptr) {
					for(unsigned int ii = 0; ii < f->Recipients.size(); ++ii) {
						SendPackets::SendServerMessage(f->Recipients[ii]->tcp_socket, "Server : File could not be received. The host doesnt have the ability to read the file.");
					}
				}
				SendPackets::StartFileProcess(false,f);
				r->ObjsToDelete_A.push_back(std::move(DeletionObject{f,0,0,0}));
			}
		}
	}
}
void ReceivePackets::InitiateFileStream(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	unsigned long long byte_count = packet->GetULL(error);
	if(*error == true)return;
	std::string filename = packet->GetString(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	RoomUser* u = (RoomUser*)Program::AsUser(sock_id);
	Room* r = Program::FindRoomWithSocket(sock_id);
	if(u != nullptr && r != nullptr) {
		
		FileStream *f = new FileStream{u, filename,byte_count, 10, 0, std::vector<RoomUser*>{}, 0};
		MemoryManagement::Add_Obj(f);
		f->Recipients.push_back(std::move(u));// Make host a recipient
		r->File_Transfers.push_back(std::move(f));
		SendPackets::NotifyRoomUsersOfFile(f,r);
	}
}
void ReceivePackets::HandleFileSection(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	int owner = packet->GetInt(error);
	if(*error == true)return;
	boost_sharedptr_bytes section = packet->GetBytes(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	RoomUser* u = (RoomUser*)Program::AsUser(owner);
	Room* r = Program::FindRoomWithSocket(owner);

	if(u != nullptr && r != nullptr) {
		for(auto &obj : r->File_Transfers) {
			FileStream * f = obj;
			if(f->Host == u) {
				SendPackets::SendFileSection(f, section);
				f->processed_bytes += section.length;
				if(f->processed_bytes >= f->total_bytes) {
					Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [r,owner,f]()
					{
						Room * _r = Program::FindRoomWithSocket(owner);
						if(_r != nullptr) {
							for(unsigned int i = 0; i < _r->File_Transfers.size(); ++i) {
								if(_r->File_Transfers[i] == f)
									r->File_Transfers.erase(r->File_Transfers.begin()+i--);
							}
							_r->ObjsToDelete_A.push_back(std::move(DeletionObject{ f,0,0,0 }));
						}
					}
					                                                                 });
				}
			}
		}
	}
}
void ReceivePackets::UserFileDecision(boost::shared_ptr<Packet> packet, int sock_id, bool *error)
{
	bool decision = packet->GetBool(error);
	if(*error == true)return;
	int host = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	RoomUser* u = (RoomUser*)Program::AsUser(sock_id);
	Room* r = Program::FindRoomWithSocket(sock_id);
	if(u != nullptr && r != nullptr) {
		for(unsigned int i =0; i < r->File_Transfers.size(); ++i) {
			if(r->File_Transfers[i]->Host->tcp_socket == host) {
				bool found = false;
				for(unsigned int ii = 0; ii < r->File_Transfers[i]->Recipients.size(); ++ii) {
					if(r->File_Transfers[i]->Recipients[ii] == u)
						found = true;
				}
				if(found == false && decision == true) {
					SendPackets::SendServerMessage(r->File_Transfers[i]->Host->tcp_socket, ("Server : " + u->name + " has accepted to receive your file stream."));
					r->File_Transfers[i]->Recipients.push_back(std::move(u));
				} else if(decision == false) {
					SendPackets::SendServerMessage(r->File_Transfers[i]->Host->tcp_socket, ("Server : " + u->name + " has refused to receive your file stream."));
				}
			}
		}
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
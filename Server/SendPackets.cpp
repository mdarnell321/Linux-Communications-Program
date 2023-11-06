#include "SendPackets.h"


void SendPackets::SendTCPToSocket(Packet* packet, int socket, bool nodelete)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	send(socket, bytearr, length, MSG_NOSIGNAL);
	if(nodelete == false)
		delete(packet);
}
void SendPackets::SendTCPToAll(Packet* packet)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	for(auto &user : Program::User_List) {
		if(user->tcp_socket != Program::Server_Socket) {
			send(user->tcp_socket, bytearr, length, MSG_NOSIGNAL);
		}
	}
	delete(packet);
}
void SendPackets::SendUDPToAllInRoom(Packet* packet, int room)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	Room* r = Program::AsRoom(room);
	if(r == nullptr)
		return;
	for(auto &e : r->endpoints) {
		struct sockaddr_in ep = e.ep;
		if(ep.sin_port != -1)
			sendto(r->udp_socket, bytearr, length, 0,(struct sockaddr*)&(ep), sizeof(ep));
	}
	delete(packet);
}
void SendPackets::SendUDPToAddress(Packet* packet, int host_socket, struct sockaddr_in address)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	if(address.sin_port != -1)
		sendto(host_socket, bytearr, length, 0,(struct sockaddr*)&(address), sizeof(address));
	delete(packet);
}
void SendPackets::SendTCPToAllInRoom(Packet* packet, int room)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	Room* r = Program::AsRoom(room);
	if(r == nullptr)
		return;
	for(auto &user : r->User_List) {
		send(user->tcp_socket, bytearr, length, MSG_NOSIGNAL);
	}
	delete(packet);
}
void SendPackets::SendTCPToAllInRoomExcept(Packet* packet, int room, int _except)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	Room* r = Program::AsRoom(room);
	if(r == nullptr)
		return;
	for(auto &user : r->User_List) {
		if(user->tcp_socket == _except || user->room_tcp_socket == _except)
			continue;
		send(user->tcp_socket, bytearr, length, MSG_NOSIGNAL);
	}
	delete(packet);
}
void SendPackets::RequestNameFromUser(int socket)
{
	Packet* packet = new Packet(1);

	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::SendRoomCreationSuccess(int socket, int port, int roomsock, int udpsock)
{
	Packet* packet = new Packet(2);

	packet->Write(port);
	packet->Write(roomsock);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::SendServerResponse(int socket, int resp)
{
	Packet* packet = new Packet(13);
	packet->Write(resp);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::SendServerMessage(int socket, std::string message)
{
	Packet* packet = new Packet(14);
	packet->Write(message);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}

void SendPackets::RequestUserMainSocket(int socket)
{
	Packet* packet = new Packet(-301);

	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::SendMessageToRoom(int room, std::string msg)
{
	Packet* packet = new Packet(4);

	packet->Write(msg);
	packet->Finalize();
	SendTCPToAllInRoom(packet, room);
}
void SendPackets::LeaveRoomSuccess(int socket)
{
	Packet* packet = new Packet(15);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::NotifyUDPAssignmentSuccess(int socket)
{
	Packet* packet = new Packet(17);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::SendUserTheirSocket(int socket)
{
	Packet* packet = new Packet(3);
	packet->Write(socket);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}
void SendPackets::Pong(int socket)
{
	Packet* packet = new Packet(16);
	packet->Finalize();
	SendTCPToSocket(packet, socket, false);
}

void SendPackets::SendNewUser(RoomUser* user_tosend, Room* r)
{
	Packet* packet = new Packet(6);

	packet->Write(user_tosend->tcp_socket);
	packet->Write(user_tosend->name);
	packet->Write(user_tosend->voice_active);
	packet->Write(user_tosend->video_active);
	packet->Write(user_tosend->w);
	packet->Write(user_tosend->h);
	packet->Finalize();
	SendTCPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::SendNewUser(RoomUser* user_tosend, int to_sock)
{
	Packet* packet = new Packet(6);

	packet->Write(user_tosend->tcp_socket);
	packet->Write(user_tosend->name);
	packet->Write(user_tosend->voice_active);
	packet->Write(user_tosend->video_active);
	packet->Write(user_tosend->w);
	packet->Write(user_tosend->h);
	packet->Finalize();
	SendTCPToSocket(packet, to_sock, false);
}
void SendPackets::SendUserVoiceStatus(RoomUser* user_tosend, Room* r)
{
	Packet* packet = new Packet(7);

	packet->Write(user_tosend->tcp_socket);
	packet->Write(user_tosend->voice_active);
	packet->Finalize();
	SendTCPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::SendUserLeftRoom(int main_socket, Room* r)
{
	Packet* packet = new Packet(8);

	packet->Write(main_socket);
	packet->Finalize();
	SendTCPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::SendUserVideoStatus(RoomUser* user_tosend, Room* r, int a, int b)
{
	Packet* packet = new Packet(9);

	packet->Write(user_tosend->tcp_socket);
	packet->Write(user_tosend->video_active);
	packet->Write(a);
	packet->Write(b);
	packet->Finalize();
	SendTCPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::NotifyRoomUsersOfFile( FileStream *file, Room* r)
{
	if(file->Host == nullptr) {
		return;
	}
	Packet* packet = new Packet(10);

	packet->Write(file->Host->tcp_socket);
	packet->Write(file->filename);
	packet->Write(file->total_bytes);
	packet->Finalize();
	SendTCPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::StartFileProcess(bool greenlight, FileStream *file)
{

	if(file->Host == nullptr) {
		return;
	}
	Room* r = Program::FindRoomWithSocket(file->Host->tcp_socket);
	if(r == nullptr) {
		return;
	}

	Packet* packet = new Packet(11);

	packet->Write(file->Host->tcp_socket);
	packet->Write(greenlight);
	packet->Finalize();
	Packet* _packet2 = new Packet(11); // for non receivers
	_packet2->Write(file->Host->tcp_socket);
	_packet2->Write(false);
	_packet2->Finalize();
	std::vector<RoomUser*> ulist = r->User_List;
	for(unsigned int i = 0; i < file->Recipients.size(); ++i) {
		for(unsigned int ii = 0; ii < ulist.size(); ++ii) {
			if(ulist[ii] == file->Recipients[i]) {
				ulist.erase(ulist.begin() + ii--);
				break;
			}
		}
		SendTCPToSocket(packet, file->Recipients[i]->tcp_socket, true);
	}
	for(unsigned int i = 0; i < ulist.size(); ++i) {
		SendTCPToSocket(_packet2, ulist[i]->tcp_socket, true);
	}
	delete(packet);
	delete(_packet2);
}
void SendPackets::FileMoveOn( FileStream *file)
{
	if(file->Host == nullptr) {
		return;
	}
	Packet* packet = new Packet(12);

	packet->Finalize();
	SendTCPToSocket(packet, file->Host->tcp_socket, false);
}
void SendPackets::SendFileSection( FileStream *file, boost_sharedptr_bytes chunk)//
{
	if(file->Host == nullptr) {
		return;
	}

	file->count_of_tens++;
	if(file->count_of_tens >= 10) {
		file->count_of_tens = 0;
		FileMoveOn(file);
	}
	Packet* packet = new Packet(-302);

	packet->Write(file->Host->tcp_socket);
	packet->Write(chunk);
	packet->Finalize();

	for(auto rec : file->Recipients) {
		SendTCPToSocket(packet, rec->room_tcp_socket, true);
	}
	delete(packet);
}
void SendPackets::SendVoiceBufferData(int socket, audio_buffer_data data, int ts)
{
	Room* r = Program::FindRoomWithSocket(socket);
	if(r == nullptr)
		return;
	Packet* packet = new Packet(-2);

	packet->Write(socket);
	packet->Write(data);
	packet->Write(ts);
	packet->Finalize();

	SendUDPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::SendVideoBufferData(int socket, boost_sharedptr_bytes data, int frameid, int index, int total_in_set, int ts, int w)
{
	Room* r = Program::FindRoomWithSocket(socket);
	if(r == nullptr)
		return;
	Packet* packet = new Packet(-3);

	packet->Write(socket);
	packet->Write(frameid);
	packet->Write(index);
	packet->Write(total_in_set);
	packet->Write(data);
	packet->Write(ts);
	packet->Write(w);
	packet->Finalize();

	SendUDPToAllInRoom(packet, r->tcp_socket);
}
void SendPackets::UDPPong(int host_socket, struct sockaddr_in addr)
{
	Packet* packet = new Packet(-4);
	packet->Finalize();
	SendUDPToAddress(packet, host_socket, addr);
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
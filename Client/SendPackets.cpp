#include "SendPackets.h"

void SendPackets::SendTCP(Packet* packet)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	send(Program::network_socket, bytearr, length, MSG_NOSIGNAL);
	delete packet;
}
void SendPackets::SendNameToServer(std::string username)
{
	Packet* packet = new Packet(1);
	packet->Write(username);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::RequestRoomCreation(std::string roomname)
{
	Packet* packet = new Packet(2);
	packet->Write(roomname);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::SendMessageToRoom(std::string message)
{
	Packet* packet = new Packet(3);
	packet->Write(message);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::OpenVoice(bool open)
{
	Packet* packet = new Packet(4);
	packet->Write(open);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::OpenVideo(bool open, int width, int height)
{
	Packet* packet = new Packet(5);
	packet->Write(open);
	packet->Write(width);
	packet->Write(height);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::InitiateFileStream(unsigned long long bytecount, std::string filename)
{
	Packet* packet = new Packet(6);
	packet->Write((unsigned long long)bytecount);
	packet->Write(filename);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::UserFileStreamDecision(int host, bool decision)
{
	Packet* packet = new Packet(7);
	packet->Write(decision);
	packet->Write(host);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::RequestLeaveRoom()
{
	Packet* packet = new Packet(8);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::TerminateFileStream()
{
	Packet* packet = new Packet(9);
	packet->Finalize();
	SendTCP(packet);
}
void SendPackets::Ping()
{
	Packet* packet = new Packet(10);
	packet->Finalize();
	SendTCP(packet);
}

/*___________________________________________________________________________________________________________________________________
THIS SECTION IS FOR ROOM TCP & UDP
____________________________________________________________________________________________________________________________________*/
void SendPackets::SendRoomTCP(Packet* packet)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	send(Program::room_socket, bytearr, length, MSG_NOSIGNAL);
	delete packet;
}
void SendPackets::SendRoomUDP(Packet* packet)
{
	const int length = packet->Bytes_To_Send.size();
	unsigned char bytearr[length];
	std::copy(packet->Bytes_To_Send.begin(), packet->Bytes_To_Send.end(), bytearr);
	sendto(Program::room_udp_socket, bytearr, length,0, (const struct sockaddr*)&Program::room_address, sizeof(Program::room_address));
	delete packet;
}
void SendPackets::SendMyTCPSocket(int socket)
{
	Packet* packet = new Packet(-1);
	packet->Write(socket);
	packet->Finalize();
	SendRoomTCP(packet);
}

void SendPackets::SendVoiceBufferData(int socket, audio_buffer_data data, int root_timestamp)
{
	Packet* packet = new Packet(-2);
	packet->Write(socket);
	packet->Write(data);
	packet->Write(root_timestamp);
	packet->Finalize();

	SendRoomUDP(packet);
}
void SendPackets::SendVideoBufferData(int frameid, int index, int socket, boost_sharedptr_bytes data, int total_in_set, int root_timestamp, int width)
{

	Packet* packet = new Packet(-3);
	packet->Write(socket);
	packet->Write(frameid);
	packet->Write(index);
	packet->Write(total_in_set);
	packet->Write(data);
	packet->Write(root_timestamp);
	packet->Write(width);
	packet->Finalize();
	SendRoomUDP(packet);
}
void SendPackets::SendFileSection(boost_sharedptr_bytes chunk)
{
	Packet* packet = new Packet(-302);
	packet->Write(Program::mysocket);
	packet->Write(chunk);
	packet->Finalize();
	SendRoomTCP(packet);
}
void SendPackets::UDPPing(int socket)
{
	Packet* packet = new Packet(-4);
	packet->Write(socket);
	packet->Finalize();
	SendRoomUDP(packet);
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
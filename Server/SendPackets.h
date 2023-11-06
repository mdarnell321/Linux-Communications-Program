#ifndef SENDPACKETS_H
#define SENDPACKETS_H

#include "Packet.h"
#include "Program.h"

class SendPackets
{
public:
	static void SendTCPToAll(Packet*);
	static void SendTCPToSocket(Packet*, int,bool);
	static void RequestNameFromUser(int);
	static void SendRoomCreationSuccess(int, int, int,int);
	static void RequestUserMainSocket(int);
	static void SendUserTheirSocket(int);
	static void SendTCPToAllInRoom(Packet*, int);
	static void SendMessageToRoom(int, std::string);
	static void SendTCPToAllInRoomExcept(Packet*, int, int);
	static void SendNewUser(RoomUser*, Room*);
	static void SendNewUser(RoomUser*, int);
	static void SendUserVoiceStatus(RoomUser*, Room*);
	static void SendUserLeftRoom(int, Room*);
	static void SendUserVideoStatus(RoomUser*, Room*, int, int);
	static void NotifyRoomUsersOfFile( FileStream *, Room*);
	static void StartFileProcess(bool,  FileStream *);
	static void SendFileSection( FileStream *, boost_sharedptr_bytes);
	static void FileMoveOn( FileStream *);
	static void SendVoiceBufferData(int, audio_buffer_data,int);
	static void SendVideoBufferData(int, boost_sharedptr_bytes, int, int, int, int, int);
	static void SendUDPToAllInRoom(Packet*, int);
	static void SendServerResponse(int, int);
	static void SendServerMessage(int, std::string);
	static void LeaveRoomSuccess(int);
	static void Pong(int);
	static void NotifyUDPAssignmentSuccess(int );
	static void SendUDPToAddress(Packet*, int, struct sockaddr_in );
	static void UDPPong(int, struct sockaddr_in);
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
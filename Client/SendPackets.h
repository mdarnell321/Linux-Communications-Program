#ifndef SENDPACKETS_H
#define SENDPACKETS_H

#include "Packet.h"
#include "Program.h"
#include "V4L2Manager.h"

class SendPackets
{
    public:
    static void SendTCP(Packet*);
    static void SendNameToServer(std::string);
    static void SendRoomTCP(Packet*);
    static void RequestRoomCreation(std::string);
    static void SendMyTCPSocket(int);
    static void SendMessageToRoom(std::string);
    static void SendRoomUDP(Packet*);
	static void SendVoiceBufferData(int, audio_buffer_data,int);
	static void OpenVoice(bool);
	static void OpenVideo(bool,int,int);
	static void SendVideoBufferData(int, int, int, boost_sharedptr_bytes, int,int, int);
	static void InitiateFileStream(unsigned long long, std::string);
	static void SendFileSection(boost_sharedptr_bytes);
	static void UserFileStreamDecision(int, bool);
	static void RequestLeaveRoom();
	static void TerminateFileStream();
	static void Ping();
	static void UDPPing(int);
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
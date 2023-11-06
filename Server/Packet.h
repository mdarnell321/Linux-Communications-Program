#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

typedef struct {
	boost::shared_ptr<short[]> array;
	unsigned int length;
	int timestamp;
} audio_buffer_data; 

typedef struct { //used if accessed on more than one thread
	boost::shared_ptr<unsigned char[]> array;
	unsigned int length;
	int index;
} boost_sharedptr_bytes;

typedef struct { //used if only accessed on one thread
	std::shared_ptr<unsigned char[]> array;
	unsigned int length;
	int index;
} sharedptr_bytes;

typedef struct {
	void *array;
	unsigned int length;
} bytes;

typedef struct {
	std::vector<boost_sharedptr_bytes> buff;
	int id;
	unsigned int required_length;
	int timestamp;
	int width;
} fragmented_frame; // for video

class Packet
{
public:
	std::vector<unsigned char> Bytes_To_Send;
	boost::shared_ptr<unsigned char[]> Received_Bytes = 0;
	unsigned int Receive_Length = 0;
	unsigned int _seeker = 0;
	void Finalize();
	void Write(int);
	int GetInt(bool*);
	void Write(short);
	short GetShort(bool*);
	unsigned long long GetULL(bool*);
	void Write(std::string);
	std::string GetString(bool*);
	void Write(audio_buffer_data);
	audio_buffer_data GetShortBufferData(bool*);
	void Write(bool);
	void Write(unsigned long long);
	bool GetBool(bool*);
	void Write(boost_sharedptr_bytes);
	bool CheckFinalizers(bool *);
	boost_sharedptr_bytes GetBytes(bool*);
	boost_sharedptr_bytes GetBytesToChar(bool*);
	Packet(boost::shared_ptr<unsigned char[]>, unsigned int);
	Packet(int);
	~Packet();
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
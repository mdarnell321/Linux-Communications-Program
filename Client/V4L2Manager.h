#ifndef V4L2MANAGER_H
#define V4L2MANAGER_H

#include <cstring>
#include <thread>
#include <algorithm> 
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <jpeglib.h>
#include "Packet.h"

class V4L2Manager
{
public:
	static unsigned int array_size; // for queue
	static std::string cam_path;
	static bool Start(int);
	static bool Initialize_MMAP(int);
	static bool SetSize(int, unsigned int, unsigned int );
	static bool GetSize(int, unsigned int*, unsigned int*);
	static bool GetFPS(int ,unsigned int*);
	static bool SetFPS(int,unsigned int);
	static bool Initialize_Device(int);
	static int Open();
	static bool Stop(int);
	static bool Readable(int , timeval*);
	static sharedptr_bytes Dequeue(int);
	static std::shared_ptr<unsigned char[]> DecompressJPEG(boost::shared_ptr<unsigned char[]> , unsigned int, unsigned int *, unsigned int *, bool );
	static fragmented_frame SplitFrame(unsigned char*, unsigned int);
	static boost_sharedptr_bytes AssembleFrame( fragmented_frame );
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
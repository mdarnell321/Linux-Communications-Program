#include "V4L2Manager.h"

#define V4L2BUFFCOUNT 5
#define VIDEO_BYTES 1024 //number of bytes to send at a time to the server when streaming video.
unsigned int buff_size = 0;
bytes* buffer = nullptr;
std::string V4L2Manager::cam_path = "";
unsigned int V4L2Manager::array_size = 0;

bool V4L2Manager::Start(int fd)
{
	for (unsigned int i = 0; i < buff_size; ++i) {
		struct v4l2_buffer buff;
		buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buff.memory = V4L2_MEMORY_MMAP;
		buff.index = i;

		if (ioctl(fd, VIDIOC_QBUF, &buff) == -1)
			return false;
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
		return false;
	return true;
}

bool V4L2Manager::Initialize_MMAP(int fd)
{

	struct v4l2_requestbuffers req;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	req.count = V4L2BUFFCOUNT;
	
	if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
		return false;
	if (req.count < 2)
		return false;
	if(buffer != nullptr)
		delete buffer;
	buff_size = req.count;
	buffer = new bytes[buff_size]();
	for (int i = 0; i < buff_size; ++i) {
		struct v4l2_buffer buff;
		buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buff.memory = V4L2_MEMORY_MMAP;
		buff.index = i;

		if (ioctl(fd, VIDIOC_QUERYBUF, &buff) == -1)
			return false;
		buffer[i].length = buff.length;
		buffer[i].array = mmap(nullptr, buff.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buff.m.offset);
		if (buffer[i].array == MAP_FAILED)
			return false;
	}
	return true;
}

bool V4L2Manager::SetSize(int fd, unsigned int width, unsigned int height)
{
	struct v4l2_format fmt;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

	if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
		return false;
	return true;
}
bool V4L2Manager::GetSize(int fd,unsigned int *width, unsigned int *height)
{
	struct v4l2_format fmt;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

	if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
		return false;
	*width = fmt.fmt.pix.width;
	*height = fmt.fmt.pix.height;

	return true;
}
bool V4L2Manager::GetFPS(int fd,unsigned int *fps)
{
	struct v4l2_streamparm sp;
	sp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(fd, VIDIOC_G_PARM, &sp) == -1)
		return false;
	*fps = (unsigned int)((float)sp.parm.capture.timeperframe.denominator/sp.parm.capture.timeperframe.numerator);
	return true;
}
bool V4L2Manager::SetFPS(int fd,unsigned int fps)
{
	struct v4l2_streamparm sp;
	sp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	sp.parm.capture.timeperframe.denominator = fps;
	sp.parm.capture.timeperframe.numerator = 1;

	if (ioctl(fd, VIDIOC_S_PARM, &sp) == -1)
		return false;
	return true;
}
bool V4L2Manager::Initialize_Device(int fd)
{
	struct v4l2_capability capability;

	if (ioctl(fd, VIDIOC_QUERYCAP, &capability) == -1)
		return false;
	if (capability.capabilities & V4L2_CAP_VIDEO_CAPTURE == false)
		return false;

	struct v4l2_format fmt;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
		return false;

	return true;
}
bool V4L2Manager::Stop(int fd)
{
	bool good = true;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
		good = false;
	for (unsigned int i = 0; i < buff_size; ++i) {
		if (munmap(buffer[i].array, buffer[i].length) == -1)
			good = false;
	}
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	req.count = 0;
	
	if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
		good = false;
	if(buffer != nullptr)
		delete buffer;
	buffer = nullptr;
	buff_size = 0;
	return good;
}
int V4L2Manager::Open()
{
	int fd = open(cam_path.c_str(), O_RDWR | O_NONBLOCK, 0);
	return fd;
}
bool V4L2Manager::Readable(int fd, timeval *tv)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	if(select(fd+1, &fds, nullptr, nullptr, tv) == 1)
		return true;
	return false;
}

sharedptr_bytes V4L2Manager::Dequeue(int fd)
{

	unsigned int size = 0;
	if (buff_size > 0) {
		struct v4l2_buffer buff;
		buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buff.memory = V4L2_MEMORY_MMAP;

		if (ioctl(fd, VIDIOC_DQBUF, &buff) == -1)
			return sharedptr_bytes{nullptr,0};
			
		if (buff.index < buff_size) {
			size = buff.bytesused;
			if(size > array_size)
				array_size = size + 2000;
			std::shared_ptr<unsigned char[]> bytearr(new unsigned char [array_size]());
			std::copy((unsigned char*)buffer[buff.index].array, (unsigned char*)buffer[buff.index].array + size, bytearr.get());

			if (ioctl(fd, VIDIOC_QBUF, &buff) == -1)
				return sharedptr_bytes{nullptr,0};
			return sharedptr_bytes{bytearr,array_size};
		}
	}
	return sharedptr_bytes{nullptr,0};
}

std::shared_ptr<unsigned char[]> V4L2Manager::DecompressJPEG(boost::shared_ptr<unsigned char[]> source, unsigned int source_size, unsigned int *width, unsigned int *height, bool scale_down)
{
	struct jpeg_decompress_struct info;
	struct jpeg_error_mgr error;
	info.err = jpeg_std_error(&error);
	jpeg_create_decompress(&info);
	jpeg_mem_src(&info, source.get(), source_size);
	if(jpeg_read_header(&info, true) != JPEG_HEADER_OK) {
		jpeg_destroy_decompress(&info);
		return nullptr;
	}

	info.scale_num = 1;
	info.scale_denom = scale_down == true ? 2:1;
	jpeg_start_decompress(&info);
	(*width) = info.output_width;
	(*height) = info.output_height;
	unsigned int row_stride = (*width) * info.output_components;
	unsigned int arr_length = (*height) * row_stride;
	std::shared_ptr<unsigned char[]> out (new unsigned char[arr_length]);
	while (info.output_scanline < *height) {
		unsigned char *data = out.get() + info.output_scanline * row_stride;
		jpeg_read_scanlines(&info, &data, 1);
	}
	jpeg_destroy_decompress(&info);
	return out;
}

fragmented_frame V4L2Manager::SplitFrame(unsigned char *arr, unsigned int length)
{
	fragmented_frame temp;

	for(unsigned int i = 0 ; i < length; ++i) {
		unsigned int len = (i + VIDEO_BYTES >= length) ? (length - i) : VIDEO_BYTES;
		boost::shared_ptr<unsigned char[]> data( new unsigned char[len]);
		for(unsigned int ii = 0; ii < len; ++ii) {
			data.get()[ii] = arr[i+ii];
		}

		temp.buff.push_back(boost_sharedptr_bytes{data,len});
		i+= len - 1;
	}
	return temp;
}
boost_sharedptr_bytes V4L2Manager::AssembleFrame( fragmented_frame pieces)
{
	unsigned int length =0;
	for(unsigned int i = 0; i < pieces.buff.size(); ++ i ) {
		length += pieces.buff[i].length;
	}
	boost::shared_ptr<unsigned char[]> assembled( new unsigned char[length]);
	unsigned int seeker = 0;
	bool corrupted = false;
	for(unsigned int i = 0; i < pieces.buff.size(); ++i ) {
		if(corrupted != true) {
			for(unsigned int ii = 0; ii < pieces.buff[i].length; ++ii) {
				if(seeker < length && pieces.buff[i].array != nullptr && pieces.buff[i].length != 0) {
					assembled[seeker++] = pieces.buff[i].array[ii];
				} else {
					corrupted = true;
				}
			}
		}
		if(pieces.buff[i].array!= nullptr)
			pieces.buff[i].array = nullptr;
	}
	if(corrupted == true)
		return boost_sharedptr_bytes{nullptr, 0};
	else
		return boost_sharedptr_bytes{assembled, length};
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

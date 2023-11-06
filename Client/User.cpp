#include "User.h"
#include "Program.h"
#include "SendPackets.h"
#include "Program.h"
#include "MemoryManagement.h"


#define FILE_BYTES 1024 //number of bytes to send at a time to the server when streaming a file.
#define VOICE_BYTES 256 //number of bytes to send at a time to the server when streaming voice.
#define BUFF_TIMESPAN 100
#define TARGET_VIDEO_REC_FPS 30
User::User(int mainsock, std::string n, bool _voice, bool _video)
{
	name = n;
	main_tcp_socket = mainsock;
	active = true;
	std::cout << n <<" has joined.\n";
	if(_voice == true)
		StartVoice();
	else
		voice_active = false;
	jobs_running = true;
	udp_jobs_thread = new std::thread(&User::UDPJobs, this);
	MemoryManagement::Add_Obj(udp_jobs_thread);
}

User::~User()
{
	std::cout << name << " left.\n";
	if(this == Program::me) {
		Program::me = nullptr;
	}
	if(termination_thread != nullptr) {
		termination_thread ->detach();
		delete termination_thread;
	}
}

void User::TaskTo_UserVideoThread(std::packaged_task<void()> a)
{
	if(jobs_running == false)
		return;

	std::lock_guard<std::mutex> lock(ActionsOnUserVideoThread_mutex);
	ActionsOnUserVideoThread.push_back(std::move(a));

}
void User::TaskTo_UserVoiceThread(std::packaged_task<void()> a)
{
	if(jobs_running == false)
		return;

	std::lock_guard<std::mutex> lock(ActionsOnUserVoiceThread_mutex);
	ActionsOnUserVoiceThread.push_back(std::move(a));

}
void User::TaskTo_UserUDPThread(std::packaged_task<void()> a)
{
	if(jobs_running == false)
		return;

	std::lock_guard<std::mutex> lock(ActionsOnUserUDPThread_mutex);
	ActionsOnUserUDPThread.push_back(std::move(a));


}

std::string Get_Executable_DIR()
{
	char path[256];
	unsigned int count = readlink("/proc/self/exe", path, 256);
	std::string s = std::string(path, count > 0 ? count : 0);
	return s.substr(0, s.find_last_of("/"));
}

void User::Handle_File(FileStream* f)
{
	int lastval = 0;
	int sectioncount = 0;
	if(this == Program::me) {
		std::ifstream inFile(Form::mainFrame->GetPick_File()->GetPath(), std::ios::binary);
		char read_byte = 0;
		unsigned long long totalpos = 0;
		unsigned long long sector_pos = 0;
		unsigned int length = 0;
		unsigned char section[FILE_BYTES];
		int index = 0;
		bool success = false;
		while(inFile.read(reinterpret_cast<char*>(&read_byte), 1) && Program::room_udp_socket != -1) {

			while(Program::CanContinueFile == false);
			success = true;
			totalpos++;

			int normalpos = (int)(((long double)totalpos / f->total_bytes)*100);
			if(normalpos > 100)
				normalpos = 100;
			else if(normalpos < 10)
				normalpos = 10;

			if(normalpos % 5 == 0 && normalpos != lastval) {
				lastval = normalpos;
				Form::mainFrame->CallAfter([normalpos]() {
					Form::mainFrame->GetMyFileGauge()->SetValue(normalpos);
					Form::mainFrame->GetMyFileGauge()->Update();
				});
			}


			if(sector_pos == 0)
				length = (int)(totalpos + (unsigned long long)FILE_BYTES >= f->total_bytes ? (f->total_bytes - totalpos) + 1 : FILE_BYTES);
			section[sector_pos++] = (unsigned char)read_byte ;
			if(sector_pos >= (unsigned long long)FILE_BYTES || totalpos == f->total_bytes) {

				if(++sectioncount == 10) {
					sectioncount = 0;
					Program::CanContinueFile = false;
				}
				boost::shared_ptr<unsigned char[]> temporary(new unsigned char[FILE_BYTES]);
				std::copy(section, section + length, temporary.get());
				SendPackets::SendFileSection(boost_sharedptr_bytes{temporary, length, index++});
				sector_pos = 0;
			}
		}
		if(success == false) {
			SendPackets::TerminateFileStream();
		}
		Program::CanContinueFile = true;
		inFile.close();
	} else {
		mkdir((Get_Executable_DIR() + "/Downloads/").c_str(), 0777);
		std::ofstream file;
		file.open(Get_Executable_DIR() +"/Downloads/" + f->filename, std::ios::binary);
		assert(file.is_open());
		while(f->total_bytes != f->bytes_processed && f->cant_continue == false&& Program::room_udp_socket != -1) {
			if(f->buff.size() > 0) {
				bool passed = false;
				std::vector<char> temp ;
				Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [f, &passed, &temp]()
				{
					if(f->buff.size() > 0) {
						temp = f->buff;
						f->bytes_processed+= temp.size();
						f->buff.clear();
					}
					passed = true;
				}
				                                                                });
				while(passed == false);
				file.write(reinterpret_cast<char*>(&temp[0]), temp.size());
				temp.clear();
			}

			int normalpos = (int)(((long double)f->bytes_processed / f->total_bytes)*100);
			if(normalpos > 100)
				normalpos = 100;
			else if(normalpos < 10)
				normalpos = 10;

			short file_index = -1;
			for(unsigned int i = 0; i < Program::File_Transfers.size(); ++i) {
				if(f == Program::File_Transfers[i]) {
					file_index = i;
					break;
				}
			}
			switch(file_index) {
			case 0:
				if(normalpos % 2 == 0 && normalpos != lastval) {
					lastval = normalpos;
					Form::mainFrame->CallAfter([ normalpos]() {
						Form::mainFrame->GetAStream_Percentage()->SetLabel(wxString::Format("%i%% ", normalpos));
						Form::mainFrame->GetAStream_Percentage()->Update();
					});
				}
				break;
			case 1:
				if(normalpos % 2 == 0 && normalpos != lastval) {
					lastval = normalpos;
					Form::mainFrame->CallAfter([ normalpos]() {
						Form::mainFrame->GetBStream_Percentage()->SetLabel(wxString::Format("%i%% ", normalpos));
						Form::mainFrame->GetBStream_Percentage()->Update();
					});
				}
				break;
			case 2:
				if(normalpos % 2 == 0 && normalpos != lastval) {
					lastval = normalpos;
					Form::mainFrame->CallAfter([ normalpos]() {
						Form::mainFrame->GetCStream_Percentage()->SetLabel(wxString::Format("%i%% ", normalpos));
						Form::mainFrame->GetCStream_Percentage()->Update();
					});
				}
				break;
			default:
				break;
			}

		}
		file.close();
	}
	Form::mainFrame->CallAfter([]() {
		Form::mainFrame->GetMyFileGauge()->SetValue(1);
		Form::mainFrame->GetMyFileGauge()->Update();
	});
	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [&f]()
	{
		for(unsigned int i =0; i < Program::File_Transfers.size(); ++i) {
			if(Program::File_Transfers[i] == f) {
				FileStream *temp_f = f;
				Program::File_Transfers.erase(Program::File_Transfers.begin()+i);
				f = nullptr;
				MemoryManagement::Delete_Obj(temp_f,2);
			}
		}
		if(f == Program::MyFile && f != nullptr) {
			MemoryManagement::Delete_Obj(Program::MyFile,2);
			Program::MyFile = nullptr;
		}
		Form::mainFrame->RefreshPendingFiles();
	}
	                                                                });
	MemoryManagement::Delete_Thread(std::move(handle_file_thread));
}

void User::UDPJobs()
{
	while (jobs_running) {
		std::vector<std::packaged_task<void()>> _temp;
		std::unique_lock<std::mutex> lock(ActionsOnUserUDPThread_mutex);
		while(ActionsOnUserUDPThread.empty() == false) {
			_temp.push_back(std::move(ActionsOnUserUDPThread.front()));
			ActionsOnUserUDPThread.pop_front();
		}
		lock.unlock();
		for(int i =0 ; i < _temp.size(); ++i) {
			(_temp[i])();
		}
	}
	MemoryManagement::Delete_Thread(std::move(udp_jobs_thread));
	MemoryManagement::Delete_Obj(std::move(this),0);
}
float MeanOfMostCommonGroup(std::vector<int> _list)
{
	std::sort(_list.begin(), _list.end());
	for(unsigned int i = 0; i < _list.size(); ++i) {
		if(_list[i] <= 0)
			_list.erase(_list.begin() + (i--));
	}
	/*for(int i = 0; i < l.size(); ++i) {
		std::cout << l[i] << "\n";
	}*/

	std::vector<std::vector<int>> grouping;
	for(int i = _list.size() - 1; i > 0; --i) {
		if(i != 0) {
			if( ((float)(_list[i]) / _list[i-1]) > 1.1) {
				grouping.push_back(std::vector<int>(_list.begin() + (i), _list.end()));
				_list.erase(_list.begin() + (i), _list.end());
				i = _list.size();
			}
		}
	}
	if(_list.size() != 0)
		grouping.push_back(std::vector<int>(_list.begin(), _list.end()));

	unsigned int count = 0;
	unsigned int index = 0;
	for(unsigned int i = 0; i < grouping.size(); ++i) {
		if(grouping[i].size() > count) {
			index = i;
			count = grouping[i].size();
		}
	}
	std::vector<int> to_use = grouping[index];

	int total = 0;
	for(unsigned int i = 0; i < to_use.size(); ++i) {
		total+= to_use[i];
	}
	return ((float)total/to_use.size());
}

void User::Voice()
{
	if(User::main_tcp_socket == Program::mysocket) { //record

		if(input_device == nullptr) {
			input_device = alcCaptureOpenDevice(nullptr,22050,AL_FORMAT_MONO16,4096);
			alcCaptureStart(input_device);
			SendPackets::OpenVoice(true);
		} else
			goto End;
		bool alreadysetbuttonactive = false;
		while(voice_active == true) {
			if(alreadysetbuttonactive == false) {
				alreadysetbuttonactive = true;
				Form::mainFrame->CallAfter([]() {
					Form::mainFrame->GetVoice_Button()->Enable(true);
				});
			}
			ALCint samples=0;
			alcGetIntegerv(input_device,ALC_CAPTURE_SAMPLES,1,&samples);
			if (samples>=VOICE_BYTES) {
				boost::shared_ptr<short[]> data(new short[samples*2]);
				alcCaptureSamples(input_device,data.get(),samples);

				SendPackets::SendVoiceBufferData(Program::mysocket, audio_buffer_data {data,(unsigned int)samples}, Program::ms_Elapsed);
			}
		}
		if(input_device != nullptr) {
			alcCaptureStop(input_device);
			alcCaptureCloseDevice(input_device);
			input_device = nullptr;
		}

		Form::mainFrame->CallAfter([]() {
			Form::mainFrame->GetVoice_Button()->Enable(true);
		});
	} else { //listen

		while(voice_buffer.size() < 25 && voice_active == true) {
			std::vector<std::packaged_task<void()>> _temp;
			std::unique_lock<std::mutex> lock(ActionsOnUserVoiceThread_mutex);
			while(ActionsOnUserVoiceThread.empty() == false && voice_active == true) {
				_temp.push_back(std::move(ActionsOnUserVoiceThread.front()));
				ActionsOnUserVoiceThread.pop_front();
			}
			lock.unlock();
			for(int i =0 ; i < _temp.size(); ++i) {
				if(voice_active == false)
					break;
				(_temp[i])();
			}
		}
		if(voice_active == false) {
			ClearVoiceBuff();
			goto End;
		}

		std::vector<int> temp_list;
		for(unsigned int i = 0; i < voice_buffer.size() - 1; ++i)
			temp_list.push_back(voice_buffer[i+1].timestamp-voice_buffer[i].timestamp);

		float avgsamplelength_ms = MeanOfMostCommonGroup(temp_list);
		const unsigned int VOICEBUFFER_SIZE = BUFF_TIMESPAN / (avgsamplelength_ms);
		while(voice_buffer.size() > VOICEBUFFER_SIZE) {
			voice_buffer.pop_front();
		}
		Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [VOICEBUFFER_SIZE,this]()
		{
			if(openal_buffer == nullptr)
				openal_buffer = new ALuint[VOICEBUFFER_SIZE];
			if(Program::output_device == nullptr) {
				Program::output_device = alcOpenDevice(nullptr);
				Program::audio_context = alcCreateContext(Program::output_device,nullptr);
				alcMakeContextCurrent(Program::audio_context);
			}
			alGenBuffers(VOICEBUFFER_SIZE,this->openal_buffer);
			alGenSources (1, &this->cur_source);
			this->passed_listen_init = true;
		}
		                                                                });
		while(passed_listen_init == false && voice_active == true);
		int retrytimes_videolead = -1;
		int retrytimes_audiolead = -1;
		bool goingthroughreset = false;
		std::deque<ALuint> set_aside_buffer; //elements of buffer were set aside due to not enough data to assign them with
		for( unsigned int i = 0; i < VOICEBUFFER_SIZE; ++i)
			set_aside_buffer.push_back(openal_buffer[i]);
		int times_queued = 0; // how many elements are in the AL queue
		int recent_timestamp = 0;
		int last_vid_timestamp_lead = 0;
		int last_vid_timestamp_lag = 0;
		while(voice_active == true) {

			std::vector<std::packaged_task<void()>> _temp;
			std::unique_lock<std::mutex> lock(ActionsOnUserVoiceThread_mutex);
			while(ActionsOnUserVoiceThread.empty() == false && voice_active == true) {
				_temp.push_back(std::move(ActionsOnUserVoiceThread.front()));
				ActionsOnUserVoiceThread.pop_front();
			}
			lock.unlock();
			for(int i =0 ; i < _temp.size(); ++i) {
				if(voice_active == false)
					break;
				(_temp[i])();
			}
			if(voice_buffer.size() + times_queued < VOICEBUFFER_SIZE)
				continue;
			//Video and audio sync process
			if(goingthroughreset == true) {
				if(voice_buffer.empty() == true && video_active == true || voice_buffer.empty()== false && (voice_buffer.front().timestamp - VideoTimeStamp) > 10 && video_active == true) {
					continue;
				}
			}

			goingthroughreset = false;
			if(video_active == true ) {
				if(recent_timestamp != 0) {

					if((recent_timestamp - VideoTimeStamp) > 100 && recent_timestamp > VideoTimeStamp && VideoTimeStamp > 0  && last_vid_timestamp_lead != VideoTimeStamp && VideoTimeStamp-last_vid_timestamp_lead > 2000 /*grace period of 2 seconds*/ ) {
						if(retrytimes_audiolead == -1)
							retrytimes_audiolead = Program::ms_Elapsed;
						if(retrytimes_audiolead > Program::ms_Elapsed)
							retrytimes_audiolead = -1;
						if( retrytimes_audiolead != -1 && Program::ms_Elapsed - retrytimes_audiolead > 2000) {
							last_vid_timestamp_lead = VideoTimeStamp;
							goingthroughreset = true;
							continue;
						}
					}
					if(recent_timestamp - VideoTimeStamp < -60 && VideoTimeStamp > 0 && last_vid_timestamp_lag != VideoTimeStamp && VideoTimeStamp-last_vid_timestamp_lag > 2000 ) {  // if video is leading
						if(retrytimes_videolead == -1)
							retrytimes_videolead = Program::ms_Elapsed;
						if(retrytimes_videolead > Program::ms_Elapsed)
							retrytimes_videolead = -1;
						if( retrytimes_videolead != -1 && Program::ms_Elapsed - retrytimes_videolead > 2000) {
							last_vid_timestamp_lag = VideoTimeStamp;
							while(voice_buffer.size() > 0 && voice_active == true) {
								voice_buffer.clear();
							}
							int finished=0;
							ALuint buff[8];
							alGetSourcei(cur_source,AL_BUFFERS_PROCESSED,&finished);
							alSourceUnqueueBuffers(cur_source,finished,buff);
							times_queued-=finished;
							for (int i=0; i<finished; ++i) {
								set_aside_buffer.push_back(buff[i]);
							}
							continue;
						}
					}
					if((recent_timestamp - VideoTimeStamp) > -60)
						retrytimes_videolead = -1;
					if((recent_timestamp - VideoTimeStamp) < 100)
						retrytimes_audiolead = -1;

				}
			} else {
				if(video_was_active != video_active && video_active == false) {
					while(voice_buffer.size() > 1 && voice_active == true)
						voice_buffer.clear();
					video_was_active = video_active;
					continue;
				}
				if(voice_buffer.size() > VOICEBUFFER_SIZE * 1.75) {
					while(voice_buffer.size() > VOICEBUFFER_SIZE) {
						voice_buffer.pop_front();
					}
				}
			}
			video_was_active = video_active;
			//Audio process
			bool first = true;
			int finished=0;

			alGetSourcei(cur_source,AL_BUFFERS_PROCESSED,&finished);
			if (finished>0) {
				ALuint buff[VOICEBUFFER_SIZE];
				alSourceUnqueueBuffers(cur_source,finished,buff);
				times_queued-=finished;
				for (int i=0; i<finished; ++i) {
					if(voice_buffer.size() > 0) { // take this finished queue and add it back to device queue
						if(first == true) {
							first = false;
							recent_timestamp = voice_buffer.front().timestamp;
						}

						alBufferData(buff[i],AL_FORMAT_MONO16,voice_buffer.front().array.get(),voice_buffer.front().length * 2,22050);
						alSourceQueueBuffers(cur_source,1,&buff[i]); //pushed on device source queue when data is assigned
						times_queued++;
						audio_buffer_data data = voice_buffer.front();
						data.array = nullptr;
						voice_buffer.pop_front();
					} else {
						set_aside_buffer.push_back(buff[i]);
					}
				}
			}

			while(voice_active == true) {
				if(voice_buffer.size() > 0 && set_aside_buffer.size() > 0) {
					if(first == true) {
						first = false;
						recent_timestamp = voice_buffer.front().timestamp;
					}
					alBufferData(set_aside_buffer.front(),AL_FORMAT_MONO16,voice_buffer.front().array.get(),voice_buffer.front().length * 2,22050);//
					alSourceQueueBuffers(cur_source,1,&set_aside_buffer.front());
					times_queued++;
					audio_buffer_data data = voice_buffer.front();
					data.array = nullptr;
					voice_buffer.pop_front();
					set_aside_buffer.pop_front();
				} else
					break;
			}

			if (times_queued > 0) {
				ALint source_state=0;
				alGetSourcei(cur_source,AL_SOURCE_STATE,&source_state);
				if (source_state!=AL_PLAYING) {
					alSourcePlay(cur_source);
				}
			}
		}

		Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this,VOICEBUFFER_SIZE]()
		{
			alSourceStopv(1,&cur_source);
			alSourcei(cur_source,AL_BUFFER,0);
			alDeleteBuffers(VOICEBUFFER_SIZE,openal_buffer);
			alDeleteSources(1,&cur_source);

			if(openal_buffer != nullptr)
				delete[] openal_buffer;
			openal_buffer = nullptr;
			ClearVoiceBuff();
			bool good = true;
			for(unsigned int i = 0; i <  Program::User_List.size(); ++i) {
				if(Program::User_List[i]->voice_active == true)
					good = false;
			}
			if(good == true) {

				if(Program::audio_context != nullptr) {
					alcMakeContextCurrent(nullptr);
					alcDestroyContext(Program::audio_context);
				}
				if(Program::output_device != nullptr) {
					alcCloseDevice(Program::output_device);
				}
				Program::output_device = nullptr;
				Program::audio_context = nullptr;
			}
		}
		                                                                });

	}
End:
	MemoryManagement::Delete_Thread(std::move(voice_thread));
	voice_thread = nullptr;
	passed_listen_init = false;
}
void User::StartVoice()
{
	if(voice_active == true || voice_thread != nullptr)
		return;

	ClearVoiceBuff();
	voice_active = true;
	voice_thread = new std::thread(&User::Voice, this);
	MemoryManagement::Add_Obj(voice_thread);
}
void User::ClearVideoBuff()
{
	frames_lost = 0;
	frames_passed = 0;
	percent_frames_lost = 0;
	vid_send_timer = 0;
	for(int i = 0; i < (int)video_buffer.size(); ++i) {
		for(int ii = 0; ii < (int)video_buffer[i].buff.size(); ++ii) {
			video_buffer[i].buff[ii].array = nullptr;
		}
	}
	video_buffer.clear();
}
void User::ClearVoiceBuff()
{
	voice_buffer.clear();
}
void User::StopVoice()
{
	voice_active = false;
}

void RefreshCamLayout()
{
	for(unsigned int a = 0; a < Program::Screen_Video_Elements.size(); ++a) {
		int ylead = 0,posy = 0, posx = 0;

		for(unsigned int i = 0; i < (Program::Screen_Video_Elements.size() > 2 ? 2 : Program::Screen_Video_Elements.size()); ++i) {
			if((int)Program::Screen_Video_Elements[i].height > ylead)
				ylead = (int)Program::Screen_Video_Elements[i].height ;
		}//Found the greatest of the 2 height wise

		if(a< 2) {
			posy = 65;
			if( a == 0) {
				posx = 530;
			} else {
				posx = 530 + Program::Screen_Video_Elements[0].width;
			}
		} else {
			posy = 65+ylead;
			if( a == 2) {
				posx = 530;
			} else {
				posx = 530 + Program::Screen_Video_Elements[2].width;
			}
		}
		Form::mainFrame->CallAfter([a,posx,posy]() {
			Program::Screen_Video_Elements[a].source_ptr->SetPosition(wxPoint(posx,posy));
		});
	}
	if(Program::Screen_Video_Elements.size() == 0) {
		Form::mainFrame->CallAfter([]() {
			Form::mainFrame->SetMinSize(wxSize(500,700));
			Form::mainFrame->SetSize(wxSize(500,700));
			Form::mainFrame->Layout();
		});
	} else {
		Form::mainFrame->CallAfter([]() {
			Form::mainFrame->SetMinSize(wxSize(1300,700));
			Form::mainFrame->SetSize(wxSize(1300,700));
			Form::mainFrame->Layout();
		});
	}
}

std::vector<VidElements> Program::Screen_Video_Elements;
void User::Video()
{
	V4L2Manager::array_size = 0;
	vid_send_timer = 0;
	int cur_frame_id = 1;
	if(User::main_tcp_socket == Program::mysocket) {
		bool forward = true;
		unsigned int set_width;
		unsigned int set_height;
		timeval tv;

		if(Program::fd == -1) {
			if((Program::fd = V4L2Manager::Open()) == -1)
				forward = false;
		}
		int init_tries = 0;
tryagain:
		if(forward == true) {
			if(V4L2Manager::Initialize_Device(Program::fd) == false)
				forward = false;
			// Set proper size
			V4L2Manager::GetSize(Program::fd, &set_width,&set_height);
			float multiplier = (float)700/set_width;
			set_height*=multiplier;
			set_width*=multiplier;
			V4L2Manager::SetSize(Program::fd, set_width,set_height);
			unsigned int temp_width = 0;
			unsigned int temp_height = 0;
			V4L2Manager::GetSize(Program::fd, &temp_width,&temp_height);
			if(temp_height != set_height|| temp_width != set_width) {
				if(temp_height > set_height && (float)set_height/temp_height < 2 && temp_width > set_width && (float)set_width/temp_width < 2 ||
				   temp_height < set_height && (float)temp_height/set_height < 2 && temp_width < set_width && (float)temp_width/set_width < 2) {
					set_width = temp_width;
					set_height = temp_height;
				} else {
					if(init_tries++ > 50) {
						std::cout << "Failure in setting proper webcam resolution.\n";
						forward = false;
					} else
						goto tryagain;
				}
			}

			if(set_width == 0 || set_height == 0)
				forward = false;

			unsigned int fps = 0;
			if(forward == true) {
				V4L2Manager::GetFPS(Program::fd, &fps);
				if(fps > TARGET_VIDEO_REC_FPS) {
					V4L2Manager::SetFPS(Program::fd,TARGET_VIDEO_REC_FPS);

					V4L2Manager::GetFPS(Program::fd, &fps);
					if(fps != TARGET_VIDEO_REC_FPS)
						forward = false;
				}
			}
			//Initialization
			if(forward == true) {
				if(V4L2Manager::Initialize_MMAP(Program::fd) == false|| V4L2Manager::Start(Program::fd) == false)
					forward = false;
			}
		}

		bool alreadysetbuttonactive = false;
		if(forward == true ) {
			SendPackets::OpenVideo(true,set_width,set_height);
			if(alreadysetbuttonactive == false) {
				alreadysetbuttonactive = true;
				Form::mainFrame->CallAfter([]() {
					Form::mainFrame->GetVideo_Button()->Enable(true);
				});
			}
			while(video_active == true) {
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				if(V4L2Manager::Readable(Program::fd, &tv) == false)
					continue;
				int rts = Program::ms_Elapsed;
				sharedptr_bytes data = V4L2Manager::Dequeue(Program::fd);

				fragmented_frame frame = V4L2Manager::SplitFrame(data.array.get(), data.length); // fragment this frame so we can send it over the network by small parts
				for(int i = 0; i < (int)frame.buff.size() ; ++i) {
					vid_send_timer = 0;
					SendPackets::SendVideoBufferData(cur_frame_id, i, Program::mysocket, frame.buff[i], frame.buff.size(),rts, set_width);
					frame.buff[i].array = nullptr;
					if(cur_frame_id > 2147483630) {
						cur_frame_id = 0;
					}
				}
				cur_frame_id++;
			}
		} else {

			Form::mainFrame->CallAfter([]() {
				Form::mainFrame->GetVideo_Button()->SetLabelText(wxString::Format("%s",  "Connect Video"));
			});
			video_active = false;
		}
		if(Program::fd != -1)
			V4L2Manager::Stop(Program::fd);
		Form::mainFrame->CallAfter([]() {
			Form::mainFrame->GetVideo_Button()->Enable(true);
		});

	} else {
		//listen
NotDoneYet:
		int start_timer = 0;

		while(video_active == true ) {

			if(video_active == false )
				break;
			
			std::vector<std::packaged_task<void()>> _temp;
			std::unique_lock<std::mutex> lock(ActionsOnUserVideoThread_mutex);
			while(ActionsOnUserVideoThread.empty() == false && video_active == true) {
				_temp.push_back(std::move(ActionsOnUserVideoThread.front()));
				ActionsOnUserVideoThread.pop_front();
			}
			lock.unlock();
			for(int i =0 ; i < _temp.size(); ++i) {
				if(video_active == false)
					break;
				(_temp[i])();
			}

			if(video_buffer.size() <6 ||(int)video_buffer[0].required_length != (int)video_buffer[0].buff.size() /*if frame has been fully assembled*/)
				continue;

			start_timer ++;
			for(unsigned int i = 0; i < video_buffer[0].buff.size(); ++i) {
				for(unsigned int ii = i; ii > 0; --ii) {
					if(video_buffer[0].buff[ii].index > video_buffer[0].buff[i].index) {
						goto ESCAPE;
					}
				}
			}
			if(video_buffer.size() > 14) { //if too much of a delay, then reset
				video_buffer.erase(video_buffer.begin(), video_buffer.end() - 2);
			}
ESCAPE:
			fragmented_frame frame = video_buffer[0];
			boost_sharedptr_bytes b = V4L2Manager::AssembleFrame(frame);
			if(b.array == nullptr) {
				video_buffer.erase(video_buffer.begin());
				continue;
			}
			unsigned int out_width = 0;
			unsigned int out_height = 0; // just in case the supposed size and decompressed size are different
			bool shallrescale = frame.width > 450 && Program::Screen_Video_Elements.size() >= 2; // this would scale it in half
			std::shared_ptr<unsigned char[]> display = V4L2Manager::DecompressJPEG( b.array,  b.length,&out_width,&out_height, shallrescale);

			if(out_width != 0 && out_height != 0 ) {
				if(video_width != out_width || video_height != out_height) {
					Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this,out_width,out_height]() // Video element layout
					{
						for(unsigned int i = 0; i < Program::Screen_Video_Elements.size(); ++i) {
							if(Program::Screen_Video_Elements[i].source_ptr == vid) {
								Program::Screen_Video_Elements[i].width = out_width;
							}
						}
						//Find Pos Y
						int ylead = 0;
						for(unsigned int i = 0; i < (Program::Screen_Video_Elements.size() > 2 ? 2 : Program::Screen_Video_Elements.size()); ++i) {
							if((int)Program::Screen_Video_Elements[i].height > ylead)
								ylead = (int)Program::Screen_Video_Elements[i].height;
						}

						int posy = Program::Screen_Video_Elements.size() < 2 ? 65 : (65+ylead);
						int posx = Program::Screen_Video_Elements.size() < 2 ? (Program::Screen_Video_Elements.size() == 0 ? 530 : (530+Program::Screen_Video_Elements[0].width)) : (Program::Screen_Video_Elements.size() == 2 ? 530 : (530+Program::Screen_Video_Elements[2].width));

						if(vid == nullptr) {
							Form::mainFrame->CallAfter([this,out_width,out_height,posx,posy]() {
								wxStaticBitmap* m_loaded = new wxStaticBitmap(Form::mainFrame, wxID_ANY, wxNullBitmap, wxPoint(posx,posy), wxDLG_UNIT(Form::mainFrame, wxSize(0,0)), 0 );
								m_loaded->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
								vid = m_loaded;
								void* black = {};
								vid->SetBitmap(wxImage(out_width,out_height,black));
								Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this,out_width,out_height]()
								{
									Program::Screen_Video_Elements.push_back(VidElements{vid,out_width, out_height});
									RefreshCamLayout();
								}
								                                                                });
							});
						} else {
							RefreshCamLayout();
						}
					}
					                                                                });
				}
				if(vid != nullptr) {
					wxImage img = wxImage( wxSize(out_width, out_height), display.get(), wxBITMAP_TYPE_JPEG);
					Form::mainFrame->CallAfter([img,this,out_width, out_height]() {
						vid->SetBitmap(img);
					});
				}
			}
			video_width = out_width;
			video_height = out_height;

			VideoTimeStamp = frame.timestamp;
			video_buffer[0].buff.clear();
			video_buffer.erase(video_buffer.begin());
		}
		ClearVideoBuff();
		Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this]()
		{
			for(unsigned int i =0 ; i < Program::Screen_Video_Elements.size(); ++i) {
				if(Program::Screen_Video_Elements[i].source_ptr == vid) {
					Program::Screen_Video_Elements.erase(Program::Screen_Video_Elements.begin() + i);
				}
			}
			if(vid != nullptr) {
				Form::mainFrame->CallAfter([this]() {
					vid->SetBitmap(wxNullBitmap);
					delete vid;
					vid = nullptr;
				});
			}
			RefreshCamLayout();
		}
		                                                                });
		if(video_active == true)
			goto NotDoneYet;
	}
	video_width = video_height = 0;
	video_active = false;
	MemoryManagement::Delete_Thread(std::move(video_thread));
	video_thread = nullptr;
	VideoTimeStamp = 0;
	DifferenceAverage = 0;
	if(voice_active == true) {
		TaskTo_UserVoiceThread( std::packaged_task<void()> { [this]()
		{
			if(voice_buffer.size() > 1) {
				audio_buffer_data add_back_in = voice_buffer.front();
				voice_buffer.clear();
				voice_buffer.push_back(add_back_in);
			}
		}
		                                                   });
	}
}

void User::StartVideo()
{
	if(video_active == true)
		return;
	ClearVideoBuff();
	video_active = true;
	video_thread = new std::thread(&User::Video, this);
	MemoryManagement::Add_Obj(video_thread);
}

void User::StopVideo()
{
	video_active = false;
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

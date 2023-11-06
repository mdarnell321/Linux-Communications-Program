# Linux Communications Program

-This project was created in CodeLite.\
-Modify "Program::HostAddress" in the client to change IP address of the server (default is "127.0.0.1").

### <ins>The Server</ins>
    
-Rooms have their own TCP and UDP sockets.\
&emsp;-Room TCP socket is used for file streaming.\
&emsp;-Room UDP socket is used for video and audio streaming.\
&emsp;-Designated threads for carrying out room UDP and TCP tasks.

-Text messages are sent through the server's TCP socket.

### <ins>Program Features</ins>

__Rooms__\
 &emsp;-Create and join rooms of up to 4 people.\
 &emsp;-Rooms are created and joined by entering a name.\
 &emsp;-Ability to disconnect from a room, and join another one.

__Voice Chat (UDP)__\
&emsp;-Ability to start and stop audio stream.

__Video Chat (UDP)__\
&emsp;-The data of a frame is split up into chunks so that it can be quickly and properly sent to the server.\
&emsp;-Ability to start and stop video stream.\
&emsp;-Stream up to 700 pixels in width at 30 FPS (FPS can manually be increased in "User::Video" of the client).\
&emsp;-4 users may stream at the same time.\
&emsp;-Audio is synced up with video.

__Text Chat (TCP)__

__File Streaming (TCP)__\
&emsp;-A user can initiatiate a file stream and other users have 10 seconds to decide if they want to receive this file.\
&emsp;-Files may not exceed 100 GB.

### <ins>Install Dependencies</ins>

1.)\
sudo apt-get install libasound2-dev\
sudo apt-get install libjack-jackd2-dev\
sudo apt-get install oss4-dev\
sudo apt-get install libpipewire-0.3-dev\
sudo apt-get install libpulse-dev\
sudo apt-get install libopenal-dev\
sudo apt-get install codelite\
sudo apt-get install codelite-plugins\
sudo apt-get install libgtk2.0-dev

2.)*Install wxWidgets (version 3.0.5 was used)*\
&emsp;1.)https://www.wxwidgets.org/downloads/ \
&emsp;2.)Extract zip\
&emsp;3.)Open directory in terminal\
&emsp;&emsp;1.)./configure\
&emsp;&emsp;2.)make\
&emsp;&emsp;3.)make install

3.)\
sudo apt-get install libwxgtk3.0-gtk3-dev\
sudo apt-get install libjpeg-dev\
sudo apt-get upgrade

### <ins>Credits</ins>

-CodeLite\
-OpenAL\
-Video4Linux2\
-libjpeg\
-wxWidgets\
-Boost

### <ins>License</ins>

[MIT](https://github.com/mdarnell123/Linux-Communications-Program/blob/master/LICENSE.md)


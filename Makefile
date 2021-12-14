CPPFLAGS =  -std=c++17

all:
	g++ -Ilib/dr_wav $(CPPFLAGS) wav2h.cpp -o wav2h
setup:
	./libcamera_setup
app:
	clear
	g++ -o camera.elf camera.cpp -ltiff -lcamera -lboost_program_options -lcamera_app -I/usr/include/libcamera-apps -I/usr/lib/arm-linux-gnueabihf -std=c++17

#  -std=c++17
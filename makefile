setup:
	apt-get install gcc libcamera-dev libtiff-dev libboost-dev
app:
	clear
	g++ -o camera.elf camera.cpp -ltiff -lcamera -lboost_program_options

#  -std=c++17
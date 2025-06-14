#include <stdio.h>
#include "camera.h"

int main(int arg_count, char* arg_values[])
{
	camera cam(formats::SRGGB10,2592,1944);
	unsigned int *buffer;
	cam.GetFrame(buffer);
	return 0;
}

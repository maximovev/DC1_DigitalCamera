// RAW_Converter.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>

#include "mrf_support.h"

using namespace maxssau;

void FillData(unsigned char* data, int size)
{
    uint8_t counter = 0;
    for (int i = 0; i < size; i++)
    {
        data[i] = counter;
        counter++;
    }
}

int main()
{
    int image_height = 1000;
    int image_width = 1000;
    int image_array_size = image_height * image_width * 2;

    unsigned char* raw_data = (unsigned char*)malloc(image_array_size);

    FillData(raw_data, image_array_size);

    mrf_array<uint8_t> file_data(raw_data, image_array_size);

    std::cout << "Hello World!\n";
}

namespace maxxsau
{
	#include <tiffio.h>

	#include "enums.h"

	int saveRawToTiff16(const std::string &filename, const uint16_t *data, size_t size, uint32_t width, uint32_t height)
	{
		int result=STATUS_OK;

		TIFF *tif = TIFFOpen(filename.c_str(), "w");
		if (!tif) 
		{
			return STATUS_FAIL;
		}

		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

		if (TIFFWriteRawStrip(tif, 0, const_cast<uint16_t*>(data), size) == -1) {
			result=STATUS_FAIL;
		}

		TIFFClose(tif);
		return result;
	}
}
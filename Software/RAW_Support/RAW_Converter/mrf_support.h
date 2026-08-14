#ifndef __mrf_support__
#define __mrf_support__

/*	
*	RAW file format support for handmade camera based on esp32-p4-wifi6-touch-lcd-3.5
*	Maxssau RAW File (MRF)
* 
*	Maximov Evgeny aka maxssau
*	Russia, Samara 2026
*	9890175@mail.ru
* 
*/

#include <stdint.h>
#include "bit_action.h"
#include "crc.h"

namespace maxssau
{
	#define MRF_MAJOR_VERSION			1
	#define MRF_MINOR_VERSION			0
	#define MRF_BUILD_VERSION			1
	#define MRF_RELEASE					0
	#define MRF_METADATA_SECTION_COUNT	16
	#define MRF_STRING_SIZE				64

	const uint8_t mrf_default_signature[] = { 'M','R','F','\0' };

	enum MRF_SENSOR_TYPE : uint16_t
	{
		MRF_SENSOR_NONE
		,MRF_SENSOR_OV5647
		,MRF_SENSOR_COUNT
	};

	enum MRF_METRING_MODE: uint8_t
	{
		MRF_METRING_MODE_NONE
		,MRF_METRING_MODE_MATRIX
		,MRF_METRING_MODE_CENTER
		,MRF_METRING_MODE_SPOT
		,MRF_METRING_MODE_FACES
		,MRF_METRING_MODE_SKY
		,MRF_METRING_MODE_HIGH_LIGHTS
		,MRF_METRING_MODE_LOW_LIGHTS
		,MRF_METRING_MODE_COUNT
	};

	enum MRF_RAW_PIXEL_FORMAT : uint8_t
	{
		MRF_PIXEL_0_BIT
		,MRF_PIXEL_8_BIT
		,MRF_PIXEL_10_BIT
		,MRF_PIXEL_12_BIT
		,MRF_PIXEL_14_BIT
		,MRF_PIXEL_16_BIT
		,MRF_PIXEL_COUNT
	};

	enum MRF_RAW_BAYER_PATTERN : uint8_t
	{
		MRF_PATTERN_NONE
		,MRF_PATTERN_RGGB
		,MRF_PATTERN_BGGR
		,MRF_PATTERN_GRBG
		,MRF_PATTERN_GBRG
		,MRF_PATTERN_COUNT
	};

	enum MRF_DATA_COMPRESSION : uint8_t
	{
		MRF_COMPRESSION_NONE
		,MRF_COMPRESSION_ZIP
	};

	template <typename DataType>class mrf_array
	{
	public:
		/*	
			Simple template array class
		*/

		mrf_array()
		{
			null_pointer = true;
		};

		void Init(DataType* pointer, uint64_t size)
		{
			data_pointer = pointer;
			array_size = size;
			null_pointer = (pointer == nullptr);
		};

		mrf_array(DataType* pointer, uint64_t size)
			: data_pointer(pointer), array_size(size), null_pointer(pointer == nullptr)
		{
		};

		bool SetElement(DataType value, uint64_t index)
		{
			bool Result = false;
			if (!GetNullStatus())
			{
				if (index < array_size)
				{
					data_pointer[index] = value;
					Result = true;
				}
			}
			return Result;
		}

		DataType* GetPointer()
		{
			return data_pointer;
		}

		DataType GetElement(uint64_t index)
		{
			DataType Result = DataType{};
			if(!GetNullStatus())
			{
				if (index < array_size)
				{
					Result = data_pointer[index];
				}
			}
			return Result;
		}

		bool GetNullStatus() const
		{
			return					null_pointer;
		}

		uint64_t GetSize()
		{
			return array_size;
		}

	private:

		bool						null_pointer;
		DataType					*data_pointer;
		uint64_t					array_size;
	};

#pragma pack(push, 1)

	typedef struct
	{
		/*
			max size 65535x65535 pixels
		*/

		MRF_SENSOR_TYPE				sensor_type;
		MRF_RAW_PIXEL_FORMAT		pixel_format;
		MRF_RAW_BAYER_PATTERN		Bayer_pattern;
		MRF_DATA_COMPRESSION		compression_type;
		uint16_t					image_width;
		uint16_t					image_height;
		uint64_t					array_size;
		uint64_t					raw_offset;
		uint32_t					crc;
	} mrf_raw_info;

	typedef struct
	{
		uint32_t					iso;
		int32_t						shutter_speed;
		char						camera_vendor[MRF_STRING_SIZE];
		char						camera_model[MRF_STRING_SIZE];
		MRF_METRING_MODE			metring_mode;
	} mrf_exif_info;

	typedef struct
	{
		uint8_t						count;
		uint16_t					type;
		uint64_t					offset;
		uint32_t					crc;
	} mrf_metadata;

	typedef struct
	{
		uint16_t					image_width;
		uint16_t					image_height;
		uint64_t					array_size;
		uint64_t					jpeg_offset;
		uint32_t					crc;
	} mrf_jpeg_info;

	typedef struct
	{
		uint16_t					major;
		uint16_t					minor;
		uint16_t					build;
		uint8_t						is_release;
	} mrf_file_version;

	typedef struct
	{
		uint8_t						signature[4] = {'M','R','F','\0'};
		mrf_file_version			version;
		mrf_raw_info				raw_info;
		mrf_jpeg_info				jpg_info;
		uint8_t						have_raw;
		uint8_t						have_jpeg;
		mrf_metadata				metadata[MRF_METADATA_SECTION_COUNT];
		mrf_exif_info				exif_info;
	} mrf_container;

	enum class MRF_ErrorCode : uint8_t
	{
		FILE_NULL
		,INVALID_SIGNATURE
		,UNSUPPORTED_VERSION
		,CRC_RAW_ERROR
		,CRC_JPEG_ERROR
		,COUNT
	};

	static const char* MRF_ErrorMessages[] =
	{
		"File Null"                  // OK
		,"Invalid signature"      // FILE_NULL
		,"Unsupported file version" // FILE_TOO_SMALL
		,"CRC RAW Error"
		,"CRC Jpeg Error"
	};

	static_assert(sizeof(MRF_ErrorMessages) / sizeof(MRF_ErrorMessages[0]) == static_cast<size_t>(MRF_ErrorCode::COUNT),
		"Error messages array size does not match error codes count");

	typedef union
	{
		mrf_container				data;
		uint8_t						bytes[sizeof(mrf_container)];
	} mrf_header;

	class MRF_File
	{
	public:
		MRF_File(mrf_array<uint8_t>* file_data)
		{
			file = file_data;
			file_valid=CheckHeader();
		};

		~MRF_File()
		{
			return;
		};

		bool CheckHeader()
		{
			bool result = false;
			errors_code = 0;
			if (file != nullptr)
			{
				file_header = reinterpret_cast<mrf_header*>(file->GetPointer());
				// check for null and minimum header size
				if (file->GetSize() >= sizeof(mrf_container))
				{
					
					SETBIT(errors_code, 0, true);
					SETBIT(errors_code, 1, CheckSignature());
					SETBIT(errors_code, 2, CheckFileVersion());
					
					if (file_header->data.have_raw)
					{
						// check CRC RAW section
					}
					else
					{
						SETBIT(errors_code, 3, true);
					}

					if (file_header->data.have_jpeg)
					{
						// check CRC JPEG section
						SETBIT(errors_code, 4, true);
					}

					for (int i = 0; i < MRF_METADATA_SECTION_COUNT; i++)
					{
						// check CRC metadata section


					}
				}
			}
			return result;
		}

		bool Generate_RAW_File(mrf_array<uint8_t>*output_buffer, mrf_container *container, mrf_array<uint8_t> *raw_data, mrf_array<uint8_t> *jpeg_data, mrf_metadata *metadata, uint8_t metadata_count)
		{
			// check for null

			if (container != nullptr)
			{
				unsigned int data_size = 0;
				unsigned int container_size = sizeof(mrf_container);
				unsigned int raw_data_size = 0;
				unsigned int jpeg_data_size = 0;

				if (raw_data != nullptr)
				{
					raw_data_size = raw_data->GetSize();
				}
				if (jpeg_data != nullptr)
				{
					jpeg_data_size = jpeg_data->GetSize();
				}

				if ((metadata != nullptr) && (metadata_count < MRF_METADATA_SECTION_COUNT))
				{
					for (int i = 0; i < metadata_count; i++)
					{
						data_size = data_size + metadata[i].count;
					}
				}

				data_size = data_size + raw_data_size + jpeg_data_size;


			}
		}

		uint64_t GetMaxHeaderBufferSize()
		{
			uint64_t	result = sizeof(mrf_container);
			return result;
		}

	private:
		mrf_array<uint8_t>* file;
		bool				file_valid;
		uint32_t			errors_code;
		mrf_header			*file_header;

		bool CheckFileVersion()
		{
			bool result = false;

			if (file_header->data.version.major <= MRF_MAJOR_VERSION)
			{
				if (file_header->data.version.minor <= MRF_MINOR_VERSION)
				{
					if (file_header->data.version.build <= MRF_BUILD_VERSION)
					{
						result = true;
					}
				}
			}

			return result;
		}

		bool CheckSignature()
		{
			bool result = false;
			int counter = 0;
			for (int i = 0; i < sizeof(mrf_default_signature); i++)
			{
				if (file_header->data.signature[i] == mrf_default_signature[i])
				{
					counter++;
				}
			}

			if (counter == sizeof(mrf_default_signature))
			{
				result = true;
			}

			return result;
		}

	};

#pragma pack(pop)

}

#endif
#ifndef __camera__
#define __camera__

using namespace libcamera;

namespace maxssau
{

	#include <iostream>
	#include <fstream>
	#include <vector>
	#include <libcamera/libcamera.h>

	enum RPiCamera_Status
	{
		RPiCamera_OK=0,
		RPiCamera_FAIL=1,
		RPiCamera_NO_CAMERA=2,
		RPiCamera_CAMERA_ERROR=3,
		RPiCamera_CAMERA_CONFIG_ERROR1=4,
		RPiCamera_CAMERA_CONFIG_ERROR2=5,
		RPiCamera_CAMERA_CONFIG_ERROR3=6,
		RPiCamera_CAMERA_CONFIG_ERROR4=7,
		RPiCamera_CAMERA_CONFIG_ERROR5=8,
		RPiCamera_REQUEST_ERROR=9,
		RPiCamera_BUFFER_ERROR=10,
		RPiCamera_START_ERROR=11

	}

	class RPiCamera
	{
		public:

			int Status_Init;

			RPiCamera(PixelFormat pixel_format,int image_height, int image_width)
			{
				cm = std::make_unique<CameraManager>();
				cm->start();
				 if (cm->cameras().empty())
				 {
					Status_Init=RPiCamera_NO_CAMERA;
					return;
				 }

				camera = cm->cameras()[0];
    			if (camera->acquire())
				{
        			Status_Init=RPiCamera_CAMERA_ERROR;
        			return;
    			}

    			std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::Raw});
    			if (!config) 
				{
					Status_Init=RPiCamera_CAMERA_CONFIG_ERROR1;
        			return;
    			}
				StreamConfiguration &streamConfig = config->at(0);
    			streamConfig.pixelFormat = pixel_format;
    			streamConfig.size = {image_height, image_width}; // Размеры сенсора
    			streamConfig.bufferCount = 1;

				if (config->validate() == CameraConfiguration::Invalid) 
				{
					Status_Init=RPiCamera_CAMERA_CONFIG_ERROR2;
					return;
    			}

				if (camera->configure(config.get()) < 0) 
				{
					Status_Init=RPiCamera_CAMERA_CONFIG_ERROR3;
					return;
    			}

    			allocator = new FrameBufferAllocator(camera);
    			for (StreamConfiguration &cfg : *config) 
				{
        			if (allocator->allocate(cfg.stream()) < 0) 
					{
						Status_Init=RPiCamera_CAMERA_CONFIG_ERROR4;
						return;
        			}
    			}

				if (camera->configure(config.get()) < 0) 
				{
					Status_Init=RPiCamera_CAMERA_CONFIG_ERROR5;
					return;
				}

				Stream *stream = streamConfig.stream();
    			const std::vector<std::unique_ptr<FrameBuffer>>&buffers = allocator->buffers(stream);
    			std::vector<Request *> requests;

    			for (unsigned int i = 0; i < buffers.size(); ++i) 
				{
        			Request *request = camera->createRequest();
        			if (!request) 
					{
            			Status_Init=RPiCamera_REQUEST_ERROR;
            			return;
        			}

        			if (request->addBuffer(stream, buffers[i].get()) < 0) 
					{
						Status_Init=RPiCamera_BUFFER_ERROR;
						return;
        			}

        			requests.push_back(request);
    			}

				if (camera->start()) 
				{
					Status_Init=RPiCamera_START_ERROR;
					return;
				}
			}

			int GetFrame(uint8_t *rawData)
			{
				Request *request = requests[0];
				camera->queueRequest(request);
				std::unique_ptr<Request> completedRequest;
				completedRequest.reset(camera->waitForCompletedRequest());
    			if (!completedRequest)
				{
        			return RPiCamera_FAIL;
    			}
				const FrameBuffer *buffer = completedRequest->findBuffer(stream);
    			if (!buffer)
				{
        			return RPiCamera_BUFFER_ERROR;
    			}

    			const FrameBuffer::Plane &plane = buffer->planes()[0];
    			rawData = static_cast<const uint8_t*>(mmap(NULL, plane.length, PROT_READ, MAP_SHARED, plane.fd.fd(), 0));
				if (rawData == MAP_FAILED)
				{
        			return RPiCamera_FAIL;
    			}
			}

			~RPiCamera()
			{
				munmap(const_cast<uint8_t*>(rawData), plane.length);
    			camera->stop();
    			allocator->free(stream);
    			delete allocator;
    			camera->release();
    			cm->stop();
			}

		private:
			std::unique_ptr<CameraManager> cm;
			std::shared_ptr<Camera> camera;
			FrameBufferAllocator *allocator;
	}

	

}

#endif
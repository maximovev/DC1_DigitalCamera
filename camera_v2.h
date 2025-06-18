#ifndef __camera__
#define __camera__

#include <iostream>
#include <fstream>
#include <vector>
#include <libcamera/libcamera.h>
#include "enums.h"

namespace maxssau
{
	

	using namespace libcamera;
	
	static std::shared_ptr<Camera> camera;

	static void requestComplete(Request *request)
	{
		if (request->status() == Request::RequestCancelled)
		{
			return;
		}
		const Request::BufferMap &buffers = request->buffers();
		for (auto bufferPair : buffers)
		{
			FrameBuffer *buffer = bufferPair.second;
			const FrameMetadata &metadata = buffer->metadata();
			request->reuse(Request::ReuseBuffers);
			//camera->queueRequest(request);
		}
	}

	class rpi_camera
	{
		public:
			int status;
			rpi_camera()
			{
				cm = std::make_unique<CameraManager>();
				cm->start();
				if(cm->cameras().empty())
				{
					status=STATUS_FAIL;
					return;
				}
				cameraId = cm->cameras()[0]->id();
				camera = cm->get(cameraId);
				camera->acquire();
				streamConfig=config->at(0);

				config=camera->generateConfiguration( { StreamRole::Viewfinder } );

				streamConfig.size.width=0;
				streamConfig.size.height=0;

				int ret=camera->configure(config.get());
				if(ret)
				{
					status=STATUS_FAIL;
					return;
				};

				config->validate();

				allocator = new FrameBufferAllocator(camera);

				for (StreamConfiguration &cfg : *config) 
				{
					int ret = allocator->allocate(cfg.stream());
					if (ret < 0) 
					{
						status=STATUS_FAIL;
						return;
					}
					size_t allocated = allocator->buffers(cfg.stream()).size();
				}
				stream = streamConfig.stream();

				const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
				std::vector<std::unique_ptr<Request>> requests;
				
				for (unsigned int i = 0; i < buffers.size(); ++i) 
				{
					std::unique_ptr<Request> request = camera->createRequest();
					if (!request)
					{
						status=STATUS_FAIL;
						return;
					}

					const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
					int ret = request->addBuffer(stream, buffer.get());
					if (ret < 0)
					{
						status=STATUS_FAIL;
						return;
					}

					ControlList &controls = request->controls();
					controls.set(controls::Brightness, 0.5);
					requests.push_back(std::move(request));
					camera->requestCompleted.connect(requestComplete);
				}
			};

			~rpi_camera()
			{
				camera->stop();
				allocator->free(stream);
				delete allocator;
				camera->release();
				camera.reset();
				cm->stop();
			};

		private:
			std::unique_ptr<CameraManager> cm;
			std::string cameraId;
			std::unique_ptr<CameraConfiguration> config;
			StreamConfiguration streamConfig;
			FrameBufferAllocator *allocator;
			Stream *stream;

			unsigned int camera_count;
	};

}

#endif
#ifndef __camera__
#define __camera__

using namespace libcamera;

namespace maxssau
{

	#include <libcamera/libcamera.h>

	enum RPiCamera_Status
	{
		RPiCamera_OK=0,
		RPiCamera_FAIL=1,
		RPiCamera_NO_CAMERA=2
	}

	class RPiCamera
	{
		public:

			int Status_Init;

			RPiCamera()
			{
				cm = std::make_unique<CameraManager>();
				cm->start();
				 if (cm->cameras().empty())
				 {
					Status_Init=RPiCamera_NO_CAMERA;
				 }

			}

			~RPiCamera()
			{

			}

		private:
			std::unique_ptr<CameraManager> cm;
			
	}

	

}

#endif
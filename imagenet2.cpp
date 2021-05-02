#include "videoSource.h"
#include "videoOutput.h"

#include "cudaFont.h"
#include "imageNet.h"

#include <signal.h>

#include <jetson-utils/imageIO.h>//same as #include <jetson-utils/loadImage.h>

#ifdef HEADLESS
	#define IS_HEADLESS() "headless"	// run without display
#else
	#define IS_HEADLESS() (const char*)NULL
#endif


bool signal_recieved = false;

void sig_handler(int signo)
{
	if( signo == SIGINT )
	{
		LogVerbose("received SIGINT\n");
		signal_recieved = true;
	}
}

int usage()
{
	printf("usage: imagenet [--help] [--network=NETWORK] ...\n");
	printf("                input_URI [output_URI]\n\n");
	printf("Classify a video/image stream using an image recognition DNN.\n");
	printf("See below for additional arguments that may not be shown above.\n\n");
	printf("positional arguments:\n");
	printf("    input_URI       resource URI of input stream  (see videoSource below)\n");
	printf("    output_URI      resource URI of output stream (see videoOutput below)\n\n");

	printf("%s", imageNet::Usage());
	printf("%s", videoSource::Usage());
	printf("%s", videoOutput::Usage());
	printf("%s", Log::Usage());

	return 0;
}

int main( int argc, char** argv )
{
	/*
	 * parse command line
	 */
    printf("======== OLLE WELIN Test 1 ============");
	commandLine cmdLine(argc, argv, IS_HEADLESS());
	if( cmdLine.GetFlag("help") )
		return usage();


	/*
	 * attach signal handler
	 */
	if( signal(SIGINT, sig_handler) == SIG_ERR )
		LogError("can't catch SIGINT\n");


	/*
	 * create input stream
	 */
	videoSource* input = videoSource::Create(cmdLine, ARG_POSITION(0));

	if( !input )
	{
		LogError("imagenet:  failed to create input stream\n");
		return 0;
	}


	/*
	 * create output stream
	 */
	videoOutput* output = videoOutput::Create(cmdLine, ARG_POSITION(1));

	if( !output )
		LogError("imagenet:  failed to create output stream\n");


	/*
	 * create font for image overlay
	 */
	cudaFont* font = cudaFont::Create();

	if( !font )
	{
		LogError("imagenet:  failed to load font for overlay\n");
		return 0;
	}


	/*
	 * create recognition network
	 */
	imageNet* net = imageNet::Create(cmdLine);

	if( !net )
	{
		LogError("imagenet:  failed to initialize imageNet\n");
		return 0;
	}


	/*
	 * processing loop
	 */


    const int record_frames_min = 10;
    int record_frame = 0;
    const float thresshold = 0.3f;
    int folder_nr = 0;
    int picture_nr = 0;

	while( !signal_recieved )
	{
		// capture next image image
		uchar3* image = NULL;

		if( !input->Capture(&image, 1000) )
		{
			// check for EOS
			if( !input->IsStreaming() )
				break;

			LogError("imagenet:  failed to capture next frame\n");
			continue;
		}

		// classify image
		float confidence = 0.0f;
		const int img_class = net->Classify(image, input->GetWidth(), input->GetHeight(), &confidence);

		if( img_class >= 0 )
		{
			LogVerbose("imagenet:  %2.5f%% class #%i (%s)\n", confidence * 100.0f, img_class, net->GetClassDesc(img_class));

			if( font != NULL )
			{
				char str[256];
				sprintf(str, "%05.2f%% %s", confidence * 100.0f, net->GetClassDesc(img_class));

				font->OverlayText(image, input->GetWidth(), input->GetHeight(),
						        str, 5, 5, make_float4(255, 255, 255, 255), make_float4(0, 0, 0, 100));

                printf("Olle print %s\n", str);

                if((img_class == 1 && confidence > thresshold) || record_frame > 0 && record_frame < record_frames_min)
                {
                    if(record_frame==0){
                        folder_nr++;
                    }
                    printf("Confidence = %f record frame =%d\n", confidence, record_frame);

                    printf("store images\n");
                    saveImage("../../../../../media/jetson/1TB-USB/store_img.jpg", image, input->GetWidth(), input->GetHeight());
                    record_frame++;
                }
                else{
                    record_frame = 0;
                }



  			}
		}

		// render outputs
		if( output != NULL )
		{
			output->Render(image, input->GetWidth(), input->GetHeight());

			// update status bar
			char str[256];
			sprintf(str, "TensorRT %i.%i.%i | %s | Network %.0f FPS", NV_TENSORRT_MAJOR, NV_TENSORRT_MINOR, NV_TENSORRT_PATCH, net->GetNetworkName(), net->GetNetworkFPS());
			output->SetStatus(str);


			// check if the user quit
			if( !output->IsStreaming() )
				signal_recieved = true;
		}

		// print out timing info
		net->PrintProfilerTimes();
	}


	/*
	 * destroy resources
	 */
	LogVerbose("imagenet:  shutting down...\n");

	SAFE_DELETE(input);
	SAFE_DELETE(output);
	SAFE_DELETE(net);

	LogVerbose("imagenet:  shutdown complete.\n");
	return 0;
}

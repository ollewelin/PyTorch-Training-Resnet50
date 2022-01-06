//******** Things for crop video stream etc *****************
#include <jetson-utils/cudaCrop.h>
#include <jetson-utils/cudaMappedMemory.h>
#include <jetson-utils/cudaColorspace.h>
#include <jetson-utils/imageIO.h>
//*************************************************************************

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


#include <iostream>
#include <fstream>
#include <sstream> // writing to memory (a string)
#include <string>

// C++ program to create a directory in Linux
#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>

// OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <vector>
using namespace std;

bool signal_recieved = false;

const int version_major = 0;
const int version_middle = 0;
const int version_minor = 1;

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
    cv::Mat opencv_frame;

  //  std::ostringstream oss; // open a string for writing EDIT: Fixed from original post
//    oss << "Writing to string in memory"; // output to string

    const int img_buff_nr = 10;//Store in RAM first before save on disk
    std::vector<uchar3> imgBuff;
    /*
     * parse command line
     */
    printf("======== Olle Welin, cat flap blocker ============\n");

    printf("======== Version : %d.%d.%d ============\n", version_major, version_middle, version_minor );

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

    //
    const int record_frames_min = 1;
    const int record_frames_max = 10000;
    int record_frame_class1 = 0;
    int record_frame_class2 = 0;

    int class_lock_cnt = 0;
    const int threshold_lock_class_cnt = 3;//1 = lock direct on one frame, 0 not allowed, 

    //const float thresshold_class_1 = 0.5f;
    //const float thresshold_class_2 = 1.9f;

    const float thresshold_class_1 = 4.0f;
    const float thresshold_class_2 = 4.0f;
    int folder_nr = 0;
    int picture_nr = 0;

    std::string usb_disk_path = "../../../../../media/jetson/USB1-8GB/";//USB1-8GB UBUNTU 18_0   1TB-USB
   //   std::string usb_disk_path = "./";//Store directly at SD card
    //   std::string usb_disk_path = "./data/";
    std::string pictures_store_path = "pic";
    std::string pictures_store_path_no_prey = "pic_cat_No_prey";
    std::string str_foldernr = "0";
    std::string str_slash = "/";
    std::string str_framenr = "0";
    std::string str_jpg = ".jpg";
    std::string foldername;

    std::string pic_file = usb_disk_path;

                            foldername = usb_disk_path + pictures_store_path;
                            // Creating a directory
                            char arr[foldername.length()+1];
                            strcpy(arr,foldername.c_str());
                            if (mkdir(arr, 0777) == -1)
                                cerr << "Error :  " << strerror(errno) << endl;
                            else
                                cout << "Directory created";
                            
                            foldername = usb_disk_path + pictures_store_path_no_prey;
                            // Creating a directory
                            arr[foldername.length()+1];
                            strcpy(arr,foldername.c_str());
                            if (mkdir(arr, 0777) == -1)
                                cerr << "Error :  " << strerror(errno) << endl;
                            else
                                cout << "Directory created";


    system("echo 79 > ../../../../../sys/class/gpio/export");
    system("echo out > ../../../../../sys/class/gpio/gpio79/direction");
    system("echo 0 > ../../../../../sys/class/gpio/gpio79/value");

    int pre_class = 0;

    if(!signal_recieved)
    {
        uchar3* image = NULL;
        int inputWidth = input->GetWidth();
        int inputHeight = input->GetHeight();
        const float crop_factor_x = 0.53;
        const float crop_factor_y = 0.95;

        const int2  crop_border = make_int2((1.0f - crop_factor_x) * 0.5f * inputWidth,
                                            (1.0f - crop_factor_y) * 0.5f * inputHeight);

// compute the ROI as (left, top, right, bottom)
        const int4 crop_roi = make_int4(crop_border.x, crop_border.y, inputWidth - crop_border.x, inputHeight - crop_border.y);

        printf("crop_roi.x=%d\n", crop_roi.x);
        printf("crop_roi.y=%d\n", crop_roi.y);
        printf("crop_roi.z=%d\n", crop_roi.z);
        printf("crop_roi.w=%d\n", crop_roi.w);
        int crop_width = crop_roi.z - crop_roi.x;
        int crop_height = crop_roi.w -crop_roi.y;
        printf("crop_width = %d\n", crop_width);
        printf("crop_height = %d\n", crop_height);

        while( !signal_recieved )
        {
            // capture next image image


            if( !input->Capture(&image, 1000) )
            {
                // check for EOS
                if( !input->IsStreaming() )
                    break;

                LogError("imagenet:  failed to capture next frame\n");
                continue;
            }
        

// crop the image
            if( CUDA_FAILED(cudaCrop(image, image, crop_roi, inputWidth, inputHeight)) )
                return false;
        void* img_gpu_src = NULL;//float3 = IMAGE_RGB32F
        if( !cudaAllocMapped(&img_gpu_src, crop_width, crop_height, IMAGE_RGB32F))
            return false;
        if( CUDA_FAILED(cudaConvertColor(image, IMAGE_RGB8, img_gpu_src, IMAGE_RGB32F, crop_width, crop_height)))
            return false;
        CUDA(cudaDeviceSynchronize());
      
        float3 *host_src;
        //Allocate host pointers
        host_src = (float3*)malloc(sizeof(float3)*crop_width*crop_height);
        cudaMemcpy(host_src, img_gpu_src, (sizeof(float3)*crop_width*crop_height), cudaMemcpyDeviceToHost);
      
        // copy over to Mat
        opencv_frame.create(crop_height, crop_width, CV_32FC3);
        float *opencv_f_zero_ptr = opencv_frame.ptr<float>(0);
        float *opencv_f_indx_ptr = opencv_frame.ptr<float>(0);
        float3 *img_gpu_src_zero_ptr = (float3*)&host_src[0];
        float3 *img_gpu_src_indx_ptr = (float3*)&host_src[0];
       int size_of_opencv_frame = opencv_frame.cols * opencv_frame.rows;
        for(int i = 0;i<size_of_opencv_frame ; i++){

            
            union{
                float3 fl3;
                float fl1[3];
            } union_data;
	    
	        img_gpu_src_indx_ptr = img_gpu_src_zero_ptr + i;
            union_data.fl3 = *img_gpu_src_indx_ptr;            
            for(int k=0;k<opencv_frame.channels();k++){
                opencv_f_indx_ptr = opencv_f_zero_ptr + i*opencv_frame.channels() + k;
                
                *opencv_f_indx_ptr = union_data.fl1[(k*(-1))+2];//(k*(-1))+2 will flip RGB to BRG, only k = no flip
            }
        }
        free(host_src);
        CUDA(cudaFreeHost(img_gpu_src));
        //printf("opencv_frame rows =%d\n", opencv_frame.rows);
        //printf("opencv_frame cols =%d\n", opencv_frame.cols);
        opencv_frame.convertTo(opencv_frame, CV_32FC3, 1.0/255.0);
     //   cv::imshow("opencv_frame", opencv_frame);
     //   cv::waitKey(1);
        opencv_frame.convertTo(opencv_frame, CV_8UC3, 255);



            // classify image
            float confidence = 0.0f;
            const int img_class = net->Classify(image, crop_width, crop_height, &confidence);

            if( img_class >= 0 )
            {
              //  LogVerbose("imagenet:  %2.5f%% class #%i (%s)\n", confidence * 10.0f, img_class, net->GetClassDesc(img_class));
                
                if( font != NULL )
                {
                    char str[256];
                    sprintf(str, "%05.2f%% %s", confidence * 10.0f, net->GetClassDesc(img_class));

                    font->OverlayText(image, crop_width, crop_height,
                                      str, 5, 5, make_float4(255, 255, 255, 255), make_float4(0, 0, 0, 100));

                       //printf("debug print %s\n", str);
 
                    if(img_class == 1 && confidence > thresshold_class_1)
                    {
                        if(class_lock_cnt < threshold_lock_class_cnt-1)
                        {
                            class_lock_cnt++;
                        }
                        else
                        {
                        printf("Set GPIO pin 79 High\n");
                        system("echo 1 > ../../../../../sys/class/gpio/gpio79/value");

                        str_framenr = std::to_string(record_frame_class1);
                        pic_file = usb_disk_path + pictures_store_path + str_slash + str_framenr + str_jpg;
                     //   printf("Confidence = %f record frame =%d\n", confidence, record_frame_class1);
                    //    printf("folder_nr = %d\n", folder_nr );
                    //    printf("store images\n");
                        // saveImage("../../../../../media/jetson/1TB-USB/store_img.jpg", image, crop_width, crop_height);

                        char arr[pic_file.length()+1];
                        strcpy(arr,pic_file.c_str());
                     //   saveImage(arr, image, crop_width, crop_height);
                        
                        cv::imwrite(arr, opencv_frame);
                        //cv::imwrite("Test.jpg", opencv_frame);
                        
                        //saveImage(oss, image, crop_width, crop_height);
                        if(record_frame_class1 < record_frames_max){
                            record_frame_class1++;   
                        }
                        else{
                            record_frame_class1 = 0;//circle buffert
                        }
                        
                        }
                    }
                    else
                    {
                    //    record_frame_class1++;
                        if(class_lock_cnt > 0)
                        {
                            class_lock_cnt--;
                        }
                        //class_lock_cnt = 0;
                    }
                    if(img_class == 2 && confidence > thresshold_class_2)
                    {
                        class_lock_cnt = 0;
                        //No prey
                        //pictures_store_path_no_prey
                     //   printf("Set GPIO pin 79 Low\n");
                        system("echo 0 > ../../../../../sys/class/gpio/gpio79/value");


                        str_framenr = std::to_string(record_frame_class2);
                        pic_file = usb_disk_path + pictures_store_path_no_prey + str_slash + str_framenr + str_jpg;
                      //  printf("Confidence = %f record frame =%d\n", confidence, record_frame_class1);
                      //  printf("folder_nr = %d\n", folder_nr );
                      //  printf("store images\n");
                        // saveImage("../../../../../media/jetson/1TB-USB/store_img.jpg", image, crop_width, crop_height);

                        char arr[pic_file.length()+1];
                        strcpy(arr,pic_file.c_str());
                        //saveImage(arr, image, crop_width, crop_height);
                        cv::imwrite(arr, opencv_frame);
                        if(record_frame_class2 < record_frames_max){
                            record_frame_class2++;   
                        }
                        else{
                            record_frame_class2 = 0;//circle buffert
                        }

                    }
                    else
                    {
                   //      printf("Set GPIO pin 79 Low\n");
                        system("echo 0 > ../../../../../sys/class/gpio/gpio79/value");
                        pre_class=0;
                    }





                }
            }

            // render outputs
            if( output != NULL )
            {
                output->Render(image, crop_width, crop_height);

                // update status bar
                char str[256];
                sprintf(str, "TensorRT %i.%i.%i | %s | Network %.0f FPS", NV_TENSORRT_MAJOR, NV_TENSORRT_MINOR, NV_TENSORRT_PATCH, net->GetNetworkName(), net->GetNetworkFPS());
                output->SetStatus(str);


                // check if the user quit
                if( !output->IsStreaming() )
                    signal_recieved = true;
            }

            // print out timing info
           // net->PrintProfilerTimes();
        }


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

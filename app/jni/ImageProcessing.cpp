#include "com_cabatuan_lazytypeconversion_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

#define  LOG_TAG    "LazyTypeConversion"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)



float t, t_sum = 0;
int counter = 0;

Mat srcBGR;


/*
 * Class:     com_cabatuan_lazytypeconversion_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_com_cabatuan_lazytypeconversion_MainActivity_process
  (JNIEnv *pEnv, jobject clazz, jobject pTarget, jbyteArray pSource){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent; // Links to Bitmap content

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   /// cv::Mat for YUV420sp source and output BGRA 
    Mat src(bitmapInfo.height + bitmapInfo.height/2, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);
    
    
/***********************************************************************************************/
/*                           Native Image Processing                                           */
/***********************************************************************************************/
 
    if(srcBGR.empty())
       srcBGR = Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC3);
    
    t = (float)getTickCount();
    
    // YUV420NV21 ---> BGR
    cvtColor(src, srcBGR, CV_YUV420sp2RGB);   
    
    // BGR ---> BGRA
    cvtColor(srcBGR, mbgra, CV_BGR2BGRA);
    
    t = 1000*((float)getTickCount() - t)/getTickFrequency();
    
    LOGI("Lazy type conversion took %0.2f ms.", t);
    
    
    // For average time
    
    t_sum += t;    
    counter++;
    
    if(counter % 10 == 0){ // Log average time every 10 frames
        LOGI("Average time for Lazy type conversion: %0.2f ms after %d frames", t_sum/counter, counter);
    }
 
/************************************************************************************************/ 
   
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}

#include "com_cabatuan_lazytypeconversion_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>
 

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

#define  LOG_TAG    "LazyTypeConversion"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#define toInt(pValue) \
 (0xff & (int32_t) pValue)

#define max(pValue1, pValue2) \
 (pValue1<pValue2) ? pValue2 : pValue1

#define clamp(pValue, pLowest, pHighest) \
 ((pValue < 0) ? pLowest : (pValue > pHighest) ? pHighest: pValue)

#define color(pColorR, pColorG, pColorB) \
           (0xFF000000 | ((pColorB << 6)  & 0x00FF0000) \
                       | ((pColorG >> 2)  & 0x0000FF00) \
                       | ((pColorR >> 10) & 0x000000FF))


/*

// Working!
void YUV2Bitmap( jbyte* source, AndroidBitmapInfo bitmapInfo, uint32_t* bitmapContent ){

   int32_t frameSize = bitmapInfo.width * bitmapInfo.height;
   int32_t yIndex, uvIndex, x, y;
   int32_t colorY, colorU, colorV;
   int32_t colorR, colorG, colorB;
   int32_t y1192;
 
   
   /// Convert YUV to RGB
   for (y = 0, yIndex = 0; y < bitmapInfo.height; ++y){
        colorU = 0; colorV = 0;

        uvIndex = frameSize + (y >> 1) * bitmapInfo.width;

        for(x = 0; x < bitmapInfo.width; ++x, ++yIndex ){
            
            colorY = max(toInt(source[yIndex]) - 16, 0);
            
            if(!(x % 2)){
                 colorV = toInt(source[uvIndex++]) -128;
                 colorU = toInt(source[uvIndex++]) -128;
            }

            /// Compute RGB from YUV
            y1192 = 1192 * colorY;
            colorR = (y1192 + 1634 * colorV);
            colorG = (y1192 - 833 * colorV - 400 * colorU);
            colorB = (y1192 + 2066 * colorU);

            colorR = clamp(colorR, 0, 262143);
            colorG = clamp(colorG, 0, 262143);
            colorB = clamp(colorB, 0, 262143);


            /// Combine R, G, B, and A into the final pixel color
 
            bitmapContent[yIndex] = color(colorR, colorG, colorB);
        }
    }
}
*/


// Input:  NV21  (CV_8UC1) 
// Output: RGBA  (CV_8UC4)
void YUV2Bitmap( const cv::Mat &src, AndroidBitmapInfo bitmapInfo, uint32_t* bitmapContent ){

    
      /// Decode raw video into output bitmap
   int32_t frameSize = bitmapInfo.width * bitmapInfo.height;
   int32_t yIndex, uvIndex, j, i;
   int32_t colorY, colorU, colorV;
   int32_t colorR, colorG, colorB;
   int32_t y1192;
    
    /// Convert to 1D array if Continuous
    //if (src.isContinuous()) {
     //   nCols = nCols * nRows;
	//	nRows = 1; // it is now a 1D array
	//}   

    for (j = 0, yIndex = 0; j < bitmapInfo.height; ++j) {
    
    	colorU = 0; colorV = 0;
    	
    	uvIndex = frameSize + (j >> 1) * bitmapInfo.width;
    	
        const char* data    = src.ptr<char>(j);  

		for (i = 0; i < bitmapInfo.width; ++i, ++yIndex) {
		     
		    colorY = max(toInt(data[yIndex]) - 16, 0);
		       
            if(!(i % 2)){
            
                colorV =  toInt(data[uvIndex++]) - 128;
                
                colorU =  toInt(data[uvIndex++]) - 128;
                
            }
            
            /// Compute RGB from YUV
            y1192 = 1192 * colorY;
            
            colorR = (y1192 + 1634 * colorV);
            colorG = (y1192 - 833 * colorV - 400 * colorU);
            colorB = (y1192 + 2066 * colorU);

            colorR = clamp(colorR, 0, 262143);
            colorG = clamp(colorG, 0, 262143);
            colorB = clamp(colorB, 0, 262143);


            /// Combine R, G, B, and A into the final pixel color 
            bitmapContent[yIndex] = color(colorR, colorG, colorB);
        }
    }
}





void GRAY2RGBA_NV21( const cv::Mat &src, cv::Mat &rgba ){

	int nRows = 2 * src.rows / 3;   // number of lines
    int nCols = src.cols;           // number of columns
    
    if(rgba.empty())
    	rgba.create( Size(nCols, nRows), CV_8UC4);  

    /// Convert to 1D array if Continuous
    if (src.isContinuous()) {
        nCols = nCols * nRows;
		nRows = 1; // it is now a 1D array
	}   

    for (int j=0; j<nRows; j++) {
    
        /// Pointer to start of the row // AVERAGE: 17 fps: FAST!
         const uchar* data   = src.ptr<uchar>(j);
         cv::Vec4b *rgba_row  = rgba.ptr<cv::Vec4b>(j);          // rgb data 

		for (int i = 0; i < nCols; i++) {
		     rgba_row[i] = cv::Vec4b( 0, 0, *data++, 255);
             //rgba_row[i][0] = 0;  // Red
             //rgba_row[i][1] = 0;        // Green 
             //rgba_row[i][2] = *data++;        // Blue
             //rgba_row[i][3] = 255;      // Opacity           
        }
    }
}







//float t, t_sum = 0;
//int counter = 0;

//Mat srcBGR;

Mat white;

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
    Mat src(bitmapInfo.height + bitmapInfo.height/2, bitmapInfo.width, CV_8SC1, (char *)source);
   //Mat src(bitmapInfo.height + bitmapInfo.height/2, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
   
   
    //Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);
    
   // AVERAGE: 17 fps: FAST!
   //YUV2Bitmap( source, bitmapInfo, bitmapContent); //works
   
   YUV2Bitmap( src, bitmapInfo, bitmapContent);
    
/***********************************************************************************************/
/*
    if (white.empty()){
        white = Mat::zeros(mbgra.size(), CV_8UC1) + 255;  // opaque
    }

    // AVERAGE: 16.8 fps after 500 frames // Simpler approach
    Mat channel[4] = {white, src.rowRange( 0, bitmapInfo.height), white, white}; 
    
    // merge
    merge(channel, 4, mbgra);    
    
/***********************************************************************************************/



    
    
/***********************************************************************************************/
/*                           Native Image Processing                                           */
/***********************************************************************************************/
 /*
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
 */
/************************************************************************************************/ 
   
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}

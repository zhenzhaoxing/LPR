//
// 加载车牌识别模型，对车牌进行定位，并添加到指定容器里
//
#include "stdafx.h"
#include "../include/Plate_location.h"
#include "../include/util.h"
using namespace std;
namespace lpr {

	//loadmodel
	PlateDetection::PlateDetection(string filename_cascade,string xml,string bin) {
		cascade.load(filename_cascade);
		net = cv::dnn::readNetFromModelOptimizer(xml, bin);
		net.setPreferableBackend(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
		net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);



	};
	int calcSafeRect(const  cv::Rect& plate, const cv::Mat& InputImage, cv::Rect_<float>& safeBoundRect)
	{
		float tl_x = plate.x > 0 ? plate.x-2 : 6;
		float tl_y = plate.y > 0 ? plate.y-2 : 6;
		float br_x = plate.x + plate.width < InputImage.cols? plate.x + plate.width - 1: InputImage.cols - 1;
		float br_y = plate.y + plate.height < InputImage.rows? plate.y + plate.height - 1: InputImage.rows - 1;
		float roi_width = br_x - tl_x+20;
		float roi_height = br_y - tl_y+20;
		if (roi_width <= 0 || roi_height <= 0) return false;
		//  a new rect not out the range of mat
		safeBoundRect = cv::Rect_<float>(tl_x, tl_y, roi_width, roi_height);
		return 0;
	}

	void PlateDetection::plateDetectionRough(cv::Mat InputImage, vector<lpr::PlateInfo>& plateInfos, int min_w, int max_w) {
		
		cv::Mat processImage;
		cvtColor(InputImage, processImage, cv::COLOR_BGR2GRAY);
		vector<cv::Rect> platesRegions;
		cv::Size minSize(min_w, min_w / 4);
		cv::Size maxSize(max_w, max_w / 4);
		//检测的函数
		
		cascade.detectMultiScale(processImage, platesRegions,
			1.1, 3,0,  minSize, maxSize);
		
		for (auto plate : platesRegions)
		{
		    /*
			int zeroadd_w = static_cast<int>(plate.width *0.22);
			int zeroadd_h = static_cast<int>(plate.height *1.2);
			int zeroadd_x = static_cast<int>(plate.width * 0.13);
			int zeroadd_y = static_cast<int>(plate.height * 0.75);
			
			// extend rects0.14 0.28 0.15 0.3
			plate.x -= plate.width * 0.14;
			plate.width += plate.width * 0.28;
			plate.y -= plate.height * 0.15;
			plate.height += plate.height * 0.3;
			*/
			int zeroadd_w = static_cast<int>(plate.width * 0.22);
			int zeroadd_h = static_cast<int>(plate.height * 1.5);
			int zeroadd_x = static_cast<int>(plate.width * 0.12);
			int zeroadd_y = static_cast<int>(plate.height * 0.80);
			plate.x -= zeroadd_x;
			plate.y -= zeroadd_y;
			plate.height += zeroadd_h;
			plate.width += zeroadd_w;
			
			cv::Mat plateImage = util::cropFromImage(InputImage, plate);
		    
			//Rect_<float> safeBoundRect;
			//int isFormRect = calcSafeRect(plate, InputImage, safeBoundRect);
			//Mat bound_mat = InputImage(safeBoundRect);
			//namedWindow("车牌定位", WINDOW_AUTOSIZE);
			//cv::imshow("车牌定位", plateImage);
			//waitKey(0);
			PlateInfo plateInfo(plateImage, plate);
			plateInfos.push_back(plateInfo);

		}
	}

	void PlateDetection::plateDetetionRough2(cv::Mat InputImage, std::vector<lpr::PlateInfo>& plateInfos)
	{
		cv::Mat mat1;
		//resize(InputImage, mat1, Size(1024, 720));
		cv::Mat blob = cv::dnn::blobFromImage(InputImage, 0.008, cv::Size(720, 1024), cv::Scalar(), true, false, 5);
		net.setInput(blob);

		//clock_t start, finish;
		//start = clock();
		//float threshold = 0.5;
		cv::Mat detection = net.forward();
		//  finish = clock();//计时结束
	    //	cout << "" << (double)(finish - start) / 1000 << "s" << finish << endl;
		//  获取推断时间
	    //	vector<double> layerTimings;
	    //	double freq = getTickFrequency() / 1000;
	    //	double time = net.getPerfProfile(layerTimings) / freq;
		//  ostringstream ss;
	    //	ss << "infernece : " << time << " ms";

		//putText(src, ss.str(), Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2, 8);

		cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
		for (int i = 0; i < detectionMat.rows; i++) {
			float confidence = detectionMat.at<float>(i, 2);
			if (confidence > 0.2) {
				size_t obj_index = (size_t)detectionMat.at<float>(i, 1);
				float tl_x = detectionMat.at<float>(i, 3) * InputImage.cols;
				float tl_y = detectionMat.at<float>(i, 4) * InputImage.rows;
				float br_x = detectionMat.at<float>(i, 5) * InputImage.cols;
				float br_y = detectionMat.at<float>(i, 6) * InputImage.rows;
				//Rect object_box((int)tl_x - 20, (int)tl_y - 40, (int)(br_x - tl_x) + 50, (int(br_y - tl_y)) + 50);
				cv::Rect object_box((int)tl_x - 10, (int)tl_y - 20, (int)(br_x - tl_x) + 25, (int(br_y - tl_y)) + 25);
				//rectangle(mat1, object_box, Scalar(0, 0, 255), 2, 8, 0);
				try {
					cv::Mat s = InputImage(object_box);
					PlateInfo plateInfo(s, object_box);
					plateInfos.push_back(plateInfo);
				}
				catch (exception e1) {

				}
				

				//	namedWindow("ssd", WINDOW_NORMAL);
					//imshow("ssd", src);
					//waitKey(0);

			
			}


		}
	}





}

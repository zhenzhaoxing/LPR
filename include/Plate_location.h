//
// 
//

#ifndef SWIFTPR_PLATEDETECTION_H
#define SWIFTPR_PLATEDETECTION_H


#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include "PlateInfo.h"
#include <vector>
namespace lpr {
	class PlateDetection {
	public:
		PlateDetection(std::string filename_cascade, std::string xml, std::string bin);
		PlateDetection();
		void LoadModel(std::string filename_cascade, std::string xml, std::string bin);
		void plateDetectionRough(cv::Mat InputImage, std::vector<lpr::PlateInfo>& plateInfos, int min_w = 36, int max_w = 800);

		void plateDetetionRough2(cv::Mat InputImage, std::vector<lpr::PlateInfo>& plateInfos);
		



	private:
		cv::CascadeClassifier cascade;
		cv::dnn::Net net;

	};

}

#endif //SWIFTPR_PLATEDETECTION_H


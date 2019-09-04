//
// 
//

#ifndef SWIFTPR_FINEMAPPING_H
#define SWIFTPR_FINEMAPPING_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
using namespace std;
namespace lpr {
	class FineMapping {
	public:
		FineMapping();


		FineMapping(string prototxt, string caffemodel);
		static cv::Mat FineMappingVertical(cv::Mat InputProposal, int sliceNum = 15, int upper = 0, int lower = -50, int windows_size = 17);
		cv::Mat FineMappingHorizon(cv::Mat FinedVertical, int leftPadding, int rightPadding);
		//angleSet
		cv::Mat fastdeskew(cv::Mat skewImage, int blockSize);

	private:
		cv::dnn::Net net1;

	};




}
#endif //SWIFTPR_FINEMAPPING_H



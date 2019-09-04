//
// 
//

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#ifndef SWIFTPR_PIPLINE_H
#define SWIFTPR_PIPLINE_H
#include "Plate_location.h"
#include "PlateInfo.h"

#include "Fine_Mapping.h"
#include "Char_Recognizer.h"

using namespace std;
namespace lpr {
	
	//保存车牌信息的类
	class PipelinePR {
	public:

		PlateDetection* plateDetection;
		FineMapping* fineMapping;
		SegmentationFreeRecognizer* charreg;
		PipelinePR(string detector_filename,string xml,string bin,
			string finemapping_prototxt, string finemapping_caffemodel,
			string char_regxml, string char_rebin
		) {
			plateDetection = new PlateDetection(detector_filename,xml,bin);
			fineMapping = new FineMapping(finemapping_prototxt, finemapping_caffemodel);
			charreg = new SegmentationFreeRecognizer(char_regxml, char_rebin);
		}

		~PipelinePR() {

			delete plateDetection;
			delete fineMapping;
			delete charreg;
		}
		/*
	"京","沪","津","渝","冀","晋","蒙","辽","吉","HEI","SU","浙","皖","闽","赣","鲁","豫","鄂","湘","粤","桂","琼","川","贵","云","藏","陕","甘","青","宁","新",
		*/

		const std::vector<std::string> CH_PLATE_CODE{ "京", "沪", "津", "渝", "冀", "晋", "蒙", "辽", "吉", "黑", "苏", "浙", "皖", "闽", "赣", "鲁", "豫", "鄂", "湘", "粤", "桂",
									   "琼", "川", "贵", "云", "藏", "陕", "甘", "青", "宁", "新", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A",
									   "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
									   "Y", "Z","港","学","使","警","澳","挂","军","北","南","广","沈","兰","成","济","海","民","航","空" };
		vector<string> plateRes;

		//保存车牌中间信息;
		//vector<pr::PlateInfo> plates;
		//声明结果列表;
		vector<PlateInfo> results;
		cv::Mat image_finemapping;
		//vector<Mat> chepaiimage;
		vector<PlateInfo> RunPiplineAsImage(cv::Mat plateImage, vector<lpr::PlateInfo>& plates) {
			
			cv::Mat a, b, c, d;
			//执行车牌粗略探测位置(结果存在plates内);
			//Mat mat_blur;
			//mat_blur = InputImage.clone();
			//GaussianBlur(plateImage, mat_blur, Size(1, 1), 0, 0);
			//cout<<plateImage.rows<<endl;
		//	cout<<plateImage.cols<<endl;
			plateDetection->plateDetectionRough(plateImage, plates);
			if (plates.empty()) {
				//cout<<"识别不出"<< plates .size()<<endl;
				plateDetection->plateDetetionRough2(plateImage, plates);
			}

			//迭代图中每个车牌;
			for (lpr::PlateInfo plateinfo : plates) {
				//获取该车牌图像(image_finemapping的finemapping是为了分割出尽量只包含单个车牌的图像);
				//clock_t startTime, endTime;
				//startTime = clock();//计时开始
				cv::Mat	image_finemapping = plateinfo.getPlateImage();

				//对图像垂直处理;
				a = fineMapping->FineMappingVertical(image_finemapping);
				//校正角度;
				b=fineMapping->fastdeskew(a, 5);
				//对图像水平处理;
				c = fineMapping->FineMappingHorizon(b, 2, 5);
				//showpic(c);
				//大小调整;
				resize(c, d, cv::Size(136+4, 36));
				plateinfo.setPlateImage(d);
				/**************************各种字符识别************************/

				std::pair<std::string, float> res = charreg->SegmentationFreeForSinglePlate(plateinfo.getPlateImage(), CH_PLATE_CODE);
				plateinfo.confidence = res.second;
				plateinfo.setPlateName(res.first);

				results.push_back(plateinfo);
			}

			
			
			return results;
		}


		void showpic(cv::Mat d) {
			cv::namedWindow("sss", cv::WINDOW_NORMAL);
			cv::imshow("sss", d);
			cv::waitKey(0);
		}



		/*
		动态识别
		*/
		vector<PlateInfo> dongtaiAsImage(cv::Mat InputImage, cv::Rect& platesRegion) {
			vector<PlateInfo> result;


			
			return result;
		}


	
	};
}
#endif //SWIFTPR_PIPLINE_H


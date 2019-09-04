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
	
	//���泵����Ϣ����
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
	"��","��","��","��","��","��","��","��","��","HEI","SU","��","��","��","��","³","ԥ","��","��","��","��","��","��","��","��","��","��","��","��","��","��",
		*/

		const std::vector<std::string> CH_PLATE_CODE{ "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "³", "ԥ", "��", "��", "��", "��",
									   "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A",
									   "B", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
									   "Y", "Z","��","ѧ","ʹ","��","��","��","��","��","��","��","��","��","��","��","��","��","��","��" };
		vector<string> plateRes;

		//���泵���м���Ϣ;
		//vector<pr::PlateInfo> plates;
		//��������б�;
		vector<PlateInfo> results;
		cv::Mat image_finemapping;
		//vector<Mat> chepaiimage;
		vector<PlateInfo> RunPiplineAsImage(cv::Mat plateImage, vector<lpr::PlateInfo>& plates) {
			
			cv::Mat a, b, c, d;
			//ִ�г��ƴ���̽��λ��(�������plates��);
			//Mat mat_blur;
			//mat_blur = InputImage.clone();
			//GaussianBlur(plateImage, mat_blur, Size(1, 1), 0, 0);
			//cout<<plateImage.rows<<endl;
		//	cout<<plateImage.cols<<endl;
			plateDetection->plateDetectionRough(plateImage, plates);
			if (plates.empty()) {
				//cout<<"ʶ�𲻳�"<< plates .size()<<endl;
				plateDetection->plateDetetionRough2(plateImage, plates);
			}

			//����ͼ��ÿ������;
			for (lpr::PlateInfo plateinfo : plates) {
				//��ȡ�ó���ͼ��(image_finemapping��finemapping��Ϊ�˷ָ������ֻ�����������Ƶ�ͼ��);
				//clock_t startTime, endTime;
				//startTime = clock();//��ʱ��ʼ
				cv::Mat	image_finemapping = plateinfo.getPlateImage();

				//��ͼ��ֱ����;
				a = fineMapping->FineMappingVertical(image_finemapping);
				//У���Ƕ�;
				b=fineMapping->fastdeskew(a, 5);
				//��ͼ��ˮƽ����;
				c = fineMapping->FineMappingHorizon(b, 2, 5);
				//showpic(c);
				//��С����;
				resize(c, d, cv::Size(136+4, 36));
				plateinfo.setPlateImage(d);
				/**************************�����ַ�ʶ��************************/

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
		��̬ʶ��
		*/
		vector<PlateInfo> dongtaiAsImage(cv::Mat InputImage, cv::Rect& platesRegion) {
			vector<PlateInfo> result;


			
			return result;
		}


	
	};
}
#endif //SWIFTPR_PIPLINE_H


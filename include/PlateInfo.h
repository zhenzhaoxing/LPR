//
// 
//

#ifndef SWIFTPR_PLATEINFO_H
#define SWIFTPR_PLATEINFO_H
#include <opencv2/opencv.hpp>
using namespace std;

namespace lpr {

	typedef vector<cv::Mat> Character;

	enum PlateColor { BLUE, YELLOW, WHITE, GREEN, BLACK, UNKNOWN };
	enum CharType { CHINESE, LETTER, LETTER_NUMS };


	class PlateInfo {
	public:
		vector<pair<CharType, cv::Mat>> plateChars;
		vector<pair<CharType, cv::Mat>> plateCoding;

		float confidence = 0;


		PlateInfo(const cv::Mat& plateData, string plateName, cv::Rect plateRect, PlateColor plateType) {
			licensePlate = plateData;
			name = plateName;
			ROI = plateRect;
			Type = plateType;
		}
		PlateInfo(const cv::Mat& plateData, cv::Rect plateRect, PlateColor plateType) {
			licensePlate = plateData;
			ROI = plateRect;
			Type = plateType;
		}
		PlateInfo(const cv::Mat& plateData, cv::Rect plateRect) {
			licensePlate = plateData;
			ROI = plateRect;
		}
		PlateInfo() {
		}



		cv::Mat getPlateImage() {
			return licensePlate;
		}

		void setPlateImage(cv::Mat plateImage) {
			licensePlate = plateImage;
		}

		cv::Rect getPlateRect() {
			return ROI;
		}

		void setPlateRect(cv::Rect plateRect) {
			ROI = plateRect;
		}
		cv::String getPlateName() {
			return name;

		}
		void setPlateName(cv::String plateName) {
			name = plateName;
		}
		int getPlateType() {
			return Type;
		}

		void appendPlateChar(const pair<CharType, cv::Mat>& plateChar)
		{
			plateChars.push_back(plateChar);
		}

		void appendPlateCoding(const pair<CharType, cv::Mat>& charProb) {
			plateCoding.push_back(charProb);
		}

		


	private:
		cv::Mat licensePlate;
		cv::Rect ROI;
		string name;
		PlateColor Type;

	};
}


#endif //SWIFTPR_PLATEINFO_H



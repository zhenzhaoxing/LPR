//
// 	对图像垂直处理;
//
#include "stdafx.h"
#include "../include/Fine_Mapping.h"
using namespace std;
namespace lpr {
	const int FINEMAPPING_H = 40;
	const int FINEMAPPING_W = 136;
	const int PADDING_UP_DOWN = 30;
	//angle
	const int ANGLE_MIN = 30;
	const int ANGLE_MAX = 150;
	const int PLATE_H = 36;
	const int PLATE_W = 136;
	void drawRect(cv::Mat image, cv::Rect rect)
	{
		cv::Point p1(rect.x, rect.y);
		cv::Point p2(rect.x + rect.width, rect.y + rect.height);
		cv::rectangle(image, p1, p2, cv::Scalar(0, 255, 0), 1);
	
	}
	FineMapping::FineMapping(string prototxt, string caffemodel) {
 	//	net = dnn::readNetFromCaffe(prototxt, caffemodel);
		net1=cv::dnn:: readNetFromModelOptimizer(prototxt, caffemodel);
		net1.setPreferableBackend(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
		net1.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

	}

	cv::Mat FineMapping::FineMappingHorizon(cv::Mat FinedVertical, int leftPadding, int rightPadding)
	{
	
		//        if(FinedVertical.channels()==1)
		//            cvtColor(FinedVertical,FinedVertical,COLOR_GRAY2BGR);
		cv::Mat inputBlob = cv::dnn::blobFromImage(FinedVertical, 1 / 255.0, cv::Size(66, 16),
			cv::Scalar(0, 0, 0), false);
	
		//net.setInput(inputBlob, "data");
		net1.setInput(inputBlob,"data");
		cv::Mat prob = net1.forward();
		
		int front = static_cast<int>(prob.at<float>(0, 0) * FinedVertical.cols);
		int back = static_cast<int>(prob.at<float>(0, 1) * FinedVertical.cols);
		front -= leftPadding;
		if (front < 0) front = 0;
		back += rightPadding;
		if (back > FinedVertical.cols - 1) back = FinedVertical.cols - 1;
		cv::Mat cropped = FinedVertical.colRange(front, back).clone();
		return  cropped;


	}

	pair<int, int> FitLineRansac(vector<cv::Point> pts, int zeroadd = 0)
	{
		pair<int, int> res;
		if (pts.size() > 2)
		{
			cv::Vec4f line;
			fitLine(pts, line, cv::DIST_HUBER, 0, 0.01, 0.01);
			float vx = line[0];
			float vy = line[1];
			float x = line[2];
			float y = line[3];
			int lefty = static_cast<int>((-x * vy / vx) + y);
			int righty = static_cast<int>(((136 - x) * vy / vx) + y);
			res.first = lefty + PADDING_UP_DOWN + zeroadd;
			res.second = righty + PADDING_UP_DOWN + zeroadd;
			return res;
		}
		res.first = zeroadd;
		res.second = zeroadd;
		return res;
	}

	cv::Mat FineMapping::FineMappingVertical(cv::Mat InputProposal, int sliceNum, int upper, int lower, int windows_size)
	{
		cv::Mat PreInputProposal;
		cv::Mat proposal;
		resize(InputProposal, PreInputProposal, cv::Size(FINEMAPPING_W, FINEMAPPING_H));
		if (InputProposal.channels() == 3)
			cvtColor(PreInputProposal, proposal, cv::COLOR_BGR2GRAY);
		else
			PreInputProposal.copyTo(proposal);
		//   proposal = PreInputProposal;
				// this will improve some sen
		cv::Mat kernal = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(1, 3));
		//        erode(proposal,proposal,kernal);


		float diff = static_cast<float>(upper - lower);
		diff /= static_cast<float>(sliceNum - 1);
		cv::Mat binary_adaptive;
		vector<cv::Point> line_upper;
		vector<cv::Point> line_lower;
		int contours_nums = 0;

		for (int i = 0; i < sliceNum; i++)
		{
			vector<vector<cv::Point> > contours;
			float k = lower + i * diff;
			adaptiveThreshold(proposal, binary_adaptive, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, windows_size, k);
			cv::Mat draw;
			binary_adaptive.copyTo(draw);
			
			findContours(binary_adaptive, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
			for (auto contour : contours)
			{
				cv::Rect bdbox = boundingRect(contour);
				float lwRatio = bdbox.height / static_cast<float>(bdbox.width);
				int  bdboxAera = bdbox.width * bdbox.height;
				if ((lwRatio > 0.7 && bdbox.width * bdbox.height > 100 && bdboxAera < 300)
					|| (lwRatio > 3.0 && bdboxAera < 100 && bdboxAera>10))
				{
					cv::Point p1(bdbox.x, bdbox.y);
					cv::Point p2(bdbox.x + bdbox.width, bdbox.y + bdbox.height);
					line_upper.push_back(p1);
					line_lower.push_back(p2);
					contours_nums += 1;
				}
			}
		}

		//cout << "contours_nums " << contours_nums << endl;

		if (contours_nums < 41)
		{
			bitwise_not(InputProposal, InputProposal);
			cv::Mat kernal = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(1, 5));
			;
			cv::Mat bak;
			resize(InputProposal, bak, cv::Size(FINEMAPPING_W, FINEMAPPING_H));
			erode(bak, bak, kernal);
			if (InputProposal.channels() == 3)
				cvtColor(bak, proposal, cv::COLOR_BGR2GRAY);
			else
				proposal = bak;
			int contours_nums = 0;

			for (int i = 0; i < sliceNum; i++)
			{
				vector<vector<cv::Point> > contours;
				float k = lower + i * diff;
				adaptiveThreshold(proposal, binary_adaptive, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, windows_size, k);
				//                imshow("image",binary_adaptive);
				//            waitKey(0);
				cv::Mat draw;
				binary_adaptive.copyTo(draw);
				findContours(binary_adaptive, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
				for (auto contour : contours)
				{
					cv::Rect bdbox = boundingRect(contour);
					float lwRatio = bdbox.height / static_cast<float>(bdbox.width);
					int  bdboxAera = bdbox.width * bdbox.height;
					if ((lwRatio > 0.7 && bdbox.width * bdbox.height > 120 && bdboxAera < 300)
						|| (lwRatio > 3.0 && bdboxAera < 100 && bdboxAera>10))
					{

						cv::Point p1(bdbox.x, bdbox.y);
						cv::Point p2(bdbox.x + bdbox.width, bdbox.y + bdbox.height);
						line_upper.push_back(p1);
						line_lower.push_back(p2);
						contours_nums += 1;
					}
				}
			}
			//             cout<<"contours_nums "<<contours_nums<<endl;
		}

		cv::Mat rgb;
		copyMakeBorder(PreInputProposal, rgb, 30, 30, 0, 0, cv::BORDER_REPLICATE);
		


		pair<int, int> A;
		pair<int, int> B;
		A = FitLineRansac(line_upper, -2);
		B = FitLineRansac(line_lower, 2);
		int leftyB = A.first;
		int rightyB = A.second;
		int leftyA = B.first;
		int rightyA = B.second;
		int cols = rgb.cols;
		int rows = rgb.rows;
		//        pts_map1  = np.float32([[cols - 1, rightyA], [0, leftyA],[cols - 1, rightyB], [0, leftyB]])
		//        pts_map2 = np.float32([[136,36],[0,36],[136,0],[0,0]])
		//        mat = cv2.getPerspectiveTransform(pts_map1,pts_map2)
		//        image = cv2.warpPerspective(rgb,mat,(136,36),flags=cv2.INTER_CUBIC)
		vector<cv::Point2f> corners(4);
		corners[0] = cv::Point2f(cols - 1, rightyA);
		corners[1] = cv::Point2f(0, leftyA);
		corners[2] = cv::Point2f(cols - 1, rightyB);
		corners[3] = cv::Point2f(0, leftyB);
		vector<cv::Point2f> corners_trans(4);
		corners_trans[0] = cv::Point2f(136, 36);
		corners_trans[1] = cv::Point2f(0, 36);
		corners_trans[2] = cv::Point2f(136, 0);
		corners_trans[3] = cv::Point2f(0, 0);
		cv::Mat transform = getPerspectiveTransform(corners, corners_trans);
		cv::Mat quad = cv::Mat::zeros(36, 136, CV_8UC3);
		warpPerspective(rgb, quad, transform, quad.size());
		
		return quad;
		
	}

	/*angle*/
	int angle(float x, float y)
	{
		return atan2(x, y) * 180 / 3.1415;
	}

	std::vector<float> avgfilter(std::vector<float> angle_list, int windowsSize) {
		std::vector<float> angle_list_filtered(angle_list.size() - windowsSize + 1);
		for (int i = 0; i < angle_list.size() - windowsSize + 1; i++) {
			float avg = 0.00f;
			for (int j = 0; j < windowsSize; j++) {
				avg += angle_list[i + j];
			}
			avg = avg / windowsSize;
			angle_list_filtered[i] = avg;
		}
		return angle_list_filtered;
	}
	void drawHist(std::vector<float> seq) {
		cv::Mat image(300, seq.size(), CV_8U);
		image.setTo(0);

		for (int i = 0; i < seq.size(); i++)
		{
			float l = *std::max_element(seq.begin(), seq.end());

			int p = int(float(seq[i]) / l * 300);

			cv::line(image, cv::Point(i, 300), cv::Point(i, 300 - p), cv::Scalar(255, 255, 255));
		}
		//cv::imshow("vis", image);
	}
	cv::Mat  correctPlateImage(cv::Mat skewPlate, float angle, float maxAngle)
	{

		cv::Mat dst;
		cv::Size size_o(skewPlate.cols, skewPlate.rows);


		int extend_padding = 0;
		//        if(angle<0)
		extend_padding = static_cast<int>(skewPlate.rows * tan(cv::abs(angle) / 180 * 3.14));
		//        else
		//            extend_padding = static_cast<int>(skewPlate.rows/tan(cv::abs(angle)/180* 3.14) );

		//        std::cout<<"extend:"<<extend_padding<<std::endl;

		cv::Size size(skewPlate.cols + extend_padding, skewPlate.rows);

		float interval = abs(sin((angle / 180) * 3.14) * skewPlate.rows);
		//        std::cout<<interval<<std::endl;

		cv::Point2f pts1[4] = { cv::Point2f(0,0),cv::Point2f(0,size_o.height),cv::Point2f(size_o.width,0),cv::Point2f(size_o.width,size_o.height) };
		if (angle > 0) {
			cv::Point2f pts2[4] = { cv::Point2f(interval, 0), cv::Point2f(0, size_o.height),
								   cv::Point2f(size_o.width, 0), cv::Point2f(size_o.width - interval, size_o.height) };
			cv::Mat M = cv::getPerspectiveTransform(pts1, pts2);
			cv::warpPerspective(skewPlate, dst, M, size);


		}
		else {
			cv::Point2f pts2[4] = { cv::Point2f(0, 0), cv::Point2f(interval, size_o.height), cv::Point2f(size_o.width - interval, 0),
								   cv::Point2f(size_o.width, size_o.height) };
			cv::Mat M = cv::getPerspectiveTransform(pts1, pts2);
			cv::warpPerspective(skewPlate, dst, M, size, cv::INTER_CUBIC);

		}
		return  dst;
	}

	cv::Mat FineMapping:: fastdeskew(cv::Mat skewImage, int blockSize) {


		const int FILTER_WINDOWS_SIZE = 1;
		std::vector<float> angle_list(180);
		memset(angle_list.data(), 0, angle_list.size() * sizeof(int));
		cv::Mat bak;
		skewImage.copyTo(bak);
		if (skewImage.channels() == 3)
			cv::cvtColor(skewImage, skewImage, cv::COLOR_RGB2GRAY);
		if (skewImage.channels() == 1)
		{
			cv::Mat eigen;
			cv::cornerEigenValsAndVecs(skewImage, eigen, blockSize, 5);
			for (int j = 0; j < skewImage.rows; j += blockSize)
			{
				for (int i = 0; i < skewImage.cols; i += blockSize)
				{
					float x2 = eigen.at<cv::Vec6f>(j, i)[4];
					float y2 = eigen.at<cv::Vec6f>(j, i)[5];
					int angle_cell = angle(x2, y2);

					angle_list[(angle_cell + 180) % 180] += 1.0;

				}
			}
		}
		std::vector<float> filtered = avgfilter(angle_list, 10);

		int maxPos = std::max_element(filtered.begin(), filtered.end()) - filtered.begin() + FILTER_WINDOWS_SIZE / 2;
		if (maxPos > ANGLE_MAX)
			maxPos = (-maxPos + 90 + 180) % 180;
		if (maxPos < ANGLE_MIN)
			maxPos -= 90;
		maxPos = 90 - maxPos;
		cv::Mat deskewed = correctPlateImage(bak, static_cast<float>(maxPos), 60.0f);

		return deskewed;
	}

}



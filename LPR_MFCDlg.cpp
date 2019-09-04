
// LPR_MFCDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "include/putText.h"
#include "include/LPR_MFC.h"
#include "include/LPR_MFCDlg.h"
#include "afxdialogex.h"
#include <string>
#include <opencv2/opencv.hpp>
#include "include/Plate_location.h"
#include <iostream>
#include<fstream>
#include "include/Pipeline.h"
#include "include/CvxText.h"
#include <shellapi.h>
using namespace lpr;
using namespace cv;
using namespace std;



#ifdef _DEBUG
#define new DEBUG_NEW
#endif



string imageOpenPath; //存储打开文件后的图片路径 全局
//保存车牌信息
std::vector<lpr::PlateInfo> plates;

vector<Mat> resultVec;//全局变量，经过判断的车牌图块集合
const map<string, string> platest{
		{"京", "JING"},{"沪", "HU"},{"津","JIN"},{"渝", "YUA"} ,{"冀", "JIA"},{"晋", "JIN"},{"蒙", "MENG"},{"辽", "LIAO"},
		{"吉", "JIB"},{"黑", "HEI"},{"苏", "SU"},{"浙", "ZHE"},{"皖", "WAN"},{"闽", "MIN"},{"赣", "GANA"},{"鲁", "LU"},
		{"豫", "YUB"},{"鄂", "E"},{"湘", "XIANG"},{"粤", "YUE"},{"桂", "GUIA"},{"琼", "QIONG"},{"川", "CHUAN"},{"贵", "GUIB"},
		{"云", "YUN"},{"藏", "ZANG"},{"陕", "SHAN"},{"甘", "GANB"},{"青", "QING"},{"宁", "NING"},{"新", "XIN"},
		{"挂", "GUA"},{"学", "XUE"},{"军", "JUN"},{"警", "JING"},{"港", "GANG"},{"澳", "AO"}

};




// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()



// CLPR_MFCDlg 对话框

void CLPR_MFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
	DDX_Control(pDX, IDC_SHOWIMG, m_showImg);
	DDX_Control(pDX, IDC_EDIT2, m_locateTime);
	//DDX_Control(pDX, IDC_EDIT3, m_recognizeTime);
}

CLPR_MFCDlg::CLPR_MFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLPR_MFCDlg::IDD, pParent)


{

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	/*
	psrc = new pr::PipelinePR("../LPR_MFC/model/cascade.xml",
		"../LPR_MFC/model/HorizonalFinemapping.prototxt", "../LPR_MFC/model/HorizonalFinemapping.caffemodel",
		"../LPR_MFC/model/Segmentation.prototxt", "../LPR_MFC/model/Segmentation.caffemodel",
		"../LPR_MFC/model/CharacterRecognization.prototxt", "../LPR_MFC/model/CharacterRecognization.caffemodel"
	);
	*/
}



BEGIN_MESSAGE_MAP(CLPR_MFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//	ON_BN_CLICKED(IDC_BUTTON1, &CLPR_MFCDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_OPEN_BUTTON, &CLPR_MFCDlg::OnBnClickedOpenButton)
	ON_BN_CLICKED(IDC_SELECT_BUTTON, &CLPR_MFCDlg::OnBnClickedSelectButton)
	ON_BN_CLICKED(IDC_RECOGNIZE_BUTTON, &CLPR_MFCDlg::OnBnClickedRecognizeButton)
	ON_BN_CLICKED(IDOK, &CLPR_MFCDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT1, &CLPR_MFCDlg::OnEnChangeEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS4, &CLPR_MFCDlg::OnNMCustomdrawProgress4)

END_MESSAGE_MAP()


// CLPR_MFCDlg 消息处理程序
//递归出来图片
void CLPR_MFCDlg::BrowseCurrentAllFile(CString strDir)
{
	if (strDir == _T(""))
	{
		return;
	}
	else
	{
		if (strDir.Right(1) != _T("\\"))
			strDir += L"\\";
		strDir = strDir + _T("*.*");
	}
	CFileFind finder;
	CString strPath;
	BOOL bWorking = finder.FindFile(strDir);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		strPath = finder.GetFilePath();
		if (finder.IsDirectory() && !finder.IsDots())
			BrowseCurrentAllFile(strPath); //递归调用  
		else if (!finder.IsDirectory() && !finder.IsDots())
		{
			//strPath就是所要获取的文件路径  
			m_List.push_back(strPath);
		}

	}
}

int CLPR_MFCDlg::TEST_PIPELINE(string filename, vector<lpr::PlateInfo> plates, ofstream& outFile)
{


	lpr::PipelinePR par("../LPR_MFC/model/cascade.xml", "../LPR_MFC/model/lpr.xml", "../LPR_MFC/model/lpr.bin",
		"../LPR_MFC/model/HorizonalFinemapping.xml", "../LPR_MFC/model/HorizonalFinemapping.bin",
		"../LPR_MFC/model/LPRnet.xml", "../LPR_MFC/model/LPRnet.bin"
	);


	cv::Mat image;
	cv::Mat shiyan = cv::imread(filename);
	//cv::imshow("sss", shiyan);
	//cv::waitKey(0);
	image = shiyan.cols > 1300 && shiyan.rows > 1300 ? matresize(shiyan, image) : shiyan;
	std::vector<lpr::PlateInfo> res = par.RunPiplineAsImage(image, plates);
	if (res.size() == 0) {
		return -1;
	}
	int index = filename.find_last_of("\\");
	string filenamestr = filename.substr(index + 1, filename.size());
	outFile << filenamestr << endl;
	//cout << filenamestr << endl;
	//text = filenamestr.c_str();
	string str;
	for (auto st : res) {
		//cout<< "字符个数"<<st.getPlateName().size() <<endl;
		if (st.confidence > 0.8 && st.getPlateName().size() >= 8
			) {
			str = st.getPlateName().replace(0, 2, platest.find(st.getPlateName().substr(0, 2))->second);
			//cout << str << endl;
			outFile << str << endl;
			//imwrite("E:\\1010000.jpg", st.getPlateImage());
			cv::Rect region = st.getPlateRect();
			cv::rectangle(image, cv::Point(region.x, region.y), cv::Point(region.x + region.width, region.y + region.height), cv::Scalar(0, 0, 255), 2);
			//	putText(image, st.getPlateName().c_str(), Point(region.x + 30, region.y - 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 255), 1,3);
			Cv310Text text1("model/simfang.ttf");
			float p = 0.9;//设置透明度
			CvScalar fontSize = { 35,0.5,0.1,0 };//字体大小,空白字符大小比例,间隔大小比例,暂时不支持
			bool isUnderLine = true;//下画线(不支持)
			text1.setFont(NULL, &fontSize, &isUnderLine, &p);
			//text.setFont(NULL, NULL, NULL, &p);   // 透明处理 
			text1.putText(image, st.getPlateName().c_str(), cvPoint(region.x + 10, region.y - 10), CV_RGB(255, 0, 0));
			cv::imwrite("C:/test-results/16080imgs/" + filenamestr, image);
		
			//imwrite("E:\\" + filenamestr, image);
			//text += str.c_str();

		}
		plates.swap(vector<lpr::PlateInfo>());
	}


	return 0;





}

Mat CLPR_MFCDlg::matresize(cv::Mat mat, cv::Mat image)
{
	cv::resize(mat, image, cv::Size(mat.cols / 3, mat.rows / 3));
	return image;
}

BOOL CLPR_MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// TODO:  在此添加额外的初始化代码
	cv::namedWindow("原始图", CV_WINDOW_NORMAL);
	HWND hWnd = (HWND)cvGetWindowHandle("原始图");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_SHOWIMG)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	//((CButton*)GetDlgItem(IDC_RADIOIMAGE))->SetCheck(true);
	//((CButton*)GetDlgItem(IDC_SOME))->SetCheck(true);
	pProgCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS4);
	pProgCtrl->SetRange(0, 100);//设置进度条范围
	pProgCtrl->ShowWindow(0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLPR_MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLPR_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLPR_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CLPR_MFCDlg::OnBnClickedOpenButton() //打开图片并显示，将图片路径加载到全局变量imageOpenPath中
{
	CEdit* pedt = (CEdit*)GetDlgItem(IDC_EDIT1);
	string sd = "";
	CString st(sd.c_str());
	pedt->SetWindowText(st);
	m_Edit.SetWindowText(_T(" "));//清空编辑框

	m_locateTime.SetWindowText(_T(" "));//清空编辑框

	CFileDialog fileDlg(TRUE, _T("*.*"), NULL, 0, NULL, this);  //不设置过滤器，NULL
	CString strFilePath;


	// 显示打开文件对话框   
	if (fileDlg.DoModal() == IDOK)
	{
		//清空图片控件
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_SHOWIMG);
		pStatic->SetBitmap(NULL);
		// 如果点击了文件对话框上的“打开”按钮，  
		strFilePath = fileDlg.GetPathName();
		strFilePath.Replace(_T("//"), _T("////"));  // imread打开文件路径是双斜杠，所以这里需要转换 

		//unicode字节工程下CString转string
		USES_CONVERSION;
		std::string imagePath(W2A(strFilePath));
		//全局
		imageOpenPath = imagePath;

		cv::Mat src = cv::imread(imageOpenPath);

		//先清空图片
		m_showImg.SetBitmap(0);
		//再刷新窗体
		this->RedrawWindow();

		Mat src_show = src.clone();

		CRect rect;
		GetDlgItem(IDC_SHOWIMG)->GetClientRect(&rect);
		Rect dst(rect.left, rect.top, rect.right, rect.bottom);
		resize(src_show, src_show, cv::Size(rect.Width(), rect.Height()));


		cv::imshow("原始图", src_show);
		resizeWindow("原始图", src_show.size().width, src_show.size().height);
		//清空容器

		plates.swap(vector<lpr::PlateInfo>());
		
	}
}



/*
*
*定位车牌和识别车牌
*
*/
void CLPR_MFCDlg::OnBnClickedSelectButton()
{


	if (imageOpenPath.empty()) {
		AfxMessageBox(_T("请先载入图像"));
		return;
	}
	// TODO:  在此添加控件通知处理程序代码
		//计算程序运行时间
	m_locateTime.SetWindowText(_T(" "));//清空编辑框
	m_Edit.SetWindowText(_T(" "));//清空编辑框
	//clock_t start, end;
	plates.swap(vector<lpr::PlateInfo>());
	lpr::PipelinePR par("../LPR_MFC/model/cascade.xml", "../LPR_MFC/model/lpr.xml", "../LPR_MFC/model/lpr.bin",
		"../LPR_MFC/model/HorizonalFinemapping.xml", "../LPR_MFC/model/HorizonalFinemapping.bin",
		"../LPR_MFC/model/LPRnet.xml", "../LPR_MFC/model/LPRnet.bin"
	);

	cv::Mat image;
	cv::Mat shiyan = cv::imread(imageOpenPath);
	//cv::imshow("sss", shiyan);
	//cv::waitKey(0);
	image = shiyan.cols > 1300 && shiyan.rows > 1300 ? matresize(shiyan, image) : shiyan;
	std::vector<lpr::PlateInfo> res = par.RunPiplineAsImage(image, plates);
	string chepai;
	if (res.size() == 0)
	{
		// 显示消息对话框   
		INT_PTR nRes;
		nRes = MessageBox(_T("无法定位到车牌！\n请重新选择图片！"), _T("提示"), MB_ICONWARNING);
		std::cout << "没有发现车牌！" << std::endl;

	}
	else {
		for (auto st : res) {
			//cout<< "字符个数"<<st.getPlateName().size() <<endl;
			if (st.confidence > 0.8 && st.getPlateName().size() >= 8
				) {
				//str = st.getPlateName().replace(0, 2, platest.find(st.getPlateName().substr(0, 2))->second);
				//cout << str << endl;
				//outFile << str << endl;
				//imwrite("E:\\1010000.jpg", st.getPlateImage());
				cv::Rect region = st.getPlateRect();
				cv::rectangle(image, cv::Point(region.x, region.y), cv::Point(region.x + region.width, region.y + region.height), cv::Scalar(0, 0, 255), 2);
				//	putText(image, st.getPlateName().c_str(), Point(region.x + 30, region.y - 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 255, 255), 1,3);
				Cv310Text text1("model/simfang.ttf");
				float p = 0.9;//设置透明度
				CvScalar fontSize = { 35,0.5,0.1,0 };//字体大小,空白字符大小比例,间隔大小比例,暂时不支持
				bool isUnderLine = true;//下画线(不支持)
				text1.setFont(NULL, &fontSize, &isUnderLine, &p);
				//text.setFont(NULL, NULL, NULL, &p);   // 透明处理 
				text1.putText(image, st.getPlateName().c_str(), cvPoint(region.x + 10, region.y - 10), CV_RGB(255, 0, 0));
				chepai += st.getPlateName() + "  \r";
			}

		}
		CString callpLicense(chepai.c_str());
		m_locateTime.SetWindowText(callpLicense);
		cout << chepai << endl;
		cv::namedWindow("结果图", WINDOW_NORMAL);
		cv::imshow("结果图", image);
		waitKey(0);

	}


}





/*
*
* 批量读取文件
*
*/
void CLPR_MFCDlg::OnBnClickedRecognizeButton()
{
	m_locateTime.SetWindowText(_T(" "));//清空编辑框
	m_Edit.SetWindowText(_T(" "));//清空编辑框
	plates.swap(vector<lpr::PlateInfo>());
	//获取文件
	isfilepath = true;
	TCHAR           szFolderPath[MAX_PATH] = { 0 };
	CString         strFolderPath = TEXT("");

	BROWSEINFO      sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = _T("请选择一个文件夹：");
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	sInfo.lpfn = NULL;

	// 显示文件夹选择对话框  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// 取得文件夹名  
		if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
		{
			strFolderPath = szFolderPath;
		}
	}
	if (lpidlBrowse != NULL)
	{
		::CoTaskMemFree(lpidlBrowse);
	}
	else
	{
		return;
	}
	CProgressCtrl* pProgCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS4);
	//pProgCtrl->show
	pProgCtrl->ShowWindow(1);
	ofstream outFile;
	vector<string> v_data;
	outFile.open("C:/test-results/No16089mresults.txt", ios::out);
	//IDC_EDIT1
	CString text;//显示文本
	m_List.clear();
	//递归后把图片保存在m_List里面
	BrowseCurrentAllFile(strFolderPath);
	int x;
	for (int i = 0; i < m_List.size(); i++)
	{
		pProgCtrl->SetPos(100 * (i + 1) / m_List.size());
		GetDlgItem(IDC_EDIT1)->GetWindowTextW(text);
		std::string STC(CW2A(m_List[i].GetString()));
		x = TEST_PIPELINE(STC, plates, outFile);
		if (x != 0)
		{
			v_data.push_back(STC.c_str());
			text += STC.c_str();
			text += " 识别失败";
		}
		else
		{
			text += STC.c_str();
			text += " 识别完成";
		}
		text += "\r\n";
        GetDlgItem(IDC_EDIT1)->SetWindowTextW(text);
		CEdit* ce = (CEdit*)(GetDlgItem(IDC_EDIT1));
		ce->LineScroll(ce->GetLineCount());
	}


	outFile.close();
	

	CString strMfc;
	string strStr = "C:\\test-results\\";
	strMfc = strStr.c_str();
	CString str;
	text += "\r\n";
	text += "写入完成，车牌识别将结果图片和输出TXT文件已保存至";
	text += strMfc;
	text += "\r\n";
	str.Format(_T("成功识别到了%d张图片,失败%d"), m_List.size() - v_data.size(),v_data.size());
	text += str;

	GetDlgItem(IDC_EDIT1)->SetWindowTextW(text);



	if (IDYES == MessageBox(_T("输出结果文件已保存至") + strMfc, _T("成功,是否打开该目录"), MB_YESNO))
	{

		ShellExecuteW(NULL, NULL, _T("explorer"), L"C:\\test-results\\", NULL, SW_SHOW);
	
	}




	
	//GetDlgItem(IDC_EDIT1)->SetWindowTextW(text);

}


void CLPR_MFCDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void CLPR_MFCDlg::OnEnChangeEdit1()
{

	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}





/*
CascadeClassifier cascade;
	cascade.load("../LPR_MFC/model/cascade.xml");
	vector<Rect> platesRegions;
	plates.swap(vector<pr::PlateInfo>());

	pr::PipelinePR par("../LPR_MFC/model/cascade.xml",
		"../LPR_MFC/model/HorizonalFinemapping.prototxt", "../LPR_MFC/model/HorizonalFinemapping.caffemodel",
		"../LPR_MFC/model/Segmentation.prototxt", "../LPR_MFC/model/Segmentation.caffemodel",
		"../LPR_MFC/model/CharacterRecognization.prototxt", "../LPR_MFC/model/CharacterRecognization.caffemodel"
	);
	VideoCapture capture(0);
	if (!capture.isOpened()) {
		// 显示消息对话框
		INT_PTR nRes;
		nRes = MessageBox(_T("无法打开摄像头！\n请重新链接！"), _T("提示"), MB_ICONWARNING);

	}
	Mat frame, imgGray;

	namedWindow("video-demo", CV_WINDOW_NORMAL);

	while (capture.read(frame)) {

		//imgGray = frame;
		cv::cvtColor(frame, imgGray, cv::COLOR_BGR2GRAY);
		cv::Size minSize(36, 36 / 4);
		cv::Size maxSize(800, 800 / 4);
		cascade.detectMultiScale(imgGray, platesRegions,
			1.1, 3, cv::CASCADE_SCALE_IMAGE, minSize, maxSize);
		if (platesRegions.size() > 0)
		{
			for (int i = 0; i < platesRegions.size(); i++)
			{
				rectangle(frame, Point(platesRegions[i].x -30,platesRegions[i].y - 30), Point(platesRegions[i].x  + platesRegions[i].width, platesRegions[i].y + platesRegions[i].height), Scalar(0, 255, 0), 2, 8);
				//识别文字
				//vector<pr::PlateInfo> ret = prc.dongtaiAsImage(frame, platesRegions[i]);
				vector<pr::PlateInfo> ret = par.dongtaiAsImage(frame, platesRegions[i]);
				string chepai = "";
				for (auto st : ret) {
					if (st.confidence > 0.9) {
						chepai += st.getPlateName() + "  ";
					}
				}

				putTextZH(frame, chepai.c_str(), Point(platesRegions[i].x + 40, platesRegions[i].y - 30), Scalar(0, 0, 255), 30, "楷体");
			}

		}

		imshow("video-demo", frame);
		if (waitKey(1) > 0)		// delay ms 等待按键退出
		{
			break;
		}
	}
	capture.release();
	destroyAllWindows();
*/


void CLPR_MFCDlg::OnNMCustomdrawProgress4(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

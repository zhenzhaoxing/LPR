
// LPR_MFCDlg.cpp : ʵ���ļ�
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



string imageOpenPath; //�洢���ļ����ͼƬ·�� ȫ��
//���泵����Ϣ
std::vector<lpr::PlateInfo> plates;

vector<Mat> resultVec;//ȫ�ֱ����������жϵĳ���ͼ�鼯��
const map<string, string> platest{
		{"��", "JING"},{"��", "HU"},{"��","JIN"},{"��", "YUA"} ,{"��", "JIA"},{"��", "JIN"},{"��", "MENG"},{"��", "LIAO"},
		{"��", "JIB"},{"��", "HEI"},{"��", "SU"},{"��", "ZHE"},{"��", "WAN"},{"��", "MIN"},{"��", "GANA"},{"³", "LU"},
		{"ԥ", "YUB"},{"��", "E"},{"��", "XIANG"},{"��", "YUE"},{"��", "GUIA"},{"��", "QIONG"},{"��", "CHUAN"},{"��", "GUIB"},
		{"��", "YUN"},{"��", "ZANG"},{"��", "SHAN"},{"��", "GANB"},{"��", "QING"},{"��", "NING"},{"��", "XIN"},
		{"��", "GUA"},{"ѧ", "XUE"},{"��", "JUN"},{"��", "JING"},{"��", "GANG"},{"��", "AO"}

};




// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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



// CLPR_MFCDlg �Ի���

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


// CLPR_MFCDlg ��Ϣ�������
//�ݹ����ͼƬ
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
			BrowseCurrentAllFile(strPath); //�ݹ����  
		else if (!finder.IsDirectory() && !finder.IsDots())
		{
			//strPath������Ҫ��ȡ���ļ�·��  
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
		//cout<< "�ַ�����"<<st.getPlateName().size() <<endl;
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
			float p = 0.9;//����͸����
			CvScalar fontSize = { 35,0.5,0.1,0 };//�����С,�հ��ַ���С����,�����С����,��ʱ��֧��
			bool isUnderLine = true;//�»���(��֧��)
			text1.setFont(NULL, &fontSize, &isUnderLine, &p);
			//text.setFont(NULL, NULL, NULL, &p);   // ͸������ 
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

	// ��������...���˵�����ӵ�ϵͳ�˵��С�
	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	cv::namedWindow("ԭʼͼ", CV_WINDOW_NORMAL);
	HWND hWnd = (HWND)cvGetWindowHandle("ԭʼͼ");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_SHOWIMG)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	//((CButton*)GetDlgItem(IDC_RADIOIMAGE))->SetCheck(true);
	//((CButton*)GetDlgItem(IDC_SOME))->SetCheck(true);
	pProgCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS4);
	pProgCtrl->SetRange(0, 100);//���ý�������Χ
	pProgCtrl->ShowWindow(0);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CLPR_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CLPR_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CLPR_MFCDlg::OnBnClickedOpenButton() //��ͼƬ����ʾ����ͼƬ·�����ص�ȫ�ֱ���imageOpenPath��
{
	CEdit* pedt = (CEdit*)GetDlgItem(IDC_EDIT1);
	string sd = "";
	CString st(sd.c_str());
	pedt->SetWindowText(st);
	m_Edit.SetWindowText(_T(" "));//��ձ༭��

	m_locateTime.SetWindowText(_T(" "));//��ձ༭��

	CFileDialog fileDlg(TRUE, _T("*.*"), NULL, 0, NULL, this);  //�����ù�������NULL
	CString strFilePath;


	// ��ʾ���ļ��Ի���   
	if (fileDlg.DoModal() == IDOK)
	{
		//���ͼƬ�ؼ�
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_SHOWIMG);
		pStatic->SetBitmap(NULL);
		// ���������ļ��Ի����ϵġ��򿪡���ť��  
		strFilePath = fileDlg.GetPathName();
		strFilePath.Replace(_T("//"), _T("////"));  // imread���ļ�·����˫б�ܣ�����������Ҫת�� 

		//unicode�ֽڹ�����CStringתstring
		USES_CONVERSION;
		std::string imagePath(W2A(strFilePath));
		//ȫ��
		imageOpenPath = imagePath;

		cv::Mat src = cv::imread(imageOpenPath);

		//�����ͼƬ
		m_showImg.SetBitmap(0);
		//��ˢ�´���
		this->RedrawWindow();

		Mat src_show = src.clone();

		CRect rect;
		GetDlgItem(IDC_SHOWIMG)->GetClientRect(&rect);
		Rect dst(rect.left, rect.top, rect.right, rect.bottom);
		resize(src_show, src_show, cv::Size(rect.Width(), rect.Height()));


		cv::imshow("ԭʼͼ", src_show);
		resizeWindow("ԭʼͼ", src_show.size().width, src_show.size().height);
		//�������

		plates.swap(vector<lpr::PlateInfo>());
		
	}
}



/*
*
*��λ���ƺ�ʶ����
*
*/
void CLPR_MFCDlg::OnBnClickedSelectButton()
{


	if (imageOpenPath.empty()) {
		AfxMessageBox(_T("��������ͼ��"));
		return;
	}
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
		//�����������ʱ��
	m_locateTime.SetWindowText(_T(" "));//��ձ༭��
	m_Edit.SetWindowText(_T(" "));//��ձ༭��
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
		// ��ʾ��Ϣ�Ի���   
		INT_PTR nRes;
		nRes = MessageBox(_T("�޷���λ�����ƣ�\n������ѡ��ͼƬ��"), _T("��ʾ"), MB_ICONWARNING);
		std::cout << "û�з��ֳ��ƣ�" << std::endl;

	}
	else {
		for (auto st : res) {
			//cout<< "�ַ�����"<<st.getPlateName().size() <<endl;
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
				float p = 0.9;//����͸����
				CvScalar fontSize = { 35,0.5,0.1,0 };//�����С,�հ��ַ���С����,�����С����,��ʱ��֧��
				bool isUnderLine = true;//�»���(��֧��)
				text1.setFont(NULL, &fontSize, &isUnderLine, &p);
				//text.setFont(NULL, NULL, NULL, &p);   // ͸������ 
				text1.putText(image, st.getPlateName().c_str(), cvPoint(region.x + 10, region.y - 10), CV_RGB(255, 0, 0));
				chepai += st.getPlateName() + "  \r";
			}

		}
		CString callpLicense(chepai.c_str());
		m_locateTime.SetWindowText(callpLicense);
		cout << chepai << endl;
		cv::namedWindow("���ͼ", WINDOW_NORMAL);
		cv::imshow("���ͼ", image);
		waitKey(0);

	}


}





/*
*
* ������ȡ�ļ�
*
*/
void CLPR_MFCDlg::OnBnClickedRecognizeButton()
{
	m_locateTime.SetWindowText(_T(" "));//��ձ༭��
	m_Edit.SetWindowText(_T(" "));//��ձ༭��
	plates.swap(vector<lpr::PlateInfo>());
	//��ȡ�ļ�
	isfilepath = true;
	TCHAR           szFolderPath[MAX_PATH] = { 0 };
	CString         strFolderPath = TEXT("");

	BROWSEINFO      sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = _T("��ѡ��һ���ļ��У�");
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	sInfo.lpfn = NULL;

	// ��ʾ�ļ���ѡ��Ի���  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// ȡ���ļ�����  
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
	CString text;//��ʾ�ı�
	m_List.clear();
	//�ݹ���ͼƬ������m_List����
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
			text += " ʶ��ʧ��";
		}
		else
		{
			text += STC.c_str();
			text += " ʶ�����";
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
	text += "д����ɣ�����ʶ�𽫽��ͼƬ�����TXT�ļ��ѱ�����";
	text += strMfc;
	text += "\r\n";
	str.Format(_T("�ɹ�ʶ����%d��ͼƬ,ʧ��%d"), m_List.size() - v_data.size(),v_data.size());
	text += str;

	GetDlgItem(IDC_EDIT1)->SetWindowTextW(text);



	if (IDYES == MessageBox(_T("�������ļ��ѱ�����") + strMfc, _T("�ɹ�,�Ƿ�򿪸�Ŀ¼"), MB_YESNO))
	{

		ShellExecuteW(NULL, NULL, _T("explorer"), L"C:\\test-results\\", NULL, SW_SHOW);
	
	}




	
	//GetDlgItem(IDC_EDIT1)->SetWindowTextW(text);

}


void CLPR_MFCDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
}


void CLPR_MFCDlg::OnEnChangeEdit1()
{

	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
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
		// ��ʾ��Ϣ�Ի���
		INT_PTR nRes;
		nRes = MessageBox(_T("�޷�������ͷ��\n���������ӣ�"), _T("��ʾ"), MB_ICONWARNING);

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
				//ʶ������
				//vector<pr::PlateInfo> ret = prc.dongtaiAsImage(frame, platesRegions[i]);
				vector<pr::PlateInfo> ret = par.dongtaiAsImage(frame, platesRegions[i]);
				string chepai = "";
				for (auto st : ret) {
					if (st.confidence > 0.9) {
						chepai += st.getPlateName() + "  ";
					}
				}

				putTextZH(frame, chepai.c_str(), Point(platesRegions[i].x + 40, platesRegions[i].y - 30), Scalar(0, 0, 255), 30, "����");
			}

		}

		imshow("video-demo", frame);
		if (waitKey(1) > 0)		// delay ms �ȴ������˳�
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}

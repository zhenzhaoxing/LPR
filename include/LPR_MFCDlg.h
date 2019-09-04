
// LPR_MFCDlg.h : 头文件
//

#pragma once
//#include "afxwin.h"
#include "Pipeline.h"

// CLPR_MFCDlg 对话框
class CLPR_MFCDlg : public CDialogEx
{
// 构造
public:
	CLPR_MFCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LPR_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	bool isfilepath;
	HICON m_hIcon;
	void BrowseCurrentAllFile(CString strDir);
	int TEST_PIPELINE(string filename, vector<lpr::PlateInfo> plates, ofstream& outFile);
	Mat matresize(cv::Mat mat, cv::Mat image);
	//车牌文件数据
	std::vector<CString> m_List;
	// 生成的消息映射函数
	CProgressCtrl* pProgCtrl;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	lpr::PipelinePR* psrc;
	CEdit m_Edit;
	//CString m_Edit;
	CString m_path;
	CStatic m_showImg;
	CEdit m_locateTime;
	afx_msg void OnBnClickedOpenButton();
	afx_msg void OnBnClickedSelectButton();
	afx_msg void OnBnClickedRecognizeButton();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEdit1();

	afx_msg void OnEnChangeEdit2();
	
	afx_msg void OnNMCustomdrawProgress4(NMHDR* pNMHDR, LRESULT* pResult);
};

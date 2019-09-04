
// LPR_MFCDlg.h : ͷ�ļ�
//

#pragma once
//#include "afxwin.h"
#include "Pipeline.h"

// CLPR_MFCDlg �Ի���
class CLPR_MFCDlg : public CDialogEx
{
// ����
public:
	CLPR_MFCDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_LPR_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	bool isfilepath;
	HICON m_hIcon;
	void BrowseCurrentAllFile(CString strDir);
	int TEST_PIPELINE(string filename, vector<lpr::PlateInfo> plates, ofstream& outFile);
	Mat matresize(cv::Mat mat, cv::Mat image);
	//�����ļ�����
	std::vector<CString> m_List;
	// ���ɵ���Ϣӳ�亯��
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

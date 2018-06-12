
// MicroAutoFocusDlg.h : 头文件
//

#pragma once
#include <uc480.h>
#include <uc480_deprecated.h>

// CMicroAutoFocusDlg 对话框
class CMicroAutoFocusDlg : public CDialogEx
{
// 构造
public:
	CMicroAutoFocusDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MICROAUTOFOCUS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtuonExit();

	// self define
	bool m_exit;
	bool m_snap;
	bool m_autofocus;

	CWinThread* pCameraThread;
//	CWinThread* pAotuFocusThread;

	CString GetDirectory();
	void ReadParam();
	void WriteParam();

	// camera varibles
	HCAM	m_hG;			// handle to frame grabber
	HWND	m_hWnd;			// handle to diplay window
	INT		m_Ret;			// return value for SDK functions
	INT		m_nColorMode;	// Y8/RGB16/RGB24/REG32
	INT		m_nBitsPerPixel;// number of bits needed store one pixel
	INT		m_nSizeX;		// width of video 
	INT		m_nSizeY;		// height of video
	INT		m_lMemoryId;	// grabber memory - buffer ID
	char*	m_pcImageMemory;// grabber memory - pointer to buffer

	CWnd *pCWnd;
	//char data[1280*1024];
	bool OpenCamera();
	void GetMaxImageSize(INT *pnSizeX, INT *pnSizeY);
	/// motor
	bool ConnectMotor();
	bool CloseMotor();
	bool ZeroMotor();
	bool MoveMotor(double distance);
	double MotorMax;
	double MotorMin;
	double m_FocusStep;
	int m_PixelLim;

	
	//////////////////////////////////////////////////////////////////////////////
	// system generates
	BOOL m_autosave;
	
	double m_motor_mm;

	int m_marker_x;
	int m_marker_y;
	int radius;
	int linewidth;
	
	int m_slider_exposure;
	int m_exposure;
//	int m_slider_zoom;
//	double m_zoom;
	afx_msg void OnNMCustomdrawSliderExposure(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditExposure();
	afx_msg void OnBnClickedButtuonStepplus();
	afx_msg void OnBnClickedButtuonStepminus();
	afx_msg void OnEnChangeEditX();
	afx_msg void OnEnChangeEditY();
	afx_msg void OnBnClickedCheckAutosave();
	CString m_FilePath;
	CString m_name;
	afx_msg void OnBnClickedButtuonSnap();
	afx_msg void OnEnChangeEditZoom();
	afx_msg void OnNMCustomdrawSliderZoom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtuonFocusPlus();
	afx_msg void OnBnClickedButtuonFocusMinus();
	afx_msg void OnEnChangeEdit();
	int m_slider_zoom;
	double m_zoom;
	double m_focus_pos;
	afx_msg void OnBnClickedButtuonFocusZero();
	afx_msg void OnBnClickedButtuonAutofocus();
	afx_msg void OnEnChangeEditSteps();
};

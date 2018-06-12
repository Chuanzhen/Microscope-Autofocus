
// MicroAutoFocusDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MicroAutoFocus.h"
#include "MicroAutoFocusDlg.h"
#include "afxdialogex.h"


#include<ZStepper_SDK.h>
#include "ImageProcess.h"

#include<opencv2/opencv.hpp>
using namespace cv;

#include <uc480.h>
#include <uc480_deprecated.h>

#define IMG_WIDTH 1280
#define IMG_HEIGHT 1024

UINT __cdecl CameraThread(LPVOID pParam);
//UINT __cdecl AutoFocusThread(LPVOID pParam);
extern char temp[1280 * 1024] = { "0" };


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CMicroAutoFocusDlg 对话框



CMicroAutoFocusDlg::CMicroAutoFocusDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMicroAutoFocusDlg::IDD, pParent)
	, m_autosave(FALSE)
	, m_exposure(10)
	, m_motor_mm(0.0002)
	, m_marker_x(100)
	, m_marker_y(100)
	, m_slider_exposure(10)
	, m_FilePath(_T(""))
	, m_name(_T(""))
	, m_slider_zoom(15)
	, m_zoom(0.5)
	, m_focus_pos(0)
{
	radius = 10;
	linewidth = 2;
	m_FocusStep = 0.0002;
	m_PixelLim = 4;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMicroAutoFocusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_AUTOSAVE, m_autosave);
	DDX_Text(pDX, IDC_EDIT_EXPOSURE, m_exposure);
	DDV_MinMaxDouble(pDX, m_exposure, 0, 100);
	DDX_Text(pDX, IDC_EDIT_STEPS, m_motor_mm);
	DDV_MinMaxDouble(pDX, m_motor_mm, 0, 1);
	DDX_Text(pDX, IDC_EDIT_X, m_marker_x);
	DDX_Text(pDX, IDC_EDIT_Y, m_marker_y);
	DDX_Slider(pDX, IDC_SLIDER_EXPOSURE, m_slider_exposure);
	DDV_MinMaxInt(pDX, m_slider_exposure, 0, 100);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, m_FilePath);
	DDX_Text(pDX, IDC_EDIT__NAME, m_name);
	DDX_Slider(pDX, IDC_SLIDER_ZOOM, m_slider_zoom);
	DDV_MinMaxInt(pDX, m_slider_zoom, 0, 100);
	DDX_Text(pDX, IDC_EDIT_ZOOM, m_zoom);
	DDV_MinMaxDouble(pDX, m_zoom, 0.2, 2.2);
	DDX_Text(pDX, IDC_EDIT_POS, m_focus_pos);
}

BEGIN_MESSAGE_MAP(CMicroAutoFocusDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BUTTUON_EXIT, &CMicroAutoFocusDlg::OnBnClickedButtuonExit)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_EXPOSURE, &CMicroAutoFocusDlg::OnNMCustomdrawSliderExposure)
	ON_EN_CHANGE(IDC_EDIT_EXPOSURE, &CMicroAutoFocusDlg::OnEnChangeEditExposure)
	ON_BN_CLICKED(ID_BUTTUON_STEPPLUS, &CMicroAutoFocusDlg::OnBnClickedButtuonStepplus)
	ON_BN_CLICKED(ID_BUTTUON_STEPMINUS, &CMicroAutoFocusDlg::OnBnClickedButtuonStepminus)
	ON_EN_CHANGE(IDC_EDIT_X, &CMicroAutoFocusDlg::OnEnChangeEditX)
	ON_EN_CHANGE(IDC_EDIT_Y, &CMicroAutoFocusDlg::OnEnChangeEditY)
	ON_BN_CLICKED(IDC_CHECK_AUTOSAVE, &CMicroAutoFocusDlg::OnBnClickedCheckAutosave)
	ON_BN_CLICKED(ID_BUTTUON_SNAP, &CMicroAutoFocusDlg::OnBnClickedButtuonSnap)
	ON_EN_CHANGE(IDC_EDIT_ZOOM, &CMicroAutoFocusDlg::OnEnChangeEditZoom)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_ZOOM, &CMicroAutoFocusDlg::OnNMCustomdrawSliderZoom)
	ON_BN_CLICKED(ID_BUTTUON_FOCUS_PLUS, &CMicroAutoFocusDlg::OnBnClickedButtuonFocusPlus)
	ON_BN_CLICKED(ID_BUTTUON_FOCUS_MINUS, &CMicroAutoFocusDlg::OnBnClickedButtuonFocusMinus)
	ON_EN_CHANGE(IDC_EDIT__NAME, &CMicroAutoFocusDlg::OnEnChangeEdit)
	ON_BN_CLICKED(ID_BUTTUON_FOCUS_ZERO, &CMicroAutoFocusDlg::OnBnClickedButtuonFocusZero)
	ON_BN_CLICKED(ID_BUTTUON_AUTOFOCUS, &CMicroAutoFocusDlg::OnBnClickedButtuonAutofocus)
	ON_EN_CHANGE(IDC_EDIT_STEPS, &CMicroAutoFocusDlg::OnEnChangeEditSteps)
END_MESSAGE_MAP()


// CMicroAutoFocusDlg 消息处理程序

BOOL CMicroAutoFocusDlg::OnInitDialog()
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
	ReadParam();
	// motor
	if (ConnectMotor() != true)
	{ 
		AfxMessageBox(L"Can't open focus motor!");
		OnCancel();
		return TRUE;
	}


	// create windows
	Mat img(Size(IMG_WIDTH * m_zoom, IMG_HEIGHT * m_zoom), CV_8UC3);
	namedWindow("LIVE", CV_WINDOW_AUTOSIZE);
	img.setTo(Scalar(255, 0, 0));
	imshow("LIVE", img);
	waitKey(100);

	HWND hShowWnd = (HWND)cvGetWindowHandle("LIVE");
	m_hWnd = hShowWnd;
	pCWnd = FromHandle(m_hWnd);
	// Camera 
	m_nColorMode = IS_SET_CM_Y8;
	m_nBitsPerPixel = 8;
	m_exit = false;
	m_hG = 0; //camera ID equals 0
	m_snap = false;
	m_autofocus = false;

	while (OpenCamera() != true)
	{
		int m = AfxMessageBox(L"fail to open camera!", MB_RETRYCANCEL);
		if (m == IDRETRY)
		{
			OpenCamera();
		}
		else
		{
			cvDestroyAllWindows();
			OnCancel();
			return TRUE;
		}
	}
	

	pCameraThread = AfxBeginThread((AFX_THREADPROC)CameraThread, this, 0U, 0UL, 0, 0);
	//*/

	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMicroAutoFocusDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMicroAutoFocusDlg::OnPaint()
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
HCURSOR CMicroAutoFocusDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMicroAutoFocusDlg::OnBnClickedButtuonExit()
{
	// TODO:  在此添加控件通知处理程序代码
	m_exit = true;

	WaitForSingleObject(pCameraThread->m_hThread, INFINITE);
	Sleep(100);

	is_FreeImageMem(m_hG, m_pcImageMemory, m_lMemoryId);
	is_ExitCamera(m_hG);
	m_hG = NULL;
	cvDestroyWindow("LIVE");
	WriteParam();
	
	CloseMotor();
	CDialog::OnCancel();
}


bool CMicroAutoFocusDlg::OpenCamera()
{
	if (m_hG != 0)
	{
		//free old image mem.
		is_FreeImageMem(m_hG, m_pcImageMemory, m_lMemoryId);
		is_ExitCamera(m_hG);
	}

	// init camera
	m_hG = (HCAM)0;							// open next camera
	m_Ret = is_InitCamera(&m_hG, NULL);		// init camera - no window handle for live required

	if (m_Ret == IS_SUCCESS)
	{
		// retrieve original image size
		SENSORINFO sInfo;
		is_GetSensorInfo(m_hG, &sInfo);

		GetMaxImageSize(&m_nSizeX, &m_nSizeY);

		// setup the color depth to the current windows setting
		//is_GetColorDepth(m_hG, &m_nBitsPerPixel, &m_nColorMode);
		is_SetColorMode(m_hG, m_nColorMode);

		// memory initialization
		is_AllocImageMem(m_hG,
			m_nSizeX,
			m_nSizeY,
			m_nBitsPerPixel,
			&m_pcImageMemory,
			&m_lMemoryId);
		is_SetImageMem(m_hG, m_pcImageMemory, m_lMemoryId);	// set memory active


		// display initialization
		is_SetImageSize(m_hG, m_nSizeX, m_nSizeY);
		is_SetDisplayMode(m_hG, IS_SET_DM_DIB);

		/*
		// enable the dialog based error report
		m_Ret = is_SetErrorReport(m_hG, IS_ENABLE_ERR_REP); //IS_DISABLE_ERR_REP);
		if( m_Ret != IS_SUCCESS )
		{
		AfxMessageBox( "ERROR: Can not enable the automatic error report!" , MB_ICONEXCLAMATION, 0 );
		return false;
		}
		*/
	}
	else
	{
		AfxMessageBox(L"ERROR: Can not open a camera!", MB_ICONEXCLAMATION, 0);
		return false;
	}

	return true;
}

void CMicroAutoFocusDlg::GetMaxImageSize(INT *pnSizeX, INT *pnSizeY)
{
	// Check if the camera supports an arbitrary AOI
	INT nAOISupported = 0;
	BOOL bAOISupported = TRUE;
	if (is_ImageFormat(m_hG,
		IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED,
		(void*)&nAOISupported,
		sizeof(nAOISupported)) == IS_SUCCESS)
	{
		bAOISupported = (nAOISupported != 0);
	}

	if (bAOISupported)
	{
		// Get maximum image size
		SENSORINFO sInfo;
		is_GetSensorInfo(m_hG, &sInfo);
		*pnSizeX = sInfo.nMaxWidth;
		*pnSizeY = sInfo.nMaxHeight;
	}
	else
	{
		// Get image size of the current format
		*pnSizeX = is_SetImageSize(m_hG, IS_GET_IMAGE_SIZE_X, 0);
		*pnSizeY = is_SetImageSize(m_hG, IS_GET_IMAGE_SIZE_Y, 0);
	}

}

void CMicroAutoFocusDlg::OnNMCustomdrawSliderExposure(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_exposure = m_slider_exposure;
	UpdateData(FALSE);
	*pResult = 0;
}


void CMicroAutoFocusDlg::OnEnChangeEditExposure()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	UpdateData(TRUE);
	if (m_exposure < 0)
	{
		m_exposure = 0;
	}
	else if (m_exposure > 100)
	{
		m_exposure = 100;
	}
	m_slider_exposure = m_exposure;
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void CMicroAutoFocusDlg::OnEnChangeEditZoom()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_zoom < 0.2)
	{
		m_zoom = 0.2;
	}
	else if (m_zoom > 2.2)
	{
		m_zoom = 2.2;
	}
	m_slider_zoom = int(m_zoom * 50 - 10);

	UpdateData(FALSE);
}


void CMicroAutoFocusDlg::OnNMCustomdrawSliderZoom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_zoom = (double(m_slider_zoom) / 50 + 0.2);
	UpdateData(FALSE);
	*pResult = 0;
}



void CMicroAutoFocusDlg::OnBnClickedButtuonStepplus()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_motor_mm = m_motor_mm * 10;
	if (m_motor_mm < 1e-6)
	{
		m_motor_mm = 1e-6;
	}
	else if (m_motor_mm > 1)
	{
		m_motor_mm = 1;
	}
	UpdateData(FALSE);
}


void CMicroAutoFocusDlg::OnBnClickedButtuonStepminus()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_motor_mm = m_motor_mm / 10;
	if (m_motor_mm < 1e-6)
	{
		m_motor_mm = 1e-6;
	}
	else if (m_motor_mm > 1)
	{
		m_motor_mm = 1;
	}
	UpdateData(FALSE);
}


void CMicroAutoFocusDlg::OnEnChangeEditX()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	UpdateData(FALSE);
}


void CMicroAutoFocusDlg::OnEnChangeEditY()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	UpdateData(TRUE);
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}


void CMicroAutoFocusDlg::OnBnClickedCheckAutosave()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_autosave == TRUE)
	{
		if (AfxMessageBox(L"Use default path?", MB_YESNO | MB_ICONINFORMATION) == IDNO)
		{
			m_FilePath = GetDirectory();
			WritePrivateProfileString(L"CAMERA", L"FolderPath", m_FilePath, L".\\Config.ini");
		}
	}
	
	UpdateData(FALSE);
}

CString CMicroAutoFocusDlg::GetDirectory()
{
	CString m_strFolder = L"";
	TCHAR serverPath[200];
	BROWSEINFO bi;
	LPITEMIDLIST pid;

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = serverPath;
	bi.lpszTitle = _T("选择输出文件路径");
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = NULL;
	bi.iImage = NULL;
	if ((pid = SHBrowseForFolder(&bi)) != NULL)
	{
		if (SUCCEEDED(SHGetPathFromIDList(pid, serverPath))) //得到文件夹的全路径，不要的话，只得本文件夹名  
		{
			CString str(L"\\");
			str = serverPath + str;
			return str;
		}
	}

	return L"";
}


void CMicroAutoFocusDlg::ReadParam()
{
	TCHAR szKeyValue[200] = { 0 };
	;; GetPrivateProfileString(L"CAMERA", L"FolderPath", NULL, szKeyValue, 200, L".\\Config.ini");
	m_FilePath = szKeyValue;
	m_exposure = GetPrivateProfileInt(L"CAMERA", L"EXPOSURE", 0, L".\\Config.ini");
	m_slider_exposure = m_exposure;
	m_marker_x = GetPrivateProfileInt(L"CAMERA", L"MARKER_X", 0, L".\\Config.ini");
	m_marker_y = GetPrivateProfileInt(L"CAMERA", L"MARKER_Y", 0, L".\\Config.ini");

	radius = GetPrivateProfileInt(L"MARKER", L"RADIUS", 0, L".\\Config.ini");
	linewidth = GetPrivateProfileInt(L"MARKER", L"LINEWIDTH", 0, L".\\Config.ini");

	int temp = GetPrivateProfileInt(L"FOCUS", L"FOCUSSTEP", 0, L".\\Config.ini");
	m_FocusStep = double(temp) / 1e6;
	m_PixelLim = GetPrivateProfileInt(L"FOCUS", L"FOCUSLIM", 0, L".\\Config.ini");
	UpdateData(FALSE);
}


void CMicroAutoFocusDlg::WriteParam()
{
	WritePrivateProfileString(L"CAMERA", L"FolderPath", m_FilePath, L".\\Config.ini");

	CString str = L"";
	str.Format(L"%d", m_exposure);
	WritePrivateProfileString(L"CAMERA", L"EXPOSURE", str, L".\\Config.ini");
	str.Format(L"%d", m_marker_x);
	WritePrivateProfileString(L"CAMERA", L"MARKER_X", str, L".\\Config.ini");
	str.Format(L"%d", m_marker_y);
	WritePrivateProfileString(L"CAMERA", L"MARKER_Y", str, L".\\Config.ini");
	
	str.Format(L"%d", 0);
	WritePrivateProfileString(L"FOCUS", L"AutoFocus", str, L".\\Config.ini");
	//cv::polyfit
}


UINT __cdecl CameraThread(LPVOID pParam)
{
	CMicroAutoFocusDlg *p_this = (CMicroAutoFocusDlg  *)pParam;

	double exposure = 0;
	double zoom = 0;
	IplImage *head = cvCreateImageHeader(cvSize(1280, 1024), 8, 1);
	Mat frame, fshow;
	double m_radius = 1000;
	double m_LabviewFocus = 1000;
	int numMarker = 5;

	IMAGE_FILE_PARAMS ImageFileParams;
	UINT mID = UINT(p_this->m_lMemoryId);
	ImageFileParams.pnImageID = &mID; // valid ID
	ImageFileParams.ppcImageMem = &p_this->m_pcImageMemory; // valid buffer
	ImageFileParams.nFileType = IS_IMG_PNG;
	ImageFileParams.nQuality = 100;

	//namedWindow("TEST", CV_WINDOW_NORMAL);

	is_SetGainBoost(p_this->m_hG, IS_SET_GAINBOOST_ON);

	while (p_this->m_exit == false)
	{
		//INT is_Exposure (HIDS hCam, UINT nCommand, void* pParam, UINT cbSizeOfParam)
		if (exposure != p_this->m_exposure)
		{
			exposure = p_this->m_exposure;
			is_Exposure(p_this->m_hG, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposure, 8);

		}
		is_FreezeVideo(p_this->m_hG, IS_WAIT);
		//is_RenderBitmap(p_this->m_hG, p_this->m_lMemoryId, p_this->m_hWnd, IS_RENDER_FIT_TO_WINDOW);

		cvSetData(head, p_this->m_pcImageMemory, 1280);
		frame = cvarrToMat(head);
		/////////////
		fshow = frame.clone();
		std::vector<Mat> bgr;
		bgr.clear();
		bgr.push_back(fshow);
		bgr.push_back(fshow);
		bgr.push_back(fshow);
		cv::merge(bgr, fshow);
		
		circle(fshow, Point(p_this->m_marker_x, p_this->m_marker_y), p_this->radius, Scalar(0, 0, 255), p_this->linewidth, 8);
		////////////////////////////////////// focus
		if (p_this->m_autofocus == true)
		{
			if (m_radius == 1000)
			{
				m_radius = findMinRadius(frame);
			}
			else
			{
				double temp = findMinRadius(frame);
				if ((m_radius - temp) > p_this->m_PixelLim) // go up
				{
					p_this->MoveMotor(p_this->m_focus_pos + p_this->m_FocusStep);
				}
				else if ((temp - m_radius) > p_this->m_PixelLim) // go down
				{
					p_this->MoveMotor(p_this->m_focus_pos - p_this->m_FocusStep);
				}
				putText(fshow, std::to_string(m_radius), cvPoint(300, 270), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
				putText(fshow, std::to_string(temp), cvPoint(300, 300), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
				m_LabviewFocus = temp;
			}
		}
		else
		{
			m_radius = 1000;
			numMarker = 5;
		}
		///////////////////////////////////////////////////////
		if (1 == GetPrivateProfileInt(L"FOCUS", L"AutoFocus", 0, L".\\Config.ini"))
		{
			if (numMarker == 5)
			{
				if (p_this->m_autofocus == true)
				{
					p_this->m_autofocus = false;
	
				}	
				m_radius = m_LabviewFocus;;

			}
			else
			{
				numMarker = numMarker - 1;
			}
			
			if (numMarker < 0)
			{
				double temp = findMinRadius(frame);
				if ((m_radius - temp) > p_this->m_PixelLim) // inverse go up
				{
					p_this->MoveMotor(p_this->m_focus_pos - 6*p_this->m_FocusStep);
					numMarker = numMarker + 4;
				}
				else if ((temp - m_radius) > p_this->m_PixelLim) //inverse  go down
				{
					p_this->MoveMotor(p_this->m_focus_pos + 6*p_this->m_FocusStep);
					numMarker = numMarker + 4;
				}
				else if (abs(temp - m_radius) <= p_this->m_PixelLim)
				{
					CString str = L"";
					str.Format(L"%d", 0);
					WritePrivateProfileString(L"FOCUS", L"AutoFocus", str, L".\\Config.ini");
					m_radius = 1000;
					numMarker = 5;
				}
				putText(fshow, std::to_string(m_LabviewFocus), cvPoint(300, 270), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
				putText(fshow, std::to_string(temp), cvPoint(300, 300), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));

			}
			else
			{

				double temp = findMinRadius(frame);
				if ((m_radius - temp) > p_this->m_PixelLim) // go up
				{
					p_this->MoveMotor(p_this->m_focus_pos + p_this->m_FocusStep);
				}
				else if ((temp - m_radius) > p_this->m_PixelLim) // go down
				{
					p_this->MoveMotor(p_this->m_focus_pos - p_this->m_FocusStep);
				}
				else if (abs(temp - m_radius) <= p_this->m_PixelLim)
				{
					CString str = L"";
					str.Format(L"%d", 0);
					WritePrivateProfileString(L"FOCUS", L"AutoFocus", str, L".\\Config.ini");
					m_radius = 1000;
					numMarker = 5;
				}
				putText(fshow, std::to_string(m_LabviewFocus), cvPoint(300, 270), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
				putText(fshow, std::to_string(temp), cvPoint(300, 300), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
			}
			
			
		}
		// opencv////////////////////////////////////////////////////////////////////////
		if (p_this->m_snap == true)
		{
			imwrite("marker.png", fshow);
			p_this->m_snap = false;

			if (p_this->m_autosave == TRUE)
			{
				time_t num;
				time(&num);
				char strtemp[20];
				sprintf_s(strtemp, "%d", num);
				CString TempTime(strtemp);
				CString Temp;

				if (p_this->m_name == L"")
				{
					Temp = p_this->m_FilePath + TempTime + L".PNG";
				}
				else
				{
					Temp = p_this->m_FilePath + p_this->m_name + L"_" + TempTime + L".PNG";;
				}
				// AfxMessageBox(Temp);
				string str(CW2A(Temp.GetString()));
				imwrite(str, frame);
			}
			else
			{
				ImageFileParams.pwchFileName = NULL;
				is_ImageFile(p_this->m_hG, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams));
			}

		}
		////////////////////////////////////////////////////////////////////

		resize(fshow, fshow, Size(int(IMG_WIDTH*p_this->m_zoom), int(IMG_HEIGHT*p_this->m_zoom)));
		imshow("LIVE", fshow);
		

	}
	return 0;
}

void CMicroAutoFocusDlg::OnBnClickedButtuonSnap()
{
	// TODO:  在此添加控件通知处理程序代码
	if (m_snap == false)
	{
		m_snap = true;
	}
}




void CMicroAutoFocusDlg::OnBnClickedButtuonFocusPlus()
{
	// TODO:  在此添加控件通知处理程序代码
	if (MoveMotor(m_focus_pos + m_motor_mm) != true)
	{
		AfxMessageBox(L"Wrong to go up!");
	}

}


void CMicroAutoFocusDlg::OnBnClickedButtuonFocusMinus()
{
	// TODO:  在此添加控件通知处理程序代码
	if (MoveMotor(m_focus_pos - m_motor_mm) != true)
	{
		AfxMessageBox(L"Wrong to go down!");
	}
}


void CMicroAutoFocusDlg::OnEnChangeEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	UpdateData(FALSE);
}


bool CMicroAutoFocusDlg::ConnectMotor()
{
	long deviceCount = 0;

	if (false == FindDevices(deviceCount))
	{
		return false;
	}

	if (false == SelectDevice(0))
	{
		return false;
	}

	long paramType = 0;
	long paramAvail = 0;
	long paramReadOnly = 0;
	MotorMin = 0;
	MotorMax = 0;
	double notcare = 0;

	// get the range for the position parameter
	if (false == GetParamInfo(PARAM_Z_POS, paramType, paramAvail, paramReadOnly, MotorMin, MotorMax, notcare))
	{
		return false;
	}

	//long velocity = 256;
	//if(false == SetParam(PARAM_Z_VELOCITY, static_cast<double> (velocity)))
	//{
	//	return 0;
	//}

	//long stepPerMM = 100;
	//if(false == SetParam(PARAM_Z_STEPS_PER_MM, static_cast<double>(stepPerMM)))
	//{
	//	return 0;
	//}

	if (false == ZeroMotor())
	{
		return false;
	}
	///////////////////
	
	UpdateData(FALSE);
	return true;
}
bool CMicroAutoFocusDlg::ZeroMotor()
{
	long setZero = 1;
	if (false == SetParam(PARAM_Z_ZERO, static_cast<double>(setZero)))
	{
		return false;
	}
	/////////////////////
	if (false == PreflightPosition())
	{
		return false;
	}

	if (false == SetupPosition())
	{
		return false;
	}

	if (false == StartPosition())
	{
		return false;
	}

	long status = STATUS_READY;
	do
	{
		if (false == StatusPosition(status))
		{
			return false;
		}
	} while (status == STATUS_BUSY);

	if (false == ReadPosition(STAGE_Z, m_focus_pos))
	{
		return false;
	}
	if (false == PostflightPosition())
	{
		return false;
	}
	UpdateData(FALSE);
	return true;
}

bool CMicroAutoFocusDlg::CloseMotor()
{
	if (false == TeardownDevice())
	{
		return false;
	}
	return true;
}
bool CMicroAutoFocusDlg::MoveMotor(double distance)
{
	if (false == SetParam(PARAM_Z_POS, static_cast<double>(distance)))
	{
		return false;
	}

	///////////////////////////////////////////

	if (false == PreflightPosition())
	{
		return false;
	}

	if (false == SetupPosition())
	{
		return false;
	}

	if (false == StartPosition())
	{
		return false;
	}

	long status = STATUS_READY;
	do
	{
		if (false == StatusPosition(status))
		{
			return false;
		}
	} while (status == STATUS_BUSY);


	if (false == ReadPosition(STAGE_Z, m_focus_pos))
	{
		return false;
	}
	UpdateData(FALSE);

	if (false == PostflightPosition())
	{
		return false;
	}
	/////////////////////////////////////////////////////
	return true;
}

void CMicroAutoFocusDlg::OnBnClickedButtuonFocusZero()
{
	// TODO:  在此添加控件通知处理程序代码
	//m_focus_pos = 0;
	
	if (false ==  ZeroMotor())
	{
		AfxMessageBox(L"Zero wrong");
	}
	
	/*
	if (MoveMotor(m_focus_pos) != true)
	{
		AfxMessageBox(L"Wrong to go ZERO!");
	}
	*/
	UpdateData(FALSE);
}


void CMicroAutoFocusDlg::OnBnClickedButtuonAutofocus()
{
	// TODO:  在此添加控件通知处理程序代码
	if (m_autofocus == false)
	{
		m_autofocus = true;
		//GetDlgItem(ID_BUTTUON_AUTOFOCUS)->EnableWindow(FALSE);
		SetDlgItemText(ID_BUTTUON_AUTOFOCUS, L"StopFocus");
	}
	else if (m_autofocus == true)
	{
		m_autofocus = false;
		SetDlgItemText(ID_BUTTUON_AUTOFOCUS, L"AutoFocus");
	}
}


void CMicroAutoFocusDlg::OnEnChangeEditSteps()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	UpdateData(TRUE);

}

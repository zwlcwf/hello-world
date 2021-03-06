
// CalSatPosDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "CalSatPos.h"
#include "CalSatPosDlg.h"
#include "afxdialogex.h"

#include "head_filenm.h"
#include "CalSatCom.h"
#include "cal.h"
#include "cal1.h"
#include "AstroCommon.h"

#define EXT 
#include "sgp.h"

void Cal2Body(TleData *tle, double tsince, double *ro, double *vo);


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCalSatPosDlg ダイアログ



CCalSatPosDlg::CCalSatPosDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CALSATPOS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_dsp_no = 0;
	m_dsp_sname = false;

	m_line_orb = false;

	m_del_min = 1;
	m_cal_method = 1;
	//m_cal_method=0;

	for (int ii = 0; ii<N_SAT; ii++)
	{
		m_flg1[ii] = 0;
		m_flg2[ii] = 0;
		m_dist[ii] = 0;
	}
	m_nsh = 0;

	m_eps = 23.445787; // 1950年分点

}

void CCalSatPosDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list_ctrl1);
	DDX_Control(pDX, IDC_PCT, m_pct);
}

BEGIN_MESSAGE_MAP(CCalSatPosDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCalSatPosDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCalSatPosDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CCalSatPosDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_NXT, &CCalSatPosDlg::OnBnClickedBtnNxt)
	ON_BN_CLICKED(IDC_BTN_PRV, &CCalSatPosDlg::OnBnClickedBtnPrv)
	ON_BN_CLICKED(IDC_BTN_ANIMA, &CCalSatPosDlg::OnBnClickedBtnAnima)
END_MESSAGE_MAP()


// CCalSatPosDlg メッセージ ハンドラー

BOOL CCalSatPosDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

//	ShowWindow(SW_MAXIMIZE);

//	ShowWindow(SW_MINIMIZE);

	// TODO: 初期化をここに追加します。

	// 見出しの作成
	TCHAR       list_title[][32] = { _T("衛星名"), _T("経度(°)"), _T("緯度(°)"),_T("高度(km)"),_T("方位(°)"),_T("高度（°）") };
	const int   clmNum = sizeof(list_title) / sizeof(list_title[0]);
	LVCOLUMN    lvc;
	memset(&lvc, 0, sizeof(lvc));
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;    // 有効フラグ



	for (int ii = 0; ii < clmNum; ii++)
	{
		lvc.iSubItem = ii;            // サブアイテム番号
		lvc.pszText = list_title[ii];;   // 見出しテキスト
		lvc.cx = 100;          // 横幅
		if (m_list_ctrl1.InsertColumn(ii, &lvc) == -1)
		{
			break;
		}
	}
	CRect rect_dlg;
	GetClientRect(&rect_dlg);

	CRect lctrlRect;
	m_list_ctrl1.GetWindowRect(&lctrlRect);
	double list_h = lctrlRect.Height();
	double list_w = lctrlRect.Width();
	m_list_ctrl1.MoveWindow(X_TBL_SAT, Y_TBL_SAT, list_w, 100, 1);


	m_longt[0] = 0.0;
	m_lat[0] = 0.0;
	m_h[0] = 0.0;
	m_azm[0] = 0.0;
	m_height[0] = 0.0;

	m_lat_deg_sgp[0] = 0.0;
	m_log_deg_sgp[0] = 0.0;
	m_h_sgp[0] = 0.0;
	m_azimth_sgp[0] = 0.0;
	m_height_sgp[0] = 0.0;

	m_decl_moon = 0.0;  ///< 月の赤緯
	m_ra_moon = 0.0;  ///< 月の赤経
	m_azm_moon = 0.0;  ///< 月の方位
	m_h_moon = 0.0;  ///< 月の高さ

	m_decl_sun = 0.0;  ///< 太陽の赤緯
	m_ra_sun = 0.0;  ///< 太陽の赤経
	m_azm_sun = 0.0;  ///< 太陽の方位
	m_h_sun = 0.0;  ///< 太陽の高さ



	m_fileName[0] = NULL;


	//	TCHAR cDir[MAX_PATH];
	//	::GetCurrentDirectory(MAX_PATH,cDir);



	::GetPrivateProfileString(_T("URL"), _T("GPS_SRV"), _T("celestrak.com"), theApp.m_gps_srv_str, MAX_URL_STR, _T("sat.ini"));
	::GetPrivateProfileString(_T("URL"), _T("GPS_URL"), _T("http://celestrak.com/NORAD/elements/gps-ops.txt"),
		theApp.m_gps_url_str, MAX_URL_STR, _T("sat.ini"));

	::GetPrivateProfileString(_T("URL"), _T("ISS_SRV"), _T("celestrak.com"), theApp.m_iss_srv_str, MAX_URL_STR, _T("sat.ini"));
	::GetPrivateProfileString(_T("URL"), _T("ISS_URL"), _T("http://celestrak.com/NORAD/elements/stations.txt"),
		theApp.m_iss_url_str, MAX_URL_STR, _T("sat.ini"));

	TCHAR currentdir[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, currentdir);
	_tcscat_s(currentdir, MAX_PATH, _T("\\sat.ini"));

	TCHAR exedir[MAX_PATH];
	TCHAR inidir[MAX_PATH];
	::GetModuleFileName(NULL, exedir, MAX_PATH);

	//exeファイルのパスを分解
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath_s(exedir, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	//引数のファイル名をexeファイルのパスでフルパス化
	_tmakepath_s(inidir, MAX_PATH, drive, dir, _T(""), _T(""));

	_tcscat_s(inidir, _T("sat.ini"));

	::GetPrivateProfileStringW(_T("PATH"), _T("TLE"), _T("."), m_TLEPath, MAX_URL_STR, inidir);

	::GetPrivateProfileString(_T("PATH"), _T("3DCO"), _T("."), m_CoPath, MAX_URL_STR, inidir);

	// =0 軌道によりモデルを自動切り替え =1 モデルと軌道が適切でない場合はエラー それ以外　m_calc_kindで指定したモデル 
	theApp.m_mode = ::GetPrivateProfileInt(_T("CALC"), _T("MODE"), 0, inidir);
	//  摂動モデル =1 sgp =2 sgp4 =3 sdp4
	theApp.m_calc_kind = ::GetPrivateProfileInt(_T("CALC"), _T("KIND"), 3, inidir);


	//ttttttttttttttttt
	//m_nsh, m_shore,
	FILE *fp = NULL;
	int err = _tfopen_s(&fp,_T("wshore.dat"), _T("r"));

	if (err==0)
	{
		_ftscanf_s(fp, _T("%d "), &m_nsh);

		m_shore = new SHR_CO[m_nsh];

		double xx, yy;
		for (int ii = 0; ii<m_nsh; ii++)
		{
			_ftscanf_s(fp, _T(" %lf ,%lf , "), &xx, &yy);
			m_shore[ii].x = xx;
			m_shore[ii].y = yy;
		}

		fclose(fp);
	}
	else {
		m_nsh = 0;
	}

	//ttttttttttt
	m_ang_unit = 1;

	m_mjd1 = -1;

	CTime tm;

	m_jst_utc = 0;

	//
	tm = CTime::GetCurrentTime();

#if 1
	m_year = tm.GetYear();
	m_month = tm.GetMonth();
	m_day = tm.GetDay();
	m_hour = tm.GetHour() - 9.0;
	m_min = tm.GetMinute();
	m_sec = tm.GetSecond();
#else
	m_year = 999999;
	m_month = 0;
	m_day = 0;
	m_hour = 0;
	m_min = 0;
	m_sec = 0;
#endif


	m_lambda = 0;
	m_phi = 0;
	// 水戸
	m_phi = 36.37;
	m_lambda = 140.47;
	// 札幌
	m_phi = 43.06;
	m_lambda = 141.35;
	// おおみか
	m_phi = 36.5;
	m_lambda = 140.6;

	// 新潟
	m_phi = 37.91;
	m_lambda = 139.03;

	m_mjd1 = mjd(m_year, m_month, m_day, m_hour, m_min, m_sec);

	this->Invalidate();

	DWORD ret;
	TCHAR exe_path[MAX_PATH];
	TCHAR chg_path[MAX_PATH];
	ret = GetModuleFileName(NULL, (LPWCH)exe_path, sizeof(exe_path));

	TCHAR *ch = _tcsrchr(exe_path, '\\');

	if (ch != NULL)
	{
		int n1 = wcslen(exe_path);
		int n2 = wcslen(ch);
		memset(chg_path, NULL, sizeof(chg_path));
		memcpy(chg_path, exe_path, n1 - n2);

		::SetCurrentDirectory((LPWCH)chg_path);

		//ttttttttttttttt
		if (theApp.m_prg_mode == 1)
		{
			//-MENU_RFTLE();
		}
		//ttttttttttt
	}



	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}


int CCalSatPosDlg::DrawCntrl(CDC *memDC)
{
	double longt;
	double lat;
	double alt;
	double hh;
	double azm;
	TCHAR str1[130];
	TCHAR str2[130];
	TCHAR str_dsp[256];


	double longt_sgp;
	double lat_sgp;
	double alt_sgp;
	double hh_sgp;
	double azm_sgp;

	int d1, m1, s1;
	int d2, m2, s2;
	int d3, m3, s3;

	int n_cb = 2;
	double h_cbody[2];
	double azm_cbody[2];

	h_cbody[0] = m_h_sun;
	azm_cbody[0] = m_azm_sun;
	h_cbody[1] = m_h_moon;
	azm_cbody[1] = m_azm_moon;

	CPosSatMng *sat_pos_mng;

	if (m_line_orb)
	{
		//-sat_pos_mng = m_sat_pos_mng;
	}
	else {
		sat_pos_mng = NULL;
	}


	if (theApp.m_calc_kind == -1)
	{ // ２体問題
		DrawSatCtrl(memDC, m_dsp_sname, m_dsp_no, 1.0, RAD, X_O, Y_O,
			m_n, sat_name_tbl, m_nsh, m_shore,
			m_longt, m_lat, m_height, m_azm, sat_pos_mng,
			n_cb, h_cbody, azm_cbody);

		//longt=m_longt[m_dsp_no];
		//lat=m_lat[m_dsp_no];
		//alt=m_h[m_dsp_no];
		//hh=m_height[m_dsp_no];
		//azm=m_azm[m_dsp_no];

		//	swprintf(str1,_T("経度　%f, %f,: 緯度 %f : 高度%f | 高度　方位　%f, %f\n"),
		//	longt,longt-360.0, lat,
		//	alt,hh,azm);
		//	
		//	memDC->TextOut(10,20,str1);

	}
	else
	{ // sgp モデル
		DrawSatCtrl(memDC, m_dsp_sname, m_dsp_no, 1.0, RAD, X_O, Y_O,
			m_n, sat_name_tbl, m_nsh, m_shore,
			m_log_deg_sgp, m_lat_deg_sgp, m_height_sgp, m_azimth_sgp, sat_pos_mng,
			n_cb, h_cbody, azm_cbody);
	}
	//---s----
	for (int jj = 0; jj<m_n; jj++)
	{
		//ttttttttttttt 2014/5/14
		if (theApp.m_calc_kind == -1)
		{ // 2体問題
			longt_sgp = m_longt[jj];
			lat_sgp = m_lat[jj];
			alt_sgp = m_h[jj];
			hh_sgp = m_height[jj];
			azm_sgp = m_azm[jj];
		}
		else { // 摂動モデル
			longt_sgp = m_log_deg_sgp[jj];
			lat_sgp = m_lat_deg_sgp[jj];
			alt_sgp = m_h_sgp[jj];
			hh_sgp = m_height_sgp[jj];
			azm_sgp = m_azimth_sgp[jj];
		}


		//tttttttttt
		if (!m_ang_unit)
		{

			swprintf_s(str2, _T("経度　%f, %f,: 緯度 %f : 高度%f | 高度　方位　%f, %f\n"),
				longt_sgp, longt_sgp - 360.0, lat_sgp,
				alt_sgp, hh_sgp, azm_sgp);
			memDC->TextOut(10, 70, str2);
		}
		else {
			if (longt_sgp<0.0)
			{
				longt_sgp += 360.0;
			}
			if (longt_sgp>180.0)
			{
				longt_sgp = longt_sgp - 360.0;
			}
			to_dms(longt_sgp, &d1, &m1, &s1);
			to_dms(lat_sgp, &d3, &m3, &s3);
			to_dms(longt_sgp - 360.0, &d2, &m2, &s2);

			if (this->m_dsp_no == jj)
			{
				swprintf_s(str_dsp, _T("経度 %d°%d′:緯度 %d°%d′高度%6.2f 方位%6.2f, 高度%6.2f\n"),
					d1, m1, d3, m3,
					alt_sgp, azm_sgp, hh_sgp);
				memDC->TextOut(X_SEL_SAT, Y_SEL_SAT, str_dsp); // 経度、緯度、高度、方位tem
			}
			//swprintf(str,_T("経度　%d°%d′%d″: 緯度 %d°%d′%d″ 高度%f | 高度　方位　%f, %f\n"),
			//	d1,m1,s1, d3,m3,s3,
			//	alt_sgp,hh_sgp,azm_sgp);
		}
		CString str_wrt;
		str_wrt.Format(_T("%d°%d′%d\" "), d1, m1, s1);
		m_list_ctrl1.SetItemText(jj, 1, str_wrt);
		str_wrt.Format(_T("%d°%d′%d\" "), d3, m3, s3);
		m_list_ctrl1.SetItemText(jj, 2, str_wrt);
		str_wrt.Format(_T("%f "), alt_sgp);
		m_list_ctrl1.SetItemText(jj, 3, str_wrt);
		str_wrt.Format(_T("%f "), azm_sgp);
		m_list_ctrl1.SetItemText(jj, 4, str_wrt);
		str_wrt.Format(_T("%f "), hh_sgp);
		m_list_ctrl1.SetItemText(jj, 5, str_wrt);
	}
	//----e---
	return 0;

}





void CCalSatPosDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CCalSatPosDlg::OnPaint()
{

	TCHAR str1[130];
	TCHAR str2[130];

	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト
		CRect rect_dlg;
		GetClientRect(&rect_dlg);
		TCHAR str_dsp[256];
		TCHAR str[256];


		memset(str, NULL, sizeof(TCHAR) / sizeof(char));

		// dc.SelectClipRgn ( &m_rgn, RGN_COPY );

		//サイズ調整
		//		m_pct.MoveWindow(10,10,rect_dlg.Width()-20,rect_dlg.Height()-5);
		m_pct.MoveWindow(X_PIC, Y_PIC, rect_dlg.Width() + XL_PIC, rect_dlg.Height() + YL_PIC);

		CBrush br_white(RGB(255, 255, 255));
		CBrush br_blue(RGB(128, 128, 255));
		CBrush br_red(RGB(255, 128, 128));

		CDC* pDC = m_pct.GetDC();
		CRect rect;
		m_pct.GetClientRect(&rect);

		//仮想デバイスコンテキスト作成
		CDC memDC;
		memDC.CreateCompatibleDC(m_pct.GetDC());

		//仮想デバイスコンテキスト用ビットマップ作成
		CBitmap memBmp;
		memBmp.CreateCompatibleBitmap(m_pct.GetDC(), rect.Width(), rect.Height());

		//仮想デバイスコンテキストにビットマップ設定
		CBitmap* memOldBmp = memDC.SelectObject(&memBmp);

		//元のブラシをoldbrに保持
		CBrush* oldbr = memDC.SelectObject(&br_white);
#if 1

		//背景を白にする
		memDC.Rectangle(&rect);
		//		memDC.SelectObject(&br_blue);
		memDC.SelectObject(&br_white);


		//-----------ここ-------------
		DrawCntrl(&memDC);  // 地図、衛星 メモリデバイスコンテクストに書き込む


							//memDC.TextOut(10,20,str);

		swprintf_s(str,256, _T("%10s"), sat_name_tbl[m_dsp_no]);

		memDC.TextOut(X_SEL_SAT_NM, Y_SEL_SAT, str); // 衛星名

#endif

													 //		 swprintf(str,_T("flg1 %d flg2 %d dist %d"), m_flg1[m_dsp_no],m_flg2[m_dsp_no],m_dist[m_dsp_no]);

		TCHAR sight[64];
		TCHAR light[64];
		int wrk = sizeof(sight);
		memset(sight, NULL, sizeof(sight));
		memset(light, NULL, sizeof(light));
		memset(str, NULL, sizeof(str));

		if (m_flg1[m_dsp_no] == 0)
		{
			_tcscpy_s(sight,64, _T("true"));
		}
		else {
			_tcscpy_s(sight,64, _T("false"));
		}
		if (m_flg2[m_dsp_no] == 0)
		{
			_tcscpy_s(light,64, _T("true"));
		}
		else {
			_tcscpy_s(light,64, _T("false"));
		}
		swprintf_s(str, 256,_T("SIGHT %s LIGHT %s DIST %d"), sight, light, m_dist[m_dsp_no]);
#if 1
		//  memDC.TextOut(30,270,str);
		memDC.TextOut(X_SEL_SAT_STS, Y_SEL_SAT_STS, str);

		swprintf_s(str,256, _T(" mode %d calc %d"), theApp.m_mode, theApp.m_calc_kind);
		//  memDC.TextOut(200,270,str);
		//  memDC.TextOut(200,30,str);
		memDC.TextOut(300, 30, str);
#endif


#if 1
		CString str_date;
		CString str_sdrm;
		if (m_mjd1>0)
		{
			m_shta = side_real(m_mjd1);
			m_shta *= 360;
			//-	str_sdrm.Format(_T(" θG　%f "),m_shta);

			int year, month, day;
			mjd_ymd(m_mjd1, &year, &month, &day);
			int h, m, s;
			frc_to_hms(m_mjd1, &h, &m, &s);
			if (m_jst_utc == 0)
			{
				h += 9;
				if (h>24)
				{
					h -= 24;
					mjd_ymd(m_mjd1 + 1, &year, &month, &day);

				}
			}
			if (m_jst_utc == 0)
			{
				str_date.Format(_T("%d 年 %d 月　%d 日　%d 時　%d　分 %d　秒　JST"), year, month, day, h, m, s);
			}
			else {
				str_date.Format(_T("%d 年 %d 月　%d 日　%d 時　%d　分 %d　秒　UTC"), year, month, day, h, m, s);

			}
		}

#if 1
		//		memDC.TextOut(10,300,str_date);
		memDC.TextOut(10, 5, str_date);
		memDC.TextOut(350, 30, str_sdrm);
#endif

#if 1
		//描画
		BitBlt(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), memDC.GetSafeHdc(), 0, 0, SRCCOPY);

		//元のブラシに戻す
		memDC.SelectObject(oldbr);

		//仮想デバイスコンテキストのビットマップを初期化
		memDC.SelectObject(memOldBmp);

		//仮想デバイスコンテキストのビットマップを廃棄
		memBmp.DeleteObject();
		CDialog::OnPaint();
#endif

#endif
	}
	//CDialogEx::OnPaint();
	
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CCalSatPosDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
///tttttttttttttttt
int FDlg(TCHAR *PathFileName)
{
	CString pathName = NULL;
	CString fileName = NULL;

//	TCHAR PathFileName[MAX_PATH];

	TCHAR head[130];
	TCHAR longstr1[130];
	TCHAR longstr2[130];
	TleData tle;
	char typerun = 'm';


	TCHAR szFilters[] = _T("Text Files (*.txt)|*.my|All Files (*.*)|*.*||");

	CFileDialog fileDlg(TRUE, _T("dat"), _T("*.dat"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if (fileDlg.DoModal() == IDOK)
	{
		pathName = fileDlg.GetPathName();

		fileName = fileDlg.GetFileName();

//		SetWindowText(fileName);

		_tcscpy_s(PathFileName, MAX_PATH, fileDlg.GetPathName());


		return 0;

	}
	else {
		return -1;

	}


}

//ttttttttttt


void CCalSatPosDlg::OnBnClickedButton1()
{
	CString pathName = NULL;
	CString fileName = NULL;

	TCHAR PathFileName[MAX_PATH];

	TCHAR head[130];
	TCHAR longstr1[130];
	TCHAR longstr2[130];
	TleData tle;
	char typerun = 'm';

	int ts, tf, delt;
	int mode = 1;
	int kind = 2;  /// 摂動モデル =1 sgp =2 sgp4 =3 sdp4
	double tsince = 0.0;
	double ro[3];
	double vo[3];





	TCHAR szFilters[] = _T("Text Files (*.txt)|*.my|All Files (*.*)|*.*||");

	CFileDialog fileDlg(TRUE, _T("dat"), _T("*.dat"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if (fileDlg.DoModal() == IDOK)
	{
		pathName = fileDlg.GetPathName();

		fileName = fileDlg.GetFileName();

		SetWindowText(fileName);

		_tcscpy_s(PathFileName, MAX_PATH, fileDlg.GetPathName());




	}
	else {
		return;

	}

	FILE *fp = NULL;
	errno_t err;


	// Open for read (will fail if file "crt_fopen_s.c" does not exist)  
	//err = fopen_s(&fp, (const TCHAR *)PathFileName, "r");
	err = _tfopen_s(&fp, PathFileName, _T("r"));
	if (err == 0)
	{
		printf("The file 'crt_fopen_s.c' was opened\n");
	}
	else
	{
		printf("The file 'crt_fopen_s.c' was not opened\n");
	}

	_fgetts(head, 129, fp);

	TCHAR mt[130];

	if (!_tccmp(head, _T("TESTDATA")))
	{
	_fgetts(mt, 129, fp);
	_stscanf_s(mt, _T("%d %d %d %d "), &kind, &ts, &tf, &delt);
	}

	_fgetts(longstr1, 129, fp);
	_fgetts(longstr2, 129, fp);


	memset(&tle, 0, sizeof(tle));

	int ret = ReadTle(longstr1, longstr2, typerun, &tle);




	memset(ro, 0.0, 3 * sizeof(double));
	memset(vo, 0.0, 3 * sizeof(double));

	tsince = ts;

	FILE *fp_o;
	err = _tfopen_s(&fp_o, _T("out.txt"), _T("a"));

	_ftprintf(fp_o, _T("  mode, kind, ts, tf, delt \n"));
	_ftprintf(fp_o, _T(" %d, %d,%d,%d,%d \n"), mode, kind, ts, tf, delt);

	int ret_cntrl = 0;

	while (tsince <= tf)
	{

		if (kind != -1)
		{
			ret_cntrl = sgp_main_cntrl(mode, &kind, &tle, tsince, ro, vo);

		}
		else {
			Cal2Body(&tle, tsince, ro, vo);
		}

		_ftprintf(fp_o, _T(" %f,%f,%f,%f,%f,%f,%f \n"), tsince, ro[0], ro[1], ro[2], vo[0], vo[1], vo[2]);


		tsince += delt;

	}
	_ftprintf(fp_o, _T("\n"));

	fclose(fp_o);

	fclose(fp);

	AfxMessageBox(_T("終了！！"));
	
}


void CCalSatPosDlg::OnBnClickedButton2()
{

	int mode=1;
		int calc_kind=1;
		int n=1;
		CString sat_name_tbl;
		int this_year=2018;
		double this_mjd1 = 58469.125000;
		double phi=deg_rad(37.91);
		double lambda=deg_rad(139.03);
		double high=0.0;
		double eps=deg_rad(23.4);
		TCHAR CoPath[256];
		TCHAR fileName[256];


		double lat_deg_sgp;
		double log_deg_sgp;
		double h_sgp;
		double decl_sgp;
		double ra_sgp;
		double azimth_sgp;
		double height_sgp;
		int dist;
		int flg1;
		int flg2;
		//CPosSatMng *sat_pos_mng;


		_tcscpy_s(CoPath, _T("D:\\Users\\CalSat\\CalSat2017New\\out\\"));


		TCHAR PathFileName[MAX_PATH];


		int ret = FDlg(PathFileName);

	CalSGP4(mode, &calc_kind,
		n, &sat_name_tbl,
		this_year, this_mjd1,
		phi, lambda, high,
		eps, CoPath, PathFileName,
		&lat_deg_sgp, &log_deg_sgp, &h_sgp,
		&decl_sgp, &ra_sgp,
		&azimth_sgp, &height_sgp, &dist,
		&flg1, &flg2,
		NULL);
}
// ファイルの先頭にヘッダー、衛星名一覧を出力
void CCalSatPosDlg::write_header(TCHAR *fname, TCHAR *ver)
{
	FILE *fp;

	int err = _tfopen_s(&fp, fname, _T("w"));
	//fp = _tfopen(fname, _T("w"));
	fwprintf(fp, _T("%s\n"), ver);
	fwprintf(fp, _T("%d\n"), m_n);
	for (int isat = 0; isat<m_n; isat++)
	{
		if (sat_name_tbl[isat].GetLength()>0)
		{
			fwprintf(fp, _T("%s\n"), sat_name_tbl[isat]);
		}
	}

	fclose(fp);
}

void CCalSatPosDlg::MENU_RFTLE()
{

	TCHAR fileName[256];
	TCHAR fileName_wk[256];
	// int year;
	TCHAR sat_name[N_SAT];
	int epoch_year;
	double epoch_val;
	double ave_nn_val;
	double period;
	double dn_val;
	double semi_val;
	double inc_val;
	double ecc_val;
	double ascd_val;
	double argperi_val;
	double mean_anomaly_val;
	double rev_val;
	int loc;
	int ret;
	TCHAR epoch_str[3000];
	TCHAR epoch_ymd[3000];


	CTime tm;
	double phi = deg_rad(m_phi);
	double lambda = deg_rad(m_lambda);
	INT_PTR ret1 = 0;

	if (theApp.m_prg_mode != 1)
	{

		CFileDialog dlg(TRUE);

		//_tcscpy(dlg.m_ofn.lpstrInitialDir,m_TLEPath);

		//dlg.m_ofn.lpstrInitialDir=m_TLEPath;

		dlg.GetOFN().lpstrInitialDir = m_TLEPath;

		ret1 = dlg.DoModal();

		if (ret1 != IDOK)
		{
			return;
		}

		_tcscpy_s(fileName_wk,256, dlg.GetPathName());
	}
	else {
		ret1 = IDOK;

		_tcscpy_s(fileName_wk,256, theApp.m_cmdl_fileName);
	}
	if (ret1 == IDOK)
	{
		_tcscpy_s(fileName,256, fileName_wk);

		_tcscpy_s(m_fileName,3000, fileName);



		FILE *fp;
		FILE *fp1;
		int err= _tfopen_s(&fp,fileName, _T("r"));
		if (err!=0)
		{
			TCHAR err_msg[512];
			_stprintf_s(err_msg, _T(" %s オープンエラー \n"), fileName);
			::AfxMessageBox(err_msg);

			return;
		}
		err = _tfopen_s(&fp1, _T("elm.txt"), _T("w"));
		if (err!=0)
		{
			TCHAR err_msg[512];
			_stprintf_s(err_msg, _T(" elm.txt オープンエラー \n"));
			::AfxMessageBox(err_msg);

			return;
		}

		fprintf(fp1, "sat_name_tbl,epoch_val_tbl,ave_nn_val_tbl,period_tbl,dn_val_tbl,semi_val_tbl,inc_val_tbl,ecc_val_tbl,ascd_val_tbl,argperi_val_tbl,mean_anomaly_val_tbl,rev_val_tbl \n");


		//	if(fp!=NULL)
		//	{


		tm = CTime::GetCurrentTime();

		m_year = tm.GetYear();
		m_month = tm.GetMonth();
		m_day = tm.GetDay();
		m_hour = tm.GetHour() - 9.0;
		m_min = tm.GetMinute();
		m_sec = tm.GetSecond();

		loc = 0;
		ret = 0;
		while (ret == 0)
		{
			//---sgp計算用は別に読み込んでいる
			ret = ReadSatElemA(fp, m_year, sat_name, &epoch_year, &epoch_val, &epoch_ymd[0],
				&ave_nn_val, &period, &dn_val, &semi_val, &inc_val, &ecc_val,
				&ascd_val, &argperi_val, &mean_anomaly_val, &rev_val);




			char ch[200];
			//---

			if (ret != 0) break;
			sat_name_tbl[loc] = sat_name;
			sat_name_tbl[loc].Trim();
			epoch_year_tbl[loc] = epoch_year;
			epoch_val_tbl[loc] = epoch_val;
			ave_nn_val_tbl[loc] = ave_nn_val;
			period_tbl[loc] = period;
			dn_val_tbl[loc] = dn_val;
			semi_val_tbl[loc] = semi_val;
			inc_val_tbl[loc] = inc_val;
			ecc_val_tbl[loc] = ecc_val;
			ascd_val_tbl[loc] = ascd_val;
			argperi_val_tbl[loc] = argperi_val;
			mean_anomaly_val_tbl[loc] = mean_anomaly_val;
			rev_val_tbl[loc] = rev_val;
			period_val_tbl[loc] = period;

			//fprintf(fp1, "%s,%f,%f,%f,%f,%f,%f,%f,%f,%f.%f,%f \n",
			//	sat_name_tbl[loc], epoch_val_tbl[loc], ave_nn_val_tbl[loc], period_tbl[loc], dn_val_tbl[loc],
			//	semi_val_tbl[loc], inc_val_tbl[loc], ecc_val_tbl[loc], ascd_val_tbl[loc], argperi_val_tbl[loc],
			//	mean_anomaly_val_tbl[loc], rev_val_tbl[loc]);
			//		fprintf(fp1,"%s \n",
			//			    sat_name_tbl[loc]);
			fprintf(fp1, "%s,%f,%f,%f, \n",
				sat_name_tbl[loc], epoch_val_tbl[loc], ave_nn_val_tbl[loc], period_tbl[loc]);
			//		fprintf(fp1, "%s \n",
			//			sat_name_tbl[loc]);
			loc++;


			//		fclose(fp);
			//fclose(fp1);



		}
		m_dsp_no = 0;

		fclose(fp1);

		// 衛星名の表示

		// 現在の表示をクリア
		while (m_list_ctrl1.GetItemCount())
		{
			m_list_ctrl1.DeleteItem(0);
		}

		m_n = loc;
		int ii = m_n - 1;
		while (ii >= 0)
		{
			m_list_ctrl1.InsertItem(0, sat_name_tbl[ii], NULL);
			ii--;
		}

		// ファイルをクリア
		TCHAR fname[MAX_PATH];



		make_fname(&m_CoPath[0], sat_name_tbl[0], _T(XYZ_TXT),
			fname);
		write_header(fname, _T(HEADER1));

		make_fname(&m_CoPath[0], sat_name_tbl[0], _T(SGP_TXT),
			fname);
		write_header(fname, _T(HEADER3));

		make_fname(&m_CoPath[0], sat_name_tbl[0], _T(G_TXT),
			fname);
		write_header(fname, _T(HEADER2));

		make_fname(&m_CoPath[0], sat_name_tbl[0], _T(GSGP_TXT),
			fname);
		write_header(fname, _T(HEADER4));
		//ttttt
		make_fname(&m_CoPath[0], sat_name_tbl[0], _T(GSGP_JP_TXT),
			fname);
		write_header(fname, _T(HEADER4));


	}

	//--------------

	double high = 0.0;
	double azimth;
	double height;
	double lat;
	double log;
	double h;
	double co[3];
	double co_sat[3];
	double co_rect[3];
	double co_obs[3];
	double co_obs1[3];
	//double m;
	//m_n=loc;
	loc = 0;

	TCHAR fname[MAX_PATH];
	_tcscpy_s(fname,260, &m_CoPath[0]);
	_tcscat_s(fname, 260,sat_name_tbl[0]);
	_tcscat_s(fname, 260, _T("out_xyz0.txt"));
	FILE *fp2;
	int err= _tfopen_s(&fp2, fname, _T("a"));
	if (err!=0)
	{
		TCHAR err_msg[512];
		_stprintf_s(err_msg, _T(" %s オープンエラー \n"), fname);
		::AfxMessageBox(err_msg);

		return;
	}

	fwprintf(fp2, _T("%d\n"), m_n);
	fwprintf(fp2, _T("%f\n"), m_mjd1);


	FILE *fp;

	//err = _tfopen_s(&fp, _T("out.txt"), _T("w"));
	//if (err!=0)
	//{
	//	TCHAR err_msg[512];
	//	_stprintf_s(err_msg, _T(" out.txt オープンエラー \n"));
	//	::AfxMessageBox(err_msg);

	//	return;
	//}

	m_mjd1 = mjd(m_year, m_month, m_day, m_hour, m_min, m_sec);

	CalSPos(m_n, phi, lambda);

	this->Invalidate();

}

void  CCalSatPosDlg::CalSPos(int n,
	double phi, double lamda)
{
	//CalSmallBodyPos(theApp.m_calc_kind, 
	//       m_n, &m_x[0], &m_y[0], &m_z[0],
	//m_year,
	//m_phi, m_lambda);
	double high = 0;
	CalSGP4(theApp.m_mode, &theApp.m_calc_kind,
		m_n, &sat_name_tbl[0],
		m_year, m_mjd1,
		deg_rad(m_phi), deg_rad(m_lambda), high,
		m_eps, m_CoPath, m_fileName,
		m_lat_deg_sgp, m_log_deg_sgp, m_h_sgp,
		m_decl_sgp, m_ra_sgp,
		m_azimth_sgp, m_height_sgp, m_dist,
		m_flg1, m_flg2,
		m_sat_pos_mng);

	CalPosSunMoon(m_eps, m_mjd1, m_phi, m_lambda,
		&m_decl_sun, &m_ra_sun, &m_azm_sun, &m_h_sun,
		&m_decl_moon, &m_ra_moon, &m_azm_moon, &m_h_moon);

}



// ファイル読み込み
void CCalSatPosDlg::OnBnClickedButton3()
{
	MENU_RFTLE();
}


void CCalSatPosDlg::Invalidate_reg()
{
	RECT rect = { 0, 0, 10, 10 }; //left, top, right, bottom
	int btn_id[] = { IDC_BTN_PRV,IDC_BUTTON3, IDC_LIST1, IDC_BTN_ANIMA };
	int nbtn = sizeof(btn_id) / sizeof(int);

//	ShowWindow(SW_HIDE);
	this->InvalidateRect(&rect, TRUE);
//	ShowWindow(SW_SHOW);

	for (int ii = 0; ii < nbtn; ii++ )
	{
		peek_msg();
		CWnd*    btn = (CWnd*)GetDlgItem(btn_id[ii]);
		btn->Invalidate();
	//	btn->ShowWindow(SW_SHOW);
	//	btn->EnableWindow(TRUE);
	}

}

// 1ステップ進める
void CCalSatPosDlg::OnBnClickedBtnNxt()
{
	double phi = deg_rad(m_phi);
	double lamda = deg_rad(m_lambda);
	double mjd0 = mjd(m_year, 1, 0, 0, 0, 0);

	if (m_del_min <= 0)
	{
		m_del_min = 1.0;
	}

	m_mjd1 += m_del_min / (60 * 24);

	CalSPos(m_n, phi, lamda);



	//this->ShowWindow(SW_HIDE);
	//this->Invalidate();

	Invalidate_reg();

	//this->InvalidateRect(NULL, TRUE);
	//this->ShowWindow(SW_SHOW);
}

// 1ステップ戻す
void CCalSatPosDlg::OnBnClickedBtnPrv()
{
	double phi = deg_rad(m_phi);
	double lambda = deg_rad(m_lambda);
	//double mjd0=mjd(m_year,1,0,0,0,0);

	if (m_del_min <= 0)
	{
		m_del_min = 1.0;
	}

	m_mjd1 -= m_del_min / (60 * 24);

	CalSPos(m_n, phi, lambda);
	//this->Invalidate();

	Invalidate_reg();

}


void CCalSatPosDlg::OnBnClickedBtnAnima()
{
	//CAnimPara dlg;
	double dmin;
	int nc;
	CString str;


	str = _T("1");

	str.Format(_T("%g"), m_del_min);
	//dlg.m_del_min = m_del_min;
    //	dlg.m_nc = 1;

	// INT_PTR ret = dlg.DoModal();

	//if (ret == IDOK)
	//{
	//	dmin = dlg.m_del_min;
	//	nc = dlg.m_nc;
	dmin = 1.0;
	nc = 1;

		CClientDC dc(this);

		int nn = this->period_tbl[0] / dmin;

		for (int ii = 0; ii<(nn + 1)*nc; ii++)
		{
			peek_msg();
			double phi = deg_rad(m_phi);
			double lambda = deg_rad(m_lambda);
			//		double mjd0=mjd(m_year,1,0,0,0,0);
			m_mjd1 += dmin / (60 * 24);

			CalSPos(m_n, phi, lambda);
			Invalidate(FALSE);

		}
	//}

}

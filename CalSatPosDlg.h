
// CalSatPosDlg.h : ヘッダー ファイル
//

#pragma once
#include "wshore.h"
#include "PosSatMng.h"

#define N_SAT	3000


#define RAD 2
// ピクチャ領域左上座標
#define X_PIC	10
#define Y_PIC	10
#define XL_PIC -20
#define YL_PIC -5
// 世界地図左上座標
#define X_O	250
#define Y_O 150
// 現在選択している衛星の位置情報
#define X_SEL_SAT 150
#define Y_SEL_SAT 300

#define X_SEL_SAT_NM 10

// 現在選択している衛星の状態
#define X_SEL_SAT_STS 30
#define Y_SEL_SAT_STS 30
// 衛星情報一覧表の位置
#define X_TBL_SAT 15
#define Y_TBL_SAT 330


// CCalSatPosDlg ダイアログ
class CCalSatPosDlg : public CDialogEx
{
// コンストラクション
public:
	CCalSatPosDlg(CWnd* pParent = nullptr);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CALSATPOS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


private:
	int m_year;		///< 年
	int m_month;	///< 月
	int m_day;	///< 日
	int m_hour;	///< 時
	int m_min;	///< 分
	int m_sec;	///< 秒
	double m_lambda; ///< 観測値経度
	double m_phi;	 ///< 観測値緯度
	double m_del_min; ///< 計算間隔(sec)

	int m_dsp_no; ///< 衛星情報表示インデックス
	int m_n;  ///<
	double m_longt[N_SAT]; ///< 衛星経度
	double m_lat[N_SAT]; ///<　衛星緯度
	double m_h[N_SAT]; ///<　衛星高度
	double m_azm[N_SAT]; ///<　方位
	double m_height[N_SAT]; ///<　高度



	double m_lat_peri[N_SAT]; ///<　近地点緯度
	double m_log_peri[N_SAT]; ///<　近地点経度


	double m_x[N_SAT]; ///<
	double m_y[N_SAT]; ///<
	double m_z[N_SAT]; ///<



	CString sat_name_tbl[N_SAT]; ///<衛星名
	int epoch_year_tbl[N_SAT]; ///<元期年
	double epoch_val_tbl[N_SAT]; ///<元期
	double ave_nn_val_tbl[N_SAT]; ///<平均運動 (Mean Motion) (回転/日)
	double period_tbl[N_SAT]; ///< 周期（分）
	double dn_val_tbl[N_SAT]; ///<平均運動の1次微分値を2で割った値。単位は 回転/day^2 
	double semi_val_tbl[N_SAT]; ///<軌道半長径
	double inc_val_tbl[N_SAT]; ///<軌道傾斜角
	double ecc_val_tbl[N_SAT]; ///<離心率
	double ascd_val_tbl[N_SAT]; ///<昇交点赤経
	double argperi_val_tbl[N_SAT]; ///<近日点引数
	double mean_anomaly_val_tbl[N_SAT]; ///< 平均近点角
	double rev_val_tbl[N_SAT]; ///< 通算軌道周回数
	double period_val_tbl[N_SAT];
	double m_mjd1; ///<
	double m_shta; // 使っていない？

	int m_jst_utc;  ///< =0 utc =1 jst				//int m_yy; ///<
					//int m_mm; ///<
					//int m_dd; ///<
					//int m_hr; ///<
					//int m_mn; ///<
					//int m_ss; ///<
	bool m_dsp_sname; ///< true 

	bool m_line_orb;  ///< 軌道の線を描く

	int m_flg1[N_SAT]; ///< 観測点から衛星が見えている =1
	int m_flg2[N_SAT]; ///< 衛星に太陽光があたっている =1
	int m_dist[N_SAT]; ///<

					   // SGP4 モデルで計算した結果
	double m_lat_deg_sgp[N_SAT]; ///< 衛星緯度
	double m_log_deg_sgp[N_SAT]; ///< 衛星経度
	double m_h_sgp[N_SAT]; ///< 衛星高度
	double m_azimth_sgp[N_SAT]; ///< 方位
	double m_height_sgp[N_SAT]; ///<　高度
	double m_vl_sat_sgp[N_SAT][3];

	double m_decl_sgp[N_SAT];  ///< 赤緯
	double m_ra_sgp[N_SAT];    ///< 赤径

	double m_eps;

	double m_decl_moon;  ///< 月の赤緯
	double m_ra_moon;  ///< 月の赤経
	double m_azm_moon;  ///< 月の方位
	double m_h_moon;  ///< 月の高さ

	double m_decl_sun;  ///< 太陽の赤緯
	double m_ra_sun;  ///< 太陽の赤経
	double m_azm_sun;  ///< 太陽の方位
	double m_h_sun;  ///< 太陽の高さ

	TCHAR m_TLEPath[MAX_PATH];///	TLEファイル読み込みディレクトリ
	TCHAR m_CoPath[MAX_PATH];///	衛星3次元座標ファイル読み込みディレクトリ
	TCHAR m_fileName[N_SAT]; ///<　two line element ファイル名（フルパス）

	bool m_ang_unit; ///< true 度分秒 false 度

	//CStatic m_pct; ///<
	CRgn m_rgn; ///<

	int m_cal_method; ///<

	CPosSatMng m_sat_pos_mng[N_SAT];

	int DrawCntrl(CDC *memDC);

	void MENU_RFTLE();

	void Invalidate_reg();

public:
	int m_nsh;
	SHR_CO *m_shore;
	void write_header(TCHAR *fname, TCHAR *ver);
	void  CalSPos(int n, double phi, double lamda);



public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

	CListCtrl m_list_ctrl1;
	CStatic m_pct;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedBtnNxt();
	afx_msg void OnBnClickedBtnPrv();
	afx_msg void OnBnClickedBtnAnima();
};

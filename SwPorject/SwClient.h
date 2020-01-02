#pragma once
#include "SwComm.h"
#include <map>
#include <Windows.h>


#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\imgproc.hpp>
#include <thread>
#include <mutex>

#include "MyUsbMgr.h"

#define ONLYONECLIENT			0xff
//#define MULTICLIENT				0xff

typedef std::map<int, cv::Mat> ImgMatMap;
typedef std::map<std::string, ImgMatMap> ImgStrMatMMaP;

class SwClient
{
private:
	BITMAPINFOHEADER m_BitMapInfoHeader;
	cv::Mat m_ShotMat;
	cv::Point m_cashStorePoint;
	LPWSTR m_AppPath;
	static DWORD m_LastKeyEventTime;
	cv::Point m_OriginPoint;
	int m_CashGoodsPricesMax;
	static ImgStrMatMMaP m_MatMMap;
	cv::Mat& AddMaskForGoods(cv::Mat& src);
	static std::mutex usbMutex;
	static std::mutex ResourceInitMutex;
	bool pause;
private:
	cv::Size GetSize(BITMAPINFOHEADER &bitMapInfoHeader);
	LRESULT MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
public:
	cv::Point Test_ChoiceOption();
	void Test_Opencv();
public:
	SwClient();
	void ReadResourceFiles();
	int Init();
	void GetLeftTopBmp();
	int Start(int x = 0, int y = 0, int width = 1920, int height = 1080);
	void Test();
	void Pause();
	void QiangMie();
	void SetOriginPoint(int left, int top);
	void SetOriginPoint(cv::Point p);

	BOOL ShotDeskTop(cv::Rect _r);
	BOOL ShotSwClient(cv::Rect _r = cv::Rect(0, 0, CLIENTWIDTH, CLIENTHEIGHT));

	BOOL MatchGoods(cv::Mat& src, cv::Mat mch);
	void GoodsPrePage();
	void GoodsNextPage();

	int OpenUi(EUiMark um, cv::Point& markPoint);
	int CloseUi(EUiMark um, bool useKey = false);

	cv::Point OpenRiChang();
	cv::Point OpenRenWu();
	cv::Point OpenWuPin();
	cv::Point OpenJiShouZhongXin();
	cv::Point OpenJiaoYiZhongXin();
	cv::Point OpenDiTu();
	cv::Point GetFuJinNPCMat(cv::Mat& retMat);

	BOOL IsRenWuOpen();

	void CloseRiChang();
	void CloseRenWu();
	void CloseWuPin();
	void CloseJiShouZhongXin();
	void CloseJiaoYiZhongXin();
	void CloseDiTu();

	cv::Point ShotRenWuWithOpenUi(cv::Mat& taskMat, ERiChang erc);
	cv::Point GetRenWuTargetMat(cv::Mat& taskMat, cv::Point taskPoint, cv::Mat& targetMat);
	cv::Point GetRenWuListMat(cv::Mat& taskMat, cv::Point taskPoint, cv::Mat& listMat);
	cv::Point GetRenWuHuiFuMat(cv::Mat& taskMat, cv::Point taskPoint, cv::Mat& huifuMat);
	DiTuLoc GetMatDiTuZuoBiao(cv::Mat &numBinMat);
	DiTuLoc FindGuaiWuTargetInRenWu(cv::Mat& taskMat, cv::Point taskPoint);

	int OpenCashStore(ECashStoreGoodsType type);
	int GetGoodsPrice(cv::Mat& goodsPageMat, int row, int col);
	cv::Mat GetGoodsIcon(cv::Mat& goodsPageMat, int row, int col);
	int GetPageGoodsInfo(PCashStoreGoods goods, ECashStoreGoodsType type);
	int GetPageGoodsInfo(cv::Mat& goodsPageMat, PCashStoreGoods goods, cv::Mat& srcMat);
	int GetMinPriceGood(cv::Point jszxPoint, cv::Mat& src, int scanfPageSize, int& price, int& page, int& row, int& col);
	int GetMinPriceGood(ECashStoreGoodsType type, int subType, int scanfPageSize, int& page, int& row, int& col);
	void BuyCashStoreGoods(int row, int col);
	int BuyDianPuGoodsAndColseUi();
	int SaveGoodsIconsToBmp(std::string dirPath);
	void YuanGuYaoGuai();
	EMap ShiMenMatchMenPaiMap(cv::Mat& targetMat);
	int GotoExceptCityApart(EMap location);

	int HandleMenPaiXunLuo(cv::Mat& tempMat, cv::Point& taskTargetP);
	int HandleBuZhuoChongWu();
	int ShiMenHandleShouJiWuZi(cv::Mat& taskMat, cv::Point taskPoint);
	int ShiMenHandleTiaoZhan(cv::Mat& targetMat, cv::Point taskTargetP);
	int GetZhanChangMastMonster(EMonster MonsterName, std::vector<cv::Point>& pointVec);
	int BuZhuoHuXiaoMeng();

	void CustomAction();
	void FuBen();
	int GetNormalZhuangTaiMat(cv::Mat& ztMat);
	void ZhuangTaiHuiFu();

	BOOL MatchDiTu(EMap em);
	EMenPai MatchMenPaiDiTu();
	int AutoMeetMonster();
	BOOL PingBiRenWu();

	ECashStoreGoodsType FindNeedGoodsType(cv::Point jszxPoint);
	//日常任务
	int DoRiChangMultiClient(bool needShutDown = false);
	int RiChangLingHu();
	int RiChangShiMen();
	int RiChangXiulian();
	int RiChangZhuoGui();
	int RiChangXuanWu();
	int RiChangQingLong();
	int RiChangBaoTu();

	int HuanJing();
	int RiChangLingHuMultiClient();

	cv::Point FindGuaiWuTargetInScene();
	EMap RenWuGetTargetPosition(cv::Mat targetMat);
	ETakType XiulianDoRenWuPre(cv::Mat taskMat, cv::Point taskPoint);
	int XiulianTiJiaoRenWu(ETakType ett, cv::Mat& targetMat, cv::Point huifuPoint);

	int HandleRenWuBuyGoodsInSWB();
	int HandleRenWuBuyGoodsInCashStore();
	int HandleRenWuBuyGoodsInDianPu(cv::Mat& targetMat, cv::Point& taskTargetP);

	bool IsRenWuComplete();

	int HandleBaoTuStep1(cv::Mat taskMat, cv::Point taskPoint);
	int HandleBaoTuBaoZangLieRen(cv::Mat taskMat, cv::Point taskPoint);
	int HandleBaoTuBaoDaDangJia(cv::Mat taskMat, cv::Point taskPoint);


	BOOL IsRiChangFinish(ERiChang rc);
	int GotoLingHuLeYuan();
	BOOL IsFighting(int waitMs = 0);
	BOOL isWalking(int checkTime = 500);
	BOOL IsZiDongXunLuNow();
	BOOL WaitForZiDongXuLu(ERiChang rc);
	BOOL TurnOnGuaiWuTuBiao();
	int WaitForNextHuiHe(int timeOut = 180000);
	int RiChangShiMenSeeMenPaiMaster();
	void CloseDuiHuaKuang(int waitMs = 3000, BOOL Multi = false);
	BOOL IsDuiHuaKuangExists(int waitMs = 1000);

	int ChoiceDuiHuaKuangFirstOption();
	int ChoiceRenWuInReWuLan(ERiChang rc);

	int WalkWithOpenDiTu(EMap ditu, DiTuLoc targetPoint);
	int GotoExceptCity(EMap position, bool canFly = true);
	int FlyWithFeiXingQi(EMap city, bool isUiOpened = false);
	int FlyWithFeiXingQi(EMap city, cv::Point centerPoint, bool isUiOpened = false);
	cv::Point TranformDiTuLocToPoint(EMap city, DiTuLoc zuobiao, bool isUiOpened = false);
	cv::Point FindFeiXingQiFlagNearPoint(cv::Point _r);
	cv::Point FindFeiXingQiFlagInAreaRect(cv::Rect _r);
	cv::Point FindHuiFuTargetInDiTu(EMap city);


	void ClientGetFous();
	void GetFocusDelayLess();
	void CustomEvent1();

	//再次套一层调用，方便pause
	int GetShotArea(cv::Rect rect, cv::Mat& mat_dest);
	int SendMouseAction(MouseOperator opt);
	int SendKeyBoardAction(EKeyBoardEventOperator eop, int code);


	void MouseMoveClickLeftTop();
	int DefaultFight(bool zidonghuifu = true);
	void SetCommandAutoFight();
	int CustomFightCommand(int shortcutCode, cv::Point point);
	int KeyBtnClickWithAlt(int code);
	int KeyBtnClickWithCtrl(int code);

	
	int KeyBtnDown(int code);
	int KeyBtnUp(int code);
	int KeyBtnClick(int code);

	int MouseLeftBtnDown();
	int MouseLeftBtnUp();
	int MouseLeftBtnClick();
	int MouseRightBtnDown();
	int MouseRightBtnUp();
	int MouseRightBtnClick();

	int MouseMoveAbsLBClick(int x, int y);

	int MouseMoveLBClick(int x, int y);
	int MouseMoveLBClick(cv::Point point);
	int MouseMoveRBClick(int x, int y);
	int MouseMoveAbsRBClick(int x, int y);
	int MouseMoveRBClick(cv::Point point);

	int MouseMoveAbsLBClickQuickly(int x, int y);


	void UsbCommDuration(int delayMs);
};


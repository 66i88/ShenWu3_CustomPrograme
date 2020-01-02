#include "stdafx.h"
#include "SwClient.h"
#include "SwComm.h"
#include "unitl.h"
#include <iostream>
#include <map>
#include <ctime>
#include <vector>


#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\imgproc.hpp>
#include <mutex>

#include "MyUsbMgr.h"

std::mutex SwClient::usbMutex;
std::mutex SwClient::ResourceInitMutex;
ImgStrMatMMaP SwClient::m_MatMMap;
DWORD SwClient::m_LastKeyEventTime = 0;

SwClient::SwClient()
{
	m_LastKeyEventTime = 0;
}

void SwClient::ReadResourceFiles()
{
	std::unique_lock<std::mutex> uqLock(ResourceInitMutex);
	if (m_MatMMap.size() > 0)
	{
		return;
	}
	LOG_MSG(DINFO, "SwClient ReadResourceFiles");
	std::vector<std::string> fileaNameVec;
	std::string rootPath = Unitl::GetAppPathS();
	std::string dirPath;
	dirPath = rootPath;
	cv::Mat rFileMat;
	dirPath = dirPath + SWPICTUREDIR + "\\uiMark\\";
	for (int i = 0; i < SwComm::UiMarkNameM.size(); i++)
	{
		std::string filePath = dirPath;
		filePath.append(SwComm::UiMarkNameM[i]);
		rFileMat = cv::imread(filePath);
		m_MatMMap["UiMark"].insert(std::make_pair(i, rFileMat));

	}
	dirPath = rootPath;
	dirPath = dirPath + SWPICTUREDIR + "\\tasknumber\\";
	for (int i = 0; i < SwComm::TaskNumberM.size(); i++)
	{
		std::string filePath = dirPath;
		filePath.append(SwComm::TaskNumberM[i]);
		//黑白，单位深
		rFileMat = cv::imread(filePath, cv::IMREAD_UNCHANGED);
		m_MatMMap["TaskNumber"].insert(std::make_pair(i, rFileMat));
	}
	dirPath = rootPath;
	dirPath = dirPath + SWPICTUREDIR + "\\map\\";
	for (int i = 0; i < SwComm::DiTuNameM.size(); i++)
	{
		std::string filePath = dirPath;
		filePath.append(SwComm::DiTuNameM[i]);
		rFileMat = cv::imread(filePath);
		m_MatMMap["Map"].insert(std::make_pair(i, rFileMat));
	}
	//寄售中心的商品需要增加遮蔽 AddMaskForGoods
	dirPath = rootPath;
	dirPath = dirPath + SWPICTUREDIR + "\\goods\\cooking\\";
	for (int i = 0; i < SwComm::CookingIconNameM.size(); i++)
	{
		std::string filePath = dirPath;
		filePath.append(SwComm::CookingIconNameM[i]);
		rFileMat = cv::imread(filePath);
		m_MatMMap["Cooking"].insert(std::make_pair(i, AddMaskForGoods(rFileMat)));

	}
	dirPath = rootPath;
	dirPath = dirPath + SWPICTUREDIR + "\\goods\\thirdDrugs\\";
	for (int i = 0; i < SwComm::ThirdDrugsIconNameM.size(); i++)
	{
		std::string filePath = dirPath;
		filePath.append(SwComm::ThirdDrugsIconNameM[i]);
		rFileMat = cv::imread(filePath);
		m_MatMMap["ThirdDrugs"].insert(std::make_pair(i, AddMaskForGoods(rFileMat)));
	}
	dirPath = rootPath;
	dirPath = dirPath + SWPICTUREDIR + "\\goods\\secondDrugs\\";
	for (int i = 0; i < SwComm::SecondDrugsIconNameM.size(); i++)
	{
		std::string filePath = dirPath;
		filePath.append(SwComm::SecondDrugsIconNameM[i]);
		rFileMat = cv::imread(filePath);
		m_MatMMap["SecondDrugs"].insert(std::make_pair(i, AddMaskForGoods(rFileMat)));
	}
}

int SwClient::Init()
{
	pause = false;
	m_CashGoodsPricesMax = 80000;
	ReadResourceFiles();
	return OPERATOR_SUCCESS;
}

void SwClient::GetLeftTopBmp()
{
	cv::Mat shotMat;
	cv::Rect shotRect(78, 454, 25, 20);
	if (GetShotArea(shotRect, shotMat))
	{
		cv::imwrite((Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\taiyang.bmp").c_str(), shotMat);
	}
}

//在桌面(x,y,width,height)查找客户端
int SwClient::Start(int x, int y, int width, int height)
{
	//GetLeftTopBmp();
	int tryMax = 5;//检测 查找客户端左上角时辰图片
	int i = 0;
	cv::Mat shotMat;
	cv::Rect shotRect(x, y, width, height);
	for (i = 0; i < tryMax; i++)
	{
		if (!GetShotArea(shotRect, shotMat))
		{
			return SHOT_ERROR;
		}
		cv::Point mchPoint = Unitl::MatchMat(shotMat, m_MatMMap["UiMark"][EUMTaiYang], 0.98);
		if (mchPoint.x != -1)
		{
			mchPoint += shotRect.tl() + cv::Point(-18, -5);
			SetOriginPoint(mchPoint.x, mchPoint.y);
			ClientGetFous();
			return OPERATOR_SUCCESS;
		}
		else {
			mchPoint = Unitl::MatchMat(shotMat, m_MatMMap["UiMark"][EUMYueLiang], 0.98);
			if (mchPoint.x != -1)
			{
				mchPoint += shotRect.tl() + cv::Point(-19, -4);
				SetOriginPoint(mchPoint.x, mchPoint.y);
				ClientGetFous();
				return OPERATOR_SUCCESS;
			}
		}
		Sleep(500);
	}
	if (i == tryMax)
	{
		return NOT_MATCH;
	}
	return OPERATOR_FAILED;
}


void SwClient::Pause()
{
	pause = true;
}

void SwClient::ClientGetFous()
{
	MouseMoveLBClick(2, 38);
}

void SwClient::MouseMoveClickLeftTop()
{
	MouseMoveLBClick(2, 38);
}

void SwClient::SetOriginPoint(int left, int top) {
	m_OriginPoint.x = left;
	m_OriginPoint.y = top;
}

void SwClient::SetOriginPoint(cv::Point p)
{
	m_OriginPoint = p;
}

/*
*	桌面(0,0)为原点，偏移截屏
*/
BOOL SwClient::ShotDeskTop(cv::Rect shotRect)
{
	if (GetShotArea(shotRect, m_ShotMat)) {
		return true;
	}
	return false;
}


/*
*	m_OriginPoint为原点，偏移截屏
*/
BOOL SwClient::ShotSwClient(cv::Rect shotRect)
{
	shotRect.x += m_OriginPoint.x;
	shotRect.y += m_OriginPoint.y;
	//客户端左上角的点，以及客户端大小
	if (GetShotArea(shotRect, m_ShotMat)) {
		return true;
	}
	return false;
}


cv::Size SwClient::GetSize(BITMAPINFOHEADER &bitMapInfoHeader) {
	cv::Size _s;
	_s.height = bitMapInfoHeader.biHeight;
	_s.width = bitMapInfoHeader.biWidth;
	return _s;
}

/*
*	匹配srcMat，一致项goods.GoodsEnum = ECSGTDefault
*/
int SwClient::GetPageGoodsInfo(cv::Mat& goodsPageMat, PCashStoreGoods goods, cv::Mat& srcMat)
{
	int s4_ret(OPERATOR_SUCCESS);
	cv::Size iSize;
	cv::Rect mRect;
	for (int r = 0; r < 5; r++)
	{
		for (int c = 0; c < 5; c++)
		{
			goods[r * 5 + c].GoodsEnum = 0;
			int prices = GetGoodsPrice(goodsPageMat, r, c);
			goods[r * 5 + c].prices = prices;
			if (prices == -1)
			{
				s4_ret = PARSE_PIXELBYTE_ERROR;
				goto DONE;
			}
			cv::Mat goodsIconMat = GetGoodsIcon(goodsPageMat, r, c);
			if (MatchGoods(srcMat, goodsIconMat))
			{
				//匹配成功
				goods[r * 5 + c].GoodsEnum = ECSGTDefault;
			}
			else {
				goods[r * 5 + c].GoodsEnum = -1; //匹配失败 GoodsEnum置-1
			}
		}
	}
DONE:
	return s4_ret;
}

int SwClient::GetPageGoodsInfo(PCashStoreGoods goods, ECashStoreGoodsType type)
{
	int s4_ret(OPERATOR_SUCCESS);
	//cv::Size iSize;
	//cv::Rect mRect;
	//if (!ShotSwClient())
	//{
	//	s4_ret = SHOT_ERROR;
	//}
	//for (int r = 0; r < 5; r++)
	//{
	//	for (int c = 0; c < 5; c++)
	//	{
	//		goods[r * 5 + c].GoodsEnum = 0;
	//		int prices = GetGoodsPrice(r, c);
	//		goods[r * 5 + c].prices = prices;
	//		if (prices == -1)
	//		{
	//			s4_ret = PARSE_PIXELBYTE_ERROR;
	//			goto DONE;
	//		}
	//		cv::Size _s(SwComm::GoodsGridWidth, SwComm::GoodsGridHeight);

	//		cv::Mat goodsIconMat = GetGoodsIcon(r, c);
	//		std::string tyepName = SwComm::GetECSGT_SName(type);
	//		for (int k = 0; k < m_MatMMap[tyepName].size(); k++)
	//		{
	//			if (m_MatMMap[tyepName][k].empty())continue;
	//			if (MatchGoods(m_MatMMap[tyepName][k], goodsIconMat))
	//			{
	//				//匹配成功
	//				goods[r * 5 + c].GoodsEnum = k;
	//				break;
	//			}
	//		}
	//	}
	//}
DONE:
	return s4_ret;
}

/*
*	此处比较会为 src 添加Mask
*/
BOOL SwClient::MatchGoods(cv::Mat& src, cv::Mat mch)
{
	if (src.data[0] != 0)
	{
		AddMaskForGoods(src);
	}
	if (Unitl::MatchMat(src, AddMaskForGoods(mch), 0.98).x != -1)
	{
		return true;
	}
	return false;
}



/*
*	tab打开地图比较
*/
EMenPai SwClient::MatchMenPaiDiTu()
{
	BOOL b_ret(false);
	cv::Mat mapMat;
	cv::Point mchPoint;
	EMenPai mp = EMenPai::EMPDefaule;
	std::map<EMenPai, int> menPaiDiTuMatIndex;
	cv::Rect rect;
	mchPoint = OpenDiTu();//打开了地图
	if (mchPoint.x == -1)
	{
		goto DONE;
	}
	rect += m_OriginPoint + mchPoint;
	rect += cv::Size(40, 40);
	if (!GetShotArea(rect, mapMat)) {
		goto DONE;
	}
	/*Unitl::SaveMatAreaToPictureDir(mapMat, cv::Rect(0, 0, 20, 20),
		"\\map\\","mowangshan.bmp");*/
	menPaiDiTuMatIndex.insert(std::make_pair<EMenPai, int>(EMPTG, (int)EMLingXiaoTianGong));
	menPaiDiTuMatIndex.insert(std::make_pair<EMenPai, int>(EMPMW, (int)EMLMoWangShan));
	menPaiDiTuMatIndex.insert(std::make_pair<EMenPai, int>(EMPFM, (int)EMFoMen));
	for (auto it : menPaiDiTuMatIndex)
	{
		if (Unitl::MatchMat(mapMat,
			m_MatMMap["Map"][it.second], 0.98).x != -1)
		{
			mp = it.first;
			goto DONE;
		}
	}

DONE:
	if (mchPoint.x != -1)
	{
		KeyBtnClick(0x09);
	}
	return mp;
}

int SwClient::RiChangShiMenSeeMenPaiMaster()
{
	cv::Mat tempMat;
	int tryTime = 0;
	cv::Point riChangPoint(-1, -1);
	int s4_ret(OPERATOR_SUCCESS);
	for (tryTime = 3; tryTime > 0; tryTime--)
	{
		if (tryTime == 2)
		{
			ZhuangTaiHuiFu();
		}
		KeyBtnClick(KEYBTN_F1);
		Sleep(DELAYMORETIMR + 500);//等待ui过渡
		/*EMenPai mp = MatchMenPaiDiTu();
		if (mp == EMPDefaule)
		{
			continue;
		}*/
		//这里打开了日程界面
		riChangPoint = OpenRiChang();
		if (riChangPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		cv::Rect shotRect(m_OriginPoint.x + riChangPoint.x + 30,
			m_OriginPoint.y + riChangPoint.y + 52, 680, 132);
		cv::Mat shotMat;
		cv::Point matchPoint;
		BOOL isFinish = false;
		if (GetShotArea(shotRect, shotMat))
		{
			matchPoint = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][EUMShiMenRenWu_WeiWanCheng1], 0.98);
			if (matchPoint.x == -1)

			{
				matchPoint = Unitl::MatchMat(shotMat,
					m_MatMMap["UiMark"][EUMShiMenRenWu_WeiWanCheng2], 0.98);
				if (matchPoint.x == -1)
				{
					s4_ret = OPERATOR_FAILED;
					goto DONE;
				}
			}
			matchPoint.x = riChangPoint.x + 30 + matchPoint.x + 30;
			matchPoint.y = riChangPoint.y + 52 + matchPoint.y + 10;
			//点击日常的图标，直达门派师傅的脚跟
			MouseMoveLBClick(matchPoint);
		}
		CloseRiChang();
		Sleep(100);
		if (!isWalking())
		{
			continue;
		}
		while (isWalking());
		if (IsDuiHuaKuangExists(3000))
		{
			break;
		}
		else {
			continue;
		}
	}
	if (tryTime == 0)
	{
		OpenRiChang();
		CloseRiChang();
		s4_ret = OPERATOR_FAILED;
	}
DONE:
	return s4_ret;
}

BOOL SwClient::IsFighting(int waitMs)
{
	cv::Mat tempMat;
	cv::Rect shotRect(m_OriginPoint.x + 680, m_OriginPoint.y + 400, 118, 200);
	int useTime = 0;
	for (; useTime <= waitMs; useTime += 80)
	{
		if (GetShotArea(shotRect, tempMat)) {
			if (Unitl::MatchMat(tempMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\fight\\zhankuang.bmp", 0.98).x != -1)
			{
				return true;
			}
		}
		Sleep(50);
	}
	return false;
}

int SwClient::DefaultFight(bool zidonghuifu)
{
	int sTime = GetTickCount();
	int uTime = 0, maxTime = 60000;
	for (; uTime < maxTime; uTime = GetTickCount() - sTime)
	{
		int fightRet = WaitForNextHuiHe();
		if (fightRet == FIGHTOVER)
		{
			Sleep(500);
			break;
		}
		else if (fightRet == WAITFIGHTCOMMAND)
		{
			KeyBtnClickWithAlt('Q');
			KeyBtnClickWithAlt('Q');
			KeyBtnClickWithAlt('Q');
			if (zidonghuifu)
			{
				zidonghuifu = false;
				ZhuangTaiHuiFu();
			}
		}
		else {
			return OPERATOR_FAILED;
		}
		Sleep(DELAYMORETIMR + 100);
	}
	if (uTime >= maxTime)
	{
		return OPERATOR_FAILED;
	}
	return OPERATOR_SUCCESS;
}

/*
*	开始前任务处于打开状态
*/
int SwClient::HandleMenPaiXunLuo(cv::Mat& tempMat, cv::Point& taskTargetP)
{
	cv::Point maskClickP = Unitl::FindPixelOnTail(tempMat, 0, 253, 253, 2);
	maskClickP += taskTargetP;
	maskClickP -= cv::Point(3, 3);
	int sTime = GetTickCount();
	int uTime = 0, maxTime = 1000 * 300;
	cv::Mat checkMat;
	for (; uTime < maxTime; uTime = GetTickCount() - sTime)
	{
		OpenRenWu();
		MouseMoveLBClick(maskClickP);
		CloseRenWu();
		Sleep(3000);
		if (isWalking())
		{
			while (isWalking());
		}
		if (IsFighting(2000))
		{
			Sleep(500);
			break;
		}
	}
	if (uTime >= maxTime)
	{
		return OPERATOR_FAILED;
	}
	return DefaultFight();
}


int SwClient::HandleBuZhuoChongWu()
{
	int tryTime = 0, subTryTime = 0;
	int walkTime = 1000;
	cv::Point flyP;
	for (tryTime = 0; tryTime < 3; tryTime++)
	{
		if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(421, 451)) != OPERATOR_SUCCESS)
		{
			return OPERATOR_FAILED;
		}
		WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(295, 34));
		if (!IsDuiHuaKuangExists(3000))
		{
			WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(295, 34));
			if (!IsDuiHuaKuangExists(3000))
			{
				continue;
			}
		}
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
		{
			continue;
		};
		int subTryTime = 5;
		cv::Mat tempMat;
		cv::Point mchPoint;
		//左侧，相对于屏幕
		cv::Rect shotRect(0, 0, 490, 200);
		shotRect += cv::Point(300, 382);
		for (; subTryTime > 0; subTryTime--)
		{
			if (!GetShotArea(shotRect + m_OriginPoint, tempMat))
			{
				return SHOT_ERROR;
			}
			mchPoint = Unitl::MatchMat(tempMat,
				m_MatMMap["UiMark"][EUiMark::EUMXinYuGouMai], 0.95);
			if (mchPoint.x == -1)
			{
				//无法使用信誉购买
				//mchPoint = Unitl::MatchMat(tempMat,
				//	m_MatMMap["UiMark"][EUiMark::EUMXianJinGouMai], 0.98);
				//if (mchPoint.x == -1) {
				//	//也无法使用现金购买
				//	return OPERATOR_FAILED;
				//}
				Sleep(100);
				continue;
			}
			mchPoint += shotRect.tl();
			mchPoint.x += 20;
			mchPoint.y += 10;
			MouseMoveLBClick(mchPoint);
			Sleep(200);
			return OPERATOR_SUCCESS;
		}
		return OPERATOR_FAILED;
	}
	return OPERATOR_FAILED;
}

/*
*	检查到任务打开则立刻返回，否则检查一段时间再返回
*/
BOOL SwClient::IsRenWuOpen()
{
	int tryTime = 5;
	cv::Point mchPoint(-1, -1);
	for (; tryTime > 0; tryTime--) {
		cv::Mat tempMat;
		cv::Rect shotRect(m_OriginPoint.x + 40, m_OriginPoint.y + 40, 200, 200);
		if (GetShotArea(shotRect, tempMat)) {
			mchPoint = Unitl::MatchMat(tempMat,
				m_MatMMap["UiMark"][EUiMark::EUMRenWu], 0.95);
			if (mchPoint.x != -1)
			{
				return true;
			}
		}
		Sleep(30);
	}
	return false;
}

cv::Point SwClient::OpenWuPin()
{
	int tryTime = 3;
	cv::Point mchPoint(-1, -1);
	OpenUi(EUMWuPin, mchPoint);
	return mchPoint;
}

cv::Point SwClient::OpenRenWu()
{
	int tryTime = 3;
	cv::Point mchPoint(-1, -1);
	OpenUi(EUMRenWu, mchPoint);
	return mchPoint;
}

cv::Point SwClient::OpenRiChang()
{
	int tryTime = 3;
	cv::Point mchPoint(-1, -1);
	OpenUi(EUMRiCheng, mchPoint);
	return mchPoint;
}

cv::Point SwClient::OpenJiShouZhongXin()
{
	int tryTime = 3;
	cv::Point mchPoint(-1, -1);
	OpenUi(EUMJiShouZhongXin, mchPoint);
	Sleep(500);//物品加载有延迟
	m_cashStorePoint = mchPoint;
	return mchPoint;
}


cv::Point SwClient::OpenJiaoYiZhongXin()
{
	int tryTime = 3;
	cv::Point mchPoint(-1, -1);
	OpenUi(EUMJiaoYiZhongXin, mchPoint);
	Sleep(500);//物品加载有延迟
	return mchPoint;
}

/*
*	返回地图左上角坐标，坐标相对于整个屏幕，并非地图上的坐标
*	失败 返回(-1,-1)
*/
cv::Point SwClient::OpenDiTu()
{
	cv::Point mchPoint(-1, -1);	//寻找世界bmp的点
	cv::Mat mapMat;
	OpenUi(EUMDiTu, mchPoint);
	if (mchPoint.x != -1)
	{
		//mchPoint指向地图左上角
		mchPoint += cv::Point(-1, 24);
	}
	return mchPoint;
}


void SwClient::CloseRenWu()
{
	CloseUi(EUMRenWu);
}
void SwClient::CloseWuPin()
{
	CloseUi(EUMWuPin);
}
void SwClient::CloseJiShouZhongXin()
{
	CloseUi(EUMJiShouZhongXin);
}
void SwClient::CloseJiaoYiZhongXin()
{
	CloseUi(EUMJiaoYiZhongXin);
}
void SwClient::CloseDiTu()
{
	CloseUi(EUMDiTu);
}

void SwClient::CloseRiChang()
{
	CloseUi(EUMRiCheng, true);
}

ECashStoreGoodsType SwClient::FindNeedGoodsType(cv::Point jszxPoint)
{
	ECashStoreGoodsType gt = ECashStoreGoodsType::ECSGTDefault;
	cv::Mat findXuMat;
	cv::Point mchPoint;
	cv::Mat tempMat;
	cv::Rect shotRect(m_OriginPoint.x + jszxPoint.x + 32,
		m_OriginPoint.y + jszxPoint.y + -28, 30, 350);
	if (GetShotArea(shotRect, tempMat))
	{
		mchPoint = Unitl::MatchMat(tempMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\xu.bmp", 0.98);
		if (mchPoint.x == -1)
		{
			goto DONE;
		}
		else
		{
			gt = ECashStoreGoodsType(1 + mchPoint.y
				/ (SwComm::GoodsTypeHeight + SwComm::GoodsTypeSpaceHeight));
		}
	}
DONE:
	return gt;
}

int SwClient::IsZiDongXunLuNow()
{
	BOOL b_ret(false);
	cv::Point mchPoint;
	cv::Mat mapMat;
	int tryTime = 3;
	for (; tryTime > 0; tryTime--)
	{
		cv::Rect shotRect(m_OriginPoint.x + 0, m_OriginPoint.y + 310, 300, 260);
		if (!GetShotArea(shotRect, mapMat))
		{
			b_ret = false;
			goto DONE;
		}
		mchPoint = Unitl::MatchMat(mapMat, m_MatMMap["UiMark"][EUiMark::EUMZiDongXunLu], 0.98);
		if (mchPoint.x != -1)
		{
			b_ret = true;
			goto DONE;
		}
		Sleep(100);
	}
	if (tryTime == 0)
	{
		b_ret = false;
	}
DONE:
	return b_ret;
}

int SwClient::WaitForZiDongXuLu(ERiChang rc)
{
	BOOL b_ret(false);
	cv::Point mchPoint;
	cv::Mat mapMat;
	int tryTime = 300000;
	switch (rc)
	{
	case ERCShiMen:
		tryTime = 35000;//35秒检测行走，最长的清河走了20+s
		break;
	case ERCLingHuLeYuan:
		break;
	case ERCXiuLian:
		tryTime = 180000;
	default:
		break;
	}
	for (; tryTime > 0; tryTime -= 100)
	{
		cv::Rect shotRect(m_OriginPoint.x + 0, m_OriginPoint.y + 310, 300, 260);
		if (!GetShotArea(shotRect, mapMat))
		{
			b_ret = false;
			goto DONE;
		}
		mchPoint = Unitl::MatchMat(mapMat, m_MatMMap["UiMark"][EUiMark::EUMZiDongXunLu], 0.98);
		if (mchPoint.x == -1)
		{
			b_ret = true;
			goto DONE;
		}
		Sleep(100);
	}
	if (tryTime == 0)
	{
		b_ret = false;
	}
DONE:
	return b_ret;
}

/*
*	在任务中查找完成的bmp
*/
bool SwClient::IsRenWuComplete()
{

	cv::Mat taskMat;
	cv::Point taskPoint = ShotRenWuWithOpenUi(taskMat, ERCDefault);
	if (Unitl::MatchMat(taskMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\wancheng.bmp", 0.98).x != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//购买吕宋果需要用现金购买
int SwClient::BuyDianPuGoodsAndColseUi()
{
	int tryTime = 3;
	cv::Mat tempMat;
	cv::Point mchPoint, clickPoint;
	cv::Point shotStartP(95, 385);
	//左侧，相对于屏幕
	cv::Rect rect(m_OriginPoint.x + shotStartP.x, m_OriginPoint.y + shotStartP.y,
		458, 158);
	for (; tryTime > 0; tryTime--)
	{
		if (!GetShotArea(rect, tempMat))
		{
			return SHOT_ERROR;
		}
		mchPoint = Unitl::MatchMat(tempMat,
			m_MatMMap["UiMark"][EUiMark::EUMXinYuGouMai], 0.98);
		if (mchPoint.x == -1)
		{
			//无法使用信誉购买
			mchPoint = Unitl::MatchMat(tempMat,
				m_MatMMap["UiMark"][EUiMark::EUMXianJinGouMai], 0.98);
			if (mchPoint.x == -1) {
				//也无法使用现金购买
				return OPERATOR_FAILED;
			}
		}
		mchPoint += shotStartP;
		clickPoint = mchPoint + cv::Point(20, 10);
		MouseMoveLBClick(clickPoint);
		Sleep(500);
		clickPoint = mchPoint + cv::Point(-20, 10);
		MouseMoveRBClick(clickPoint);
		Sleep(300);
		return OPERATOR_SUCCESS;
	}
	return OPERATOR_FAILED;
}

int SwClient::HandleRenWuBuyGoodsInSWB()
{
	cv::Point retPoint = OpenJiaoYiZhongXin();
	cv::Point mchPoint;
	cv::Mat tempMat;
	cv::Rect rect(0, -10, 200, 200);
	rect += retPoint;
	rect += m_OriginPoint;
	if (GetShotArea(rect, tempMat)) {
		mchPoint = Unitl::MatchMat(tempMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\zhuangbeixiangguan.bmp", 0.98);
		if (mchPoint.x != -1)
		{
			MouseMoveLBClick(retPoint + mchPoint);
			Sleep(300);
		}
		else {
			if (GetShotArea(rect, tempMat)) {
				mchPoint = Unitl::MatchMat(tempMat,
					Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\zhuangbeixiangguan_active.bmp", 0.98);
				if (mchPoint.x == -1)
				{
					return OPERATOR_FAILED;
				}
			}
		}
		MouseMoveLBClick(retPoint + cv::Point(92, 38));
		Sleep(300);

		cv::Rect shotRect(m_OriginPoint.x, m_OriginPoint.y, 200, 200);
		if (GetShotArea(shotRect, tempMat)) {
			mchPoint = Unitl::MatchMat(tempMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\3jizhuangbeiling.bmp", 0.98);
			if (mchPoint.x != -1)
			{
				MouseMoveLBClick(retPoint + cv::Point(458, 103));
				Sleep(300);
				MouseMoveLBClick(retPoint + cv::Point(468, 379));
				Sleep(300);
				CloseJiaoYiZhongXin();
				return OPERATOR_SUCCESS;
			}
			else {
				return OPERATOR_FAILED;
			}
		}
	}
	return OPERATOR_SUCCESS;
}

/*
*	在寄售中心购买任务物品，先查找带需求标记的任务，然后扫描一定页数购买价格最小的物品，
*	返回前检查任务是否完成
*/
int SwClient::HandleRenWuBuyGoodsInCashStore()
{
	cv::Point jszxPoint;
	int tryTime = 5;
	int s4_ret(OPERATOR_SUCCESS);

	for (; tryTime > 0; tryTime--)
	{
		if (tryTime != 0) {
			//可能是是商品刷新了导致购买失败，第二次重新打开Ui
			CloseJiShouZhongXin();
			Sleep(300);
		}
		jszxPoint = OpenJiShouZhongXin();
		if (jszxPoint.x == -1)
		{
			continue;
		}
		//寻找任务品类型，带'需'字的位置
		ECashStoreGoodsType csg = FindNeedGoodsType(jszxPoint);
		if (csg == ECashStoreGoodsType::ECSGTDefault)
		{
			//身上有该物品
			CloseJiShouZhongXin();
			s4_ret = OPERATOR_SUCCESS;
			goto DONE;
		}
		else
		{
			Sleep(DELAYMORETIMR + 200);
			MouseMoveClickLeftTop();
			BOOL isFound = false;
			cv::Mat taskGoodsMat;
			int curPage, page, row, col;
			cv::Mat pageMat;
			//寻找任务需要的物品,扫描30页，确定所需物品
			for (curPage = 0; !isFound && curPage < 30; curPage++)
			{
				cv::Rect shotRect(0, 0, 260, 400);
				shotRect += jszxPoint;
				shotRect += cv::Point(172, -21);
				if (!GetShotArea(shotRect + m_OriginPoint, pageMat))
				{
					return SHOT_ERROR;
				}
				/*cv::imshow("pageMat", pageMat);
				cv::waitKey(2323232);*/
				for (int i = 0; !isFound && i < 5; i++)
				{
					for (int j = 0; !isFound && j < 5; j++)
					{
						cv::Mat goodsIcon = GetGoodsIcon(pageMat, i, j);
						int k = 0;
						for (; k < goodsIcon.cols; k++)
						{
							int pixIndex = goodsIcon.channels()*k;
							if (goodsIcon.data[pixIndex] == 40 &&
								goodsIcon.data[pixIndex + 1] == 36 &&
								goodsIcon.data[pixIndex + 2] == 176)
							{
								taskGoodsMat = goodsIcon;
								AddMaskForGoods(taskGoodsMat);
								isFound = true;
								break;
							}
						}
					}
				}
				if (!isFound) {
					GoodsNextPage();
					Sleep(DELAYMORETIMR + 600);
				}
				else {
					break;
				}
			}
			//确定了所需物品
			if (isFound)
			{
				/*for (int p = 0; p < curPage; p++)
				{
				GoodsPrePage();
				Sleep(DELAYMORETIMR + 300);
				}*/
				//寻找价格最小的物品，默认寄售中心已经打开
				//int scanfPageSize = 25 - curPage;
				int scanfPageSize = 10;
				int priceMin = INT_MAX;
				if (curPage >= 30) {
					continue;
				}
				if (scanfPageSize + curPage > 29)
				{
					scanfPageSize = 29 - curPage;
				}
				if (GetMinPriceGood(jszxPoint, taskGoodsMat, scanfPageSize, priceMin, page, row, col) != OPERATOR_SUCCESS)
				{
					continue;
				}
				else {
					if (priceMin > m_CashGoodsPricesMax)
					{
						LOG_MSG(DINFO, "价格大于%d.购买失败。", m_CashGoodsPricesMax);
						s4_ret = OPERATOR_FAILED;
						goto DONE;
					}
					//GetMinPriceGood 移动了商品页，购买商品前需要切换到page
					for (int p = 0; p < scanfPageSize - page; p++)
					{
						GoodsPrePage();
						Sleep(DELAYMORETIMR + 500);
					}
					cv::Rect shotRect(0, 0, 260, 400);
					shotRect += jszxPoint;
					shotRect += cv::Point(172, -21);
					if (!GetShotArea(shotRect + m_OriginPoint, pageMat))
					{
						return SHOT_ERROR;
					}
					CashStoreGoods goods[25];
					int reCheckRow = -1, reCheckCol = -1;
					if (GetPageGoodsInfo(pageMat, goods, taskGoodsMat) == OPERATOR_SUCCESS)
					{
						for (int j = 0; j < 25; j++)
						{
							if (goods[j].GoodsEnum == ECSGTDefault) //Mat匹配成功项
							{
								if (goods[j].prices == priceMin)
								{
									priceMin = goods[j].prices;
									page = curPage;
									reCheckRow = j / 5;
									reCheckCol = j % 5;
									break;
								}
							}
						}
						//recheck pass
						if (reCheckRow != -1 && reCheckCol != -1)
						{
							BuyCashStoreGoods(reCheckRow, reCheckCol);
							Sleep(DELAYMORETIMR + 200);
							CloseJiShouZhongXin();
							s4_ret = OPERATOR_SUCCESS;
							goto DONE;
						}
						else {
							bool notmch = true;
							continue;
						}
					}
				}
			}
		}
	}
	if (tryTime == 0)
	{
		s4_ret = OPERATOR_FAILED;
	}
DONE:
	CloseJiShouZhongXin();
	if (s4_ret == OPERATOR_SUCCESS && !IsRenWuComplete())
	{
		s4_ret = OPERATOR_FAILED;
	}
	return s4_ret;
}

/*
*	任务ui已打开
*/
int SwClient::HandleRenWuBuyGoodsInDianPu(cv::Mat& targetMat, cv::Point& targetPoint)
{
	int tryTime;
	int s4_ret(OPERATOR_SUCCESS);
	int fly_ret(OPERATOR_FAILED);
	EMap city;
	ENormalStoreType nst = ENSTFuShiDian;
	cv::Point maskClickP;
	//要去的店铺类型
	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\wuqidian.bmp", 0.98).x != -1)
	{
		nst = ENSTWuQiDian;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\yaodian.bmp", 0.98).x != -1)
	{
		nst = ENSTWuQiDian;
	}

	/*else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\wuqidian.bmp", 0.98).x != -1)
	{
		nst = ENSTWuQiDian;
	}*/

	//要飞的城市
	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\linxianzhen.bmp", 0.98).x != -1)
	{
		city = EMLinXianZhen;//飞临仙镇
		if (nst == ENSTWuQiDian)
		{
			fly_ret = FlyWithFeiXingQi(city, cv::Point(268, 389));
		}
		else {
			fly_ret = FlyWithFeiXingQi(city, cv::Point(177, 338));
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\aolaiguo.bmp", 0.98).x != -1)
	{
		city = EMAoLaiGuo;
		if (nst == ENSTWuQiDian)
		{
			fly_ret = FlyWithFeiXingQi(city, cv::Point(580, 250));
		}
		else {
			fly_ret = FlyWithFeiXingQi(city, cv::Point(450, 170));
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\changancheng.bmp", 0.98).x != -1)
	{
		city = EMChangAnCheng;
		if (nst == ENSTWuQiDian)
		{
			fly_ret = FlyWithFeiXingQi(city, cv::Point(669, 354));
		}
		else if (nst == ENSTWuQiDian)
		{
			fly_ret = FlyWithFeiXingQi(city, cv::Point(614, 371));
		}
		else {
			fly_ret = FlyWithFeiXingQi(city, cv::Point(650, 400));
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\qinghezhen.bmp", 0.98).x != -1)
	{
		city = EMQingHeZhen;
		if (nst == ENSTWuQiDian)
		{
			fly_ret = FlyWithFeiXingQi(city, cv::Point(458, 131));
		}
		else {
			fly_ret = FlyWithFeiXingQi(city, cv::Point(571, 345));
		}
	}
	else {
		int other = 1;
	}
	if (fly_ret != OPERATOR_SUCCESS)
	{
		if (FlyWithFeiXingQi(city, true) != OPERATOR_SUCCESS) {
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
	}
	OpenRenWu();
	maskClickP = Unitl::FindPixelOnTail(targetMat, 0, 253, 253, 4);
	maskClickP += targetPoint;
	maskClickP -= cv::Point(3, 3);
	MouseMoveLBClick(maskClickP);//点击要去买物品的地方

	for (tryTime = 3; tryTime > 0; tryTime--)
	{
		if (tryTime != 3)
		{
			OpenRenWu();
			MouseMoveLBClick(maskClickP);//点击要去买物品的地方
		}
		if (!IsDuiHuaKuangExists(30000))
		{
			continue;
		}
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
		{
			continue;
		};
		Sleep(500);
		MouseMoveClickLeftTop();
		Sleep(500);//进入店铺等待触发商品界面
		if (BuyDianPuGoodsAndColseUi() == OPERATOR_SUCCESS)
		{
			s4_ret = OPERATOR_SUCCESS;
			goto DONE;
		}
		else {
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
	}
DONE:
	Sleep(100);
	CloseRenWu();
	return s4_ret;
}

/*
*	任务处于打开状态
*/
int SwClient::ShiMenHandleShouJiWuZi(cv::Mat& taskMat, cv::Point taskPoint)
{
	int tryTime = 0;
	EMap city;
	cv::Point flyPoint(-1, -1);
	cv::Mat tempMat, taskListMat;
	cv::Point taskTargetP;
	taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, tempMat);
	GetRenWuListMat(taskMat, taskPoint, taskListMat);
	bool isComplete = false;
	if (Unitl::MatchMat(taskMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\wancheng.bmp", 0.98).x != -1)
	{
		isComplete = true;
	}

	if (Unitl::MatchMat(tempMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\jiaoyizhongxin.bmp", 0.98).x != -1)
	{
		CloseRenWu();
		if (!isComplete && HandleRenWuBuyGoodsInCashStore() != OPERATOR_SUCCESS)
		{
			return OPERATOR_FAILED;
		}
		//提交物品


		RiChangShiMenSeeMenPaiMaster();
		return OPERATOR_SUCCESS;
		//以下弃用
		//这里打开了日程界面
		cv::Point riChangPoint = OpenRiChang();
		if (riChangPoint.x == -1)
		{
			CloseRiChang();
			return OPERATOR_FAILED;
		}
		cv::Rect shotRect(m_OriginPoint.x + riChangPoint.x + 30,
			m_OriginPoint.y + riChangPoint.y + 52, 680, 132);
		cv::Mat shotMat;
		cv::Point matchPoint;
		BOOL isFinish = false;
		if (GetShotArea(shotRect, shotMat))
		{
			matchPoint = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][EUMShiMenRenWu_WeiWanCheng1], 0.98);
			if (matchPoint.x == -1)

			{
				matchPoint = Unitl::MatchMat(shotMat,
					m_MatMMap["UiMark"][EUMShiMenRenWu_WeiWanCheng2], 0.98);
				if (matchPoint.x == -1)
				{
					CloseRiChang();
					return OPERATOR_FAILED;
				}
			}
			matchPoint.x = riChangPoint.x + 30 + matchPoint.x + 30;
			matchPoint.y = riChangPoint.y + 52 + matchPoint.y + 10;
			//点击日常的图标，直达门派师傅的脚跟
			MouseMoveLBClick(matchPoint);
			CloseRiChang();
		}
		return OPERATOR_SUCCESS;
	}
	else {
		if (!isComplete && HandleRenWuBuyGoodsInDianPu(tempMat, taskTargetP) != OPERATOR_SUCCESS)
		{
			return OPERATOR_FAILED;
		}
		RiChangShiMenSeeMenPaiMaster();
	}
	return OPERATOR_SUCCESS;
}

int SwClient::GotoExceptCityApart(EMap location)
{
	int s4ret(OPERATOR_SUCCESS);
	cv::Mat ztMat;
	GetNormalZhuangTaiMat(ztMat);
	if (Unitl::MatchMat(ztMat, m_MatMMap["UiMark"][EUMQuMoXiang], 0.98).x == -1)
	{
		cv::Point wpPoint = OpenWuPin();
		cv::Mat goodsMat;
		cv::Rect rect(325, -50, 250, 390);
		rect += wpPoint;
		GetShotArea(rect + m_OriginPoint, goodsMat);
		cv::Point qmxPoint = Unitl::MatchMat(goodsMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\wupin\\qumoxiang.bmp", 0.98);
		if (qmxPoint.x != -1)
		{
			qmxPoint += rect.tl();
			MouseMoveRBClick(qmxPoint);
		}
		CloseWuPin();
		//使用驱魔香
	}
	switch (location)
	{
	case EMFoMen:
		if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(508, 101)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMChangAnCheng);
		}
		break;
	case EMTianCe:
		if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(108, 334)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMChangAnCheng);
		}
		break;
	case EMQiXingFangCun:
		if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(145, 129)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMLinXianZhen);
		}
		break;
	case EMTianMoLi:
		if (FlyWithFeiXingQi(EMAoLaiGuo, cv::Point(640, 104)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMAoLaiGuo);
		}
		break;
	case EMLingXiaoTianGong:
		if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(629, 129)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMLinXianZhen);
		}
		break;
	case EMDongHaiLongGong:
		if (FlyWithFeiXingQi(EMQingHeZhen, cv::Point(609, 123)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMQingHeZhen);
		}
		break;
	case EMNanHaiPuTuo:
		//点驿站飞
		if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(347, 414)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMChangAnCheng);
		}
		break;
	case EMZhenYuanWuZhuang:
	case EMLMoWangShan:
	case EMWanShouLing:
		if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(631, 446)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMLinXianZhen);
		}
		break;
	case EMYouMingDiFu:
	case EMPanSiLing:
	case EMWuMingGu:
		if (FlyWithFeiXingQi(EMNvErGuo, cv::Point(619, 425)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMNvErGuo);
		}
		break;
	default:
		break;
	}
	return s4ret;
}

EMap SwClient::ShiMenMatchMenPaiMap(cv::Mat& targetMat)
{
	std::vector<std::pair<EMap, std::string>> nPMap = {
		std::pair<EMap, std::string>(EMap::EMFoMen,"fomen.bmp"),
		std::pair<EMap, std::string>(EMap::EMTianCe,"tiance.bmp"),
		std::pair<EMap, std::string>(EMap::EMQiXingFangCun,"qixingfangcun.bmp"),
		std::pair<EMap, std::string>(EMap::EMTianMoLi,"tianmoli.bmp"),

		std::pair<EMap, std::string>(EMap::EMLingXiaoTianGong,"lingxiaotiangong.bmp"),
		std::pair<EMap, std::string>(EMap::EMDongHaiLongGong,"donghailonggong.bmp"),
		std::pair<EMap, std::string>(EMap::EMNanHaiPuTuo,"nanhaiputuo.bmp"),
		std::pair<EMap, std::string>(EMap::EMZhenYuanWuZhuang,"zhenyuanwuzhuang.bmp"),

		std::pair<EMap, std::string>(EMap::EMYouMingDiFu,"youmingdifu.bmp"),
		std::pair<EMap, std::string>(EMap::EMLMoWangShan,"mowangshan.bmp"),
		std::pair<EMap, std::string>(EMap::EMWanShouLing,"wanshouling.bmp"),
		std::pair<EMap, std::string>(EMap::EMPanSiLing,"pansiling.bmp"),
		std::pair<EMap, std::string>(EMap::EMWuMingGu,"wuminggu.bmp")
	};
	for (auto el : nPMap)
	{
		if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\" + el.second, 0.98).x != -1)
		{
			return el.first;
		}
	}
	return EMap::EMDefault;
}

/*
*	任务处于打开状态
*/
int SwClient::ShiMenHandleTiaoZhan(cv::Mat& targetMat, cv::Point taskTargetP)
{
	int tryTime = 0;
	int s4ret(OPERATOR_SUCCESS);
	cv::Point flyPoint(-1, -1);
	cv::Mat tempMat, taskListMat;
	EMap ditu = ShiMenMatchMenPaiMap(targetMat);
	if (ditu == EMap::EMDefault)
	{
		s4ret = OPERATOR_FAILED;
		goto DONE;
	}
	else
	{
		bool pasue = true;
	}
	GotoExceptCityApart(ditu);
	HandleMenPaiXunLuo(targetMat, taskTargetP);
DONE:
	return s4ret;
}


/*
*	开始出手后检查当前回合是否结束 或 战斗结束, 阻塞检查
*/
int SwClient::WaitForNextHuiHe(int timeOut)
{
	cv::Mat commMat;
	int useTime = 0;
	int delayTime = 100;
	MouseMoveClickLeftTop();
	cv::Rect shotRect(m_OriginPoint.x + 644, m_OriginPoint.y + 111, 154, 80);
	bool isStartFight = false;
	for (useTime = 0; useTime < timeOut; useTime += delayTime)
	{
		if (IsFighting())
		{
			isStartFight = true;
		}
		else {
			if (isStartFight) {
				return FIGHTOVER;
			}
		}
		Sleep(delayTime);
		if (GetShotArea(shotRect, commMat)) {
			if (Unitl::MatchMat(commMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\fight\\fashu.bmp", 0.98).x != -1)
			{
				return WAITFIGHTCOMMAND;
			}
		}
	}
	return OPERATOR_FAILED;
}

/*
*
*	第二排未完成检测
*/
int SwClient::GetZhanChangMastMonster(EMonster MonsterName, std::vector<cv::Point>& pointVec)
{
	//x Increment 28
	pointVec.clear();
	cv::Point firstTargetPoint(354, 510);
	//存放找到目标的Point
	for (int row = 0; row < 2; row++)
	{
		if (row == 1)break;
		for (int i = 0; i < 5; i++)
		{
			cv::Point targetPont(firstTargetPoint.x + i * 28, firstTargetPoint.y);
			//54, 26;
			cv::Mat tNameMat;
			//鼠标悬浮在目标图标上，得到怪物名称图片
			usbMutex.lock();
#ifdef MULTICLIENT
			GetFocusDelayLess();
#endif
			UsbCommDuration(60);
			MouseOperator mo(MoveAbs, m_OriginPoint.x + targetPont.x, m_OriginPoint.y + targetPont.y);
			MyUsbMgr::GetInstance()->SendMouseOperator(mo);
			usbMutex.unlock();
			//MouseMoveAbs(targetPont.x, targetPont.y);

			//怪物名称图片的Rect
			Sleep(200);
			cv::Rect tNameRect(m_OriginPoint.x, m_OriginPoint.y, 200, 32);
			//第一排的怪物名称图片位置与第二排的不一样
			if (row == 0)
			{
				tNameRect.x += targetPont.x - 100;
				tNameRect.y += targetPont.y - 47;
			}
			else
			{
				/*	tNameRect.x += targetPont.x - 100;
					tNameRect.y += targetPont.y - 47;*/
			}
			if (!GetShotArea(tNameRect, tNameMat)) {
				return SHOT_ERROR;
			}
			std::string mchFilePath = Unitl::GetAppPathS() + SWPICTUREDIR + "\\fight\\";
			switch (MonsterName)
			{
			case Defaule:
				break;
			case HuXiaoMeng:
				mchFilePath += "huxiaomeng.bmp";
				break;
			default:
				break;
			}
			if (Unitl::MatchMat(tNameMat, mchFilePath, 0.98).x != -1)
			{
				pointVec.push_back(targetPont);
			}
		}
	}
	return OPERATOR_SUCCESS;
}

void SwClient::ZhuangTaiHuiFu()
{
	MouseMoveRBClick(CLIENTWIDTH - 50, 8);
	Sleep(DELAYMORETIMR + 50);
	MouseMoveRBClick(CLIENTWIDTH - 150, 8);
	Sleep(DELAYMORETIMR + 50);
	MouseMoveRBClick(CLIENTWIDTH - 50, 20);
	Sleep(DELAYMORETIMR + 50);
	MouseMoveRBClick(CLIENTWIDTH - 150, 20);
	Sleep(DELAYMORETIMR + 50);
	MouseMoveRBClick(CLIENTWIDTH - 150, 42);
	Sleep(DELAYMORETIMR + 50);
}
BOOL SwClient::TurnOnGuaiWuTuBiao()
{
	cv::Point mchPoint;
	cv::Mat shotMat;
	cv::Rect shotRect(m_OriginPoint.x + 340, m_OriginPoint.y + 488, 460, 75);
	if (GetShotArea(shotRect, shotMat)) {
		mchPoint = Unitl::MatchMat(shotMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\fight\\guaiwutubiao.bmp", 0.98);
		if (mchPoint.x == -1)
		{
			mchPoint = Unitl::MatchMat(shotMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\fight\\mubiao.bmp", 0.98);
			if (mchPoint.x != -1)
			{
				MouseMoveLBClick(mchPoint.x + 340, mchPoint.y + 488);
			}
			else {
				return false;
			}
		}
	}
	return true;
}
int SwClient::BuZhuoHuXiaoMeng()
{
	//x Increment 28
	std::vector<cv::Point> pointVec;
	TurnOnGuaiWuTuBiao();
	bool huifuFlag = false;
	while (true)
	{
		//获取目标狐小萌
		GetZhanChangMastMonster(EMonster::HuXiaoMeng, pointVec);
		//pointVec = 0 的情况,全杀
		if (pointVec.size() > 0)
		{
			//is huxiaomeng
			KeyBtnClickWithAlt('G');
			Sleep(200);
			MouseMoveLBClick(pointVec[0]);
			Sleep(200);
			//携带宠物
			KeyBtnClickWithAlt('D');
			KeyBtnClickWithAlt('D');	//发送多一次指令
		}
		else {
			KeyBtnClickWithAlt('Q');
			KeyBtnClickWithAlt('Q');
			KeyBtnClickWithAlt('Q');	//发送多一次指令
		}
		if (!huifuFlag) {
			Sleep(500);
			ZhuangTaiHuiFu();
			huifuFlag = true;
		}
		int ret = WaitForNextHuiHe();
		if (ret == WAITFIGHTCOMMAND)
		{
			continue;
		}
		else if (ret == FIGHTOVER)
		{
			return OPERATOR_SUCCESS;
		}
		else
		{
			return OPERATOR_FAILED;
		}
	}
	return OPERATOR_SUCCESS;
}

/*
*	以检查存在为主，存在时立刻返回，不存在时多次检查
*/
BOOL SwClient::IsDuiHuaKuangExists(int waitMs)
{
	cv::Rect shotRect(203, 389, 70, 70);
	cv::Mat shotMat;
	MouseMoveClickLeftTop();
	int useTime = 0;
	int startTime = GetTickCount();
	//部分地方可能响应慢，需要多次检查
	for (; useTime < waitMs; )
	{
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			cv::Point mchClickP = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][EUMDuiHuaKuang], 0.98);
			if (mchClickP.x != -1)
			{
				return true;
			}
		}
		Sleep(50);
		useTime = GetTickCount() - startTime;
	}
	return false;
}



void SwClient::CloseDuiHuaKuang(int waitMs, BOOL Multi)
{
	cv::Rect shotRect(203, 389, 70, 70);
	cv::Mat shotMat;
	int useTime = 0;
	MouseMoveClickLeftTop();
	int startTime = GetTickCount();
	int isDone = false;
	if (Multi)
	{
		waitMs = waitMs < 1500 ? 1500 : waitMs;
	}
	for (; useTime < waitMs; )
	{
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			cv::Point mchClickP = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][EUMDuiHuaKuang], 0.98);;
			if (mchClickP.x != -1)
			{
				mchClickP += shotRect.tl() + cv::Point(-50, 0);
				MouseMoveRBClick(mchClickP);
				Sleep(100);
				MouseMoveClickLeftTop();
				Sleep(100);
				isDone = true;
			}
			else
			{
				if (isDone)
				{
					return;
				}
			}
		}
		if (Multi)
		{
			Sleep(300);
		}
		else
		{
			Sleep(100);
		}
		useTime = GetTickCount() - startTime;
	}
	return;
}


int SwClient::ChoiceDuiHuaKuangFirstOption()
{
	MouseMoveClickLeftTop();
	cv::Rect shotRect(272, 284, 120, 140);
	cv::Mat mat;
	if (!IsDuiHuaKuangExists())
	{
		return OPERATOR_FAILED;
	}
	Sleep(300);
	if (GetShotArea(shotRect + m_OriginPoint, mat)) {
		cv::Point maskClickP = Unitl::FindPixel(mat, 0, 253, 253, 4);
		if (maskClickP.x != -1) {
			MouseMoveLBClick(shotRect.x + maskClickP.x,
				shotRect.y + maskClickP.y);
			return OPERATOR_SUCCESS;
		}
	}
	return OPERATOR_FAILED;
}

/*
*	比较当前地图
*/
BOOL SwClient::MatchDiTu(EMap em)
{
	BOOL b_ret(false);
	cv::Mat mapMat;
	cv::Point mchPoint;
	cv::Rect rect;
	mchPoint = OpenDiTu();//打开了地图
	if (mchPoint.x == -1)
	{
		b_ret = false;
		goto DONE;
	}
	rect += m_OriginPoint + mchPoint;
	rect += cv::Size(40, 40);
	if (!GetShotArea(rect, mapMat)) {
		b_ret = false;
		goto DONE;
	}
	/*cv::Rect saveRect(0, 0, 20, 20);
	cv::imwrite(Unitl::GetAppPathS() + SWPICTUREDIR + "\\map\\fomen.bmp",
		mapMat(saveRect));*/
	if (Unitl::MatchMat(mapMat,
		m_MatMMap["Map"][em], 0.98).x != -1)
	{
		b_ret = true;
		goto DONE;
	}
DONE:
	CloseDiTu();
	Sleep(80);
	return b_ret;
}


int SwClient::AutoMeetMonster()
{
	BOOL b_ret(false);
	cv::Mat mapMat;
	std::string filePath;
	static int AutoMeetMonsterClickCount = 0;
	int tryTime = 0;
	int mapWidth, mapHeight;
	cv::Point mapLT;
	for (int k = 0; k < 360000; )
	{
		if (IsFighting())
		{
			b_ret = true;
			Sleep(250);//等待战场ui切换
			goto DONE;
		}
		//检查移动
		if (k % 5500 == 0)
		{
			if (AutoMeetMonsterClickCount % 2 == 0)
			{
				WalkWithOpenDiTu(EMLingHuLeYuan, DiTuLoc(10, 10));
			}
			else {
				WalkWithOpenDiTu(EMLingHuLeYuan,
					DiTuLoc(SwComm::DiTuZuoBiaoMax[EMLingHuLeYuan].x - 20,
						SwComm::DiTuZuoBiaoMax[EMLingHuLeYuan].y - 20));
			}
			CloseDiTu();
			AutoMeetMonsterClickCount++;
		}
		k += 500;
		Sleep(DELAYMORETIMR + 500);
	}
DONE:
	return b_ret;
}

int SwClient::GotoLingHuLeYuan()
{
	int tryTime;
	//灵狐乐园任务所需飞行不多，延迟时间多些不碍事，另一方面飞长安会卡
	for (tryTime = 0; tryTime < 3; tryTime++)
	{
		PingBiRenWu();
		Sleep(500);
		KeyBtnClick(KEYBTN_F2);
		Sleep(DELAYMORETIMR + 200);
		KeyBtnClick(0x30);//飞长安
		Sleep(DELAYMORETIMR + 200);
		MouseMoveLBClick(245, 240);
		Sleep(DELAYMORETIMR + 1500);//ui过渡
		if (!MatchDiTu(EMap::EMChangAnCheng))
		{
			continue;
		}
		WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(184, 213));
		while (isWalking(800));
		CloseDiTu();
		Sleep(500);
		MouseMoveLBClick(435, 246);			//点击狐小仙
		Sleep(DELAYMORETIMR + 800);
		//单人需要选择2次
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS) {
			return OPERATOR_FAILED;
		}
		Sleep(DELAYMORETIMR + 800);
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS) {
			return OPERATOR_FAILED;
		};
		Sleep(DELAYMORETIMR + 1500);	//ui过渡
		if (MatchDiTu(EMap::EMLingHuLeYuan))
		{
			return OPERATOR_SUCCESS;
		}
	}
	return OPERATOR_FAILED;
}

BOOL SwClient::IsRiChangFinish(ERiChang rc)
{
	MouseMoveClickLeftTop();
	cv::Rect rect;
	cv::Mat shotMat;
	BOOL isFinish = false;
	//这里打开了日程界面
	cv::Point riChangPoint = OpenRiChang();
	if (riChangPoint.x == -1)
	{
		CloseRiChang();
		Sleep(1111);
		return false;
	}
	//指定的未完成的图片
	cv::Mat *tempMat = NULL;
	switch (rc)
	{
	case ERCShiMen:
		tempMat = &m_MatMMap["UiMark"][EUiMark::EUMShiMenRenWu_WanCheng];
		break;
	case ERCLingHuLeYuan:
		tempMat = &m_MatMMap["UiMark"][EUiMark::EUMLingHuLeYuan_WanCheng];
		break;
	case ERCXiuLian:
		tempMat = &m_MatMMap["UiMark"][EUiMark::EUMXiuLian_WanCheng];
		break;
	case ERCBaoTu:
		tempMat = &m_MatMMap["UiMark"][EUiMark::EUMBaoTuRenWu_WanCheng];
		break;
	default:
		break;
	}
	rect = cv::Rect(m_OriginPoint.x + riChangPoint.x + 30,
		m_OriginPoint.y + riChangPoint.y + 52, 680, 132);
	if (GetShotArea(rect, shotMat)) {
		//找到图片则完成，remark
		if ((Unitl::MatchMat(shotMat, *tempMat, 0.98)).x != -1)
		{
			isFinish = true;
		}
	}
	CloseRiChang();
	Sleep(1111);
	//Sleep(50);	//界面影响了其他界面截图
	return isFinish;
}

int SwClient::RiChangLingHu()
{
	int tryTime = 0;
	BOOL b_ret(false);
	cv::Mat mapMat;
	cv::Point mchPoint;
	if (IsRiChangFinish(ERiChang::ERCLingHuLeYuan))
	{
		return OPERATOR_SUCCESS;
	}
	if (!MatchDiTu(EMLingHuLeYuan))
	{
		if (GotoLingHuLeYuan() == OPERATOR_FAILED)
		{
			return OPERATOR_FAILED;
		}
	}


	for (; ; )
	{
		BOOL result = AutoMeetMonster();
		if (result)
		{
			BuZhuoHuXiaoMeng();
			Sleep(DELAYMORETIMR + 500);//界面ui过度
			//是否完成
			if (IsRiChangFinish(ERiChang::ERCLingHuLeYuan))
			{
				return OPERATOR_SUCCESS;
			}
		}
		else {
			return OPERATOR_FAILED;
		}
	}
	return OPERATOR_SUCCESS;
}


int SwClient::RiChangLingHuMultiClient()
{
	int s4_ret(OPERATOR_SUCCESS);

	return s4_ret;
}

cv::Point SwClient::ShotRenWuWithOpenUi(cv::Mat& taskMat, ERiChang erc)
{
	std::string yBmp = "";
	switch (erc)
	{
	case ERCShiMen:
		yBmp = Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\youshimen.bmp";
		break;
	case ERCLingHuLeYuan:
		break;
	case ERCXiuLian:
		yBmp = Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\youxiulian.bmp";
		break;
	case ERCBaoTu:
		yBmp = Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\youbaotu.bmp";
		break;
	case ERCZhuoGui:
		yBmp = Unitl::GetAppPathS() + SWPICTUREDIR + "\\zhuogui\\youzhuogui.bmp";
		break;
	default:
		break;
	}
	cv::Point taskPoint;
	cv::Rect shotRect;
	int tryTime = 3;
	taskPoint = OpenRenWu();
	taskPoint += cv::Point(25, 0);
	shotRect = cv::Rect(0, 0, 500, 350);
	shotRect += taskPoint + m_OriginPoint;
	for (tryTime; tryTime > 0; tryTime--)
	{
		if (!GetShotArea(shotRect, taskMat))
		{
			return cv::Point(-1, -1);
		}
		/*cv::imshow("taskMat", taskMat);
		cv::waitKey(23232);*/
		//不需要比较则直接返回
		if (ERCDefault == erc)
		{
			return taskPoint;
		}
		//没有选中
		if (Unitl::MatchMat(taskMat, yBmp, 0.98).x == -1)
		{
			ChoiceRenWuInReWuLan(erc);
			Sleep(300);
			MouseMoveClickLeftTop();
		}
		else
		{
			break;
		}
	}
	if (tryTime == 0)
	{
		return cv::Point(-1, -1);
	}
	return taskPoint;
}

cv::Point SwClient::GetRenWuTargetMat(cv::Mat& taskMat, cv::Point taskPoint, cv::Mat& targetMat)
{
	cv::Point taskTargetP(280, 0);
	cv::Rect rect(taskTargetP.x, taskTargetP.y
		, taskMat.cols - taskTargetP.x, 87);
	targetMat = taskMat(rect);
	rect = rect + m_OriginPoint;
	return taskTargetP + taskPoint;
}


cv::Point SwClient::GetRenWuListMat(cv::Mat& taskMat, cv::Point taskPoint, cv::Mat& listMat)
{
	cv::Point taskListP(5, 30);
	//任务目标，相对于屏幕
	cv::Rect rect(taskListP.x, taskListP.y, 235, 300);
	listMat = taskMat(rect);
	rect = rect + m_OriginPoint;
	//返回任务列表topleft，相对于屏幕
	return taskListP + taskPoint;
}

cv::Point SwClient::GetRenWuHuiFuMat(cv::Mat& taskMat, cv::Point taskPoint, cv::Mat& huifuMat)
{
	cv::Point huifuP(280, 92);
	//任务目标，相对于屏幕
	cv::Rect rect(huifuP.x, huifuP.y, taskMat.cols - huifuP.x, 60);
	huifuMat = taskMat(rect);
	rect = rect + m_OriginPoint;
	//返回任务回复topleft，相对于屏幕
	return huifuP + taskPoint;
}

int SwClient::ChoiceRenWuInReWuLan(ERiChang rc)
{
	int tryTime = 3;
	std::string fileName;
	switch (rc)
	{
	case ERCShiMen:
		fileName = Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\zuoshimen.bmp";
		break;
	case ERCLingHuLeYuan:
		fileName = "";
		break;
	case ERCXiuLian:
		fileName = Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\zuoxiulian.bmp";
		break;
	case ERCZhuoGui:
		fileName = Unitl::GetAppPathS() + SWPICTUREDIR + "\\zhuogui\\zuozhuogui.bmp";
		break;
	case ERCBaoTu:
		fileName = Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\zuobaotu.bmp";
		break;
	default:
		break;
	}
	cv::Point taskPoint = OpenRenWu();
	if (taskPoint.x == -1)
	{
		return OPERATOR_FAILED;
	}
	cv::Mat tempMat;
	cv::Point mchPoint;
	cv::Point taskTargetP(taskPoint.x + 38, taskPoint.y + 38);
	//左侧，相对于屏幕
	cv::Rect rect(m_OriginPoint.x + taskTargetP.x, m_OriginPoint.y + taskTargetP.y,
		130, 320);
	if (!GetShotArea(rect, tempMat))
	{
		return SHOT_ERROR;
	}
	mchPoint = Unitl::MatchMat(tempMat, fileName, 0.98);
	if (mchPoint.x == -1)
	{
		//已经选了或者没有领取该任务
		return OPERATOR_FAILED;
	}
	mchPoint += taskTargetP;
	Sleep(DELAYMORETIMR + 200);
	MouseMoveLBClick(mchPoint);
	Sleep(100);
	MouseMoveLBClick(mchPoint);
	return OPERATOR_SUCCESS;
}

EMap SwClient::RenWuGetTargetPosition(cv::Mat targetMat)
{
	//提交任务，找对应的人，先走到目标所在地图
	EMap position = EMDefault;
	//优先匹配清河镇外，长安城外，和其他有交集
	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\qinghezhenwai.bmp", 0.98).x != -1)
	{
		position = EMap::EMQingHeZhenWai;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\changanchengwai.bmp", 0.98).x != -1)
	{
		position = EMap::EMChangAnChengWai;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\datangjingwai.bmp", 0.98).x != -1)
	{
		position = EMap::EMDaTangJingWai;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\changancheng.bmp", 0.98).x != -1)
	{
		position = EMap::EMChangAnCheng;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\aolaiguo.bmp", 0.98).x != -1)
	{
		position = EMap::EMAoLaiGuo;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\qinghezhen.bmp", 0.98).x != -1)
	{
		position = EMap::EMQingHeZhen;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\linxianzhen.bmp", 0.98).x != -1)
	{
		position = EMap::EMLinXianZhen;

	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\nverguo.bmp", 0.98).x != -1)
	{
		position = EMap::EMNvErGuo;
	}
	else {
		if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\nanhaiputou.bmp", 0.98).x != -1)
		{
			position = EMap::EMNanHaiPuTuo;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\qixingfangcun.bmp", 0.98).x != -1)
		{
			position = EMap::EMQiXingFangCun;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\datangguojing.bmp", 0.98).x != -1)
		{
			position = EMap::EMDaTangGuoJing;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\donghailonggong.bmp", 0.98).x != -1)
		{
			position = EMap::EMDongHaiLongGong;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\wusizang.bmp", 0.98).x != -1)
		{
			position = EMap::EMWuSiZang;
		}
		/*	else if (Unitl::MatchMat(huifuMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\fomen.bmp", 0.98).x != -1)
		{
		position = EMap::EMFoMen;
		}*/
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\jinyueyuan.bmp", 0.98).x != -1)
		{
			position = EMap::EMJinYueYuan;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\jinluandian.bmp", 0.98).x != -1)
		{
			position = EMap::EMJinLuanDian;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\wanshouling.bmp", 0.98).x != -1)
		{
			position = EMap::EMWanShouLing;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\kunlunshan.bmp", 0.98).x != -1)
		{
			position = EMap::EMKunLunShan;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\huaguoshan.bmp", 0.98).x != -1)
		{
			position = EMap::EMHuaGuoShan;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\tianmoli.bmp", 0.98).x != -1)
		{
			position = EMap::EMTianMoLi;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\tiance.bmp", 0.98).x != -1)
		{
			position = EMap::EMTianCe;
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\fomen.bmp", 0.98).x != -1)
		{
			position = EMap::EMFoMen;
		}
		else {
			int other = 1;
		}
	}
	return position;
}

/*
*	去非城市，飞行前检查驱魔香
*/
int SwClient::GotoExceptCity(EMap position, bool canFly)
{
	int tryTime = 3;
	if (!canFly)
	{

	}
	cv::Mat ztMat;
	GetNormalZhuangTaiMat(ztMat);
	if (Unitl::MatchMat(ztMat, m_MatMMap["UiMark"][EUMQuMoXiang], 0.98).x == -1)
	{
		cv::Point wpPoint = OpenWuPin();
		cv::Mat goodsMat;
		cv::Rect rect(325, -50, 250, 390);
		rect += wpPoint;
		GetShotArea(rect + m_OriginPoint, goodsMat);
		cv::Point qmxPoint = Unitl::MatchMat(goodsMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\wupin\\qumoxiang.bmp", 0.98);
		if (qmxPoint.x != -1)
		{
			qmxPoint += rect.tl();
			MouseMoveRBClick(qmxPoint);
		}
		CloseWuPin();
		//使用驱魔香
	}

	for (; tryTime > 0; tryTime--)
	{
		//门派地图
		if (position >= EMap::EMFoMen && position <= EMap::EMPanSiLing)
		{
			//门派
			switch (position)
			{
			case EMFoMen:
				if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(501, 95)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMChangAnCheng);
				}
				WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(366, 324));
				Sleep(500);
				while (isWalking(500));
				break;
			case EMTianCe:
				if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(90, 350)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMChangAnCheng);
				}
				WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(1, 122));
				Sleep(500);
				while (isWalking(500));
				break;
			case EMQiXingFangCun:
				if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(145, 141)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMLinXianZhen);
				}
				WalkWithOpenDiTu(EMLinXianZhen, DiTuLoc(9, 118));
				Sleep(500);
				while (isWalking(500));
				break;
			case EMTianMoLi:
				if (FlyWithFeiXingQi(EMAoLaiGuo, cv::Point(643, 129)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMAoLaiGuo);
				}
				WalkWithOpenDiTu(EMAoLaiGuo, DiTuLoc(155, 115));
				Sleep(500);
				while (isWalking(500));
				break;
			case EMLingXiaoTianGong:

				break;
			case EMDongHaiLongGong:
				if (FlyWithFeiXingQi(EMQingHeZhen, cv::Point(600, 129)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMQingHeZhen);
				}
				WalkWithOpenDiTu(EMQingHeZhen, DiTuLoc(155, 118));
				Sleep(500);
				while (isWalking(500));
				WalkWithOpenDiTu(EMQingHeZhenWai, DiTuLoc(110, 72));
				Sleep(100);
				CloseDiTu();
				Sleep(500);
				while (isWalking(500));
				MouseMoveLBClick(484, 245);
				if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
				{
					continue;
				};
				break;
			case EMNanHaiPuTuo:
				//点驿站飞
				if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(347, 414)) != OPERATOR_SUCCESS)
				{
					WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(218, 53));
					Sleep(40000);
				}
				else {
					WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(218, 53));
					while (isWalking(500));
				}
				if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
				{
					continue;
				};
				Sleep(500);
				WalkWithOpenDiTu(EMDaTangGuoJing, DiTuLoc(186, 165));
				CloseDiTu();
				Sleep(1000);
				while (isWalking(500));
				MouseMoveLBClick(432, 249);
				Sleep(200);
				if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
				{
					continue;
				};
				break;
			case EMZhenYuanWuZhuang:

				break;
			case EMYouMingDiFu:

				break;
			case EMLMoWangShan:

				break;
			case EMWanShouLing:
				if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(633, 441)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMLinXianZhen);
				}
				WalkWithOpenDiTu(EMLinXianZhen, DiTuLoc(192, 3));
				Sleep(500);
				while (isWalking(500));
				WalkWithOpenDiTu(EMWuSiZang, DiTuLoc(2, 7));
				Sleep(500);
				while (isWalking(500));
				break;
			case EMPanSiLing:
				if (FlyWithFeiXingQi(EMNvErGuo, cv::Point(619, 425)) != OPERATOR_SUCCESS)
				{
					FlyWithFeiXingQi(EMNvErGuo);
				}
				WalkWithOpenDiTu(EMNvErGuo, DiTuLoc(158, 12));
				Sleep(500);
				while (isWalking(500));
				WalkWithOpenDiTu(EMDaTangJingWai, DiTuLoc(303, 162));
				Sleep(500);
				while (isWalking(500));
				break;
			default:
				break;
			}
			break;
		}

		if (position == EMChangAnChengWai) {
			if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(705, 480)) != OPERATOR_SUCCESS)
			{
				if (FlyWithFeiXingQi(EMChangAnCheng) != OPERATOR_SUCCESS)
				{
					continue;
				}
			}
			WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(547, 5));
			Sleep(500);
			while (isWalking(500));
			break;
		}
		else if (position == EMDaTangJingWai) {
			if (FlyWithFeiXingQi(EMNvErGuo, cv::Point(640, 430)) != OPERATOR_SUCCESS)
			{
				if (FlyWithFeiXingQi(EMNvErGuo) != OPERATOR_SUCCESS)
				{
					continue;
				}
			}
			WalkWithOpenDiTu(EMNvErGuo, DiTuLoc(157, 10));
			Sleep(500);
			while (isWalking(500));
			break;
		}
		else if (position == EMDaTangGuoJing) {
			if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(347, 414)) != OPERATOR_SUCCESS)
			{
				if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(721, 114)) != OPERATOR_SUCCESS)
				{
					continue;
				}
				WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(548, 323));
				Sleep(500);
				while (isWalking(500));
			}
			else {
				WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(218, 53));
				Sleep(500);
				while (isWalking(500));
				if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
				{
					continue;
				};
			}
			break;
		}
		else if (position == EMWuSiZang)
		{
			if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(633, 441)) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(EMLinXianZhen);
			}
			WalkWithOpenDiTu(EMLinXianZhen, DiTuLoc(192, 3));
			Sleep(500);
			while (isWalking(500));
			break;
		}
		else if (position == EMJinLuanDian) {
			if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(133, 173)) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(EMChangAnCheng);
			}
			WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(32, 277));
			Sleep(500);
			while (isWalking(500));
			break;
		}
		else if (position == EMQingHeZhenWai)
		{
			if (FlyWithFeiXingQi(EMQingHeZhen, cv::Point(600, 129)) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(EMQingHeZhen);
			}
			WalkWithOpenDiTu(EMQingHeZhen, DiTuLoc(155, 118));
			Sleep(500);
			while (isWalking(500));
			break;
		}
		else if (position == EMKunLunShan)
		{
			if (FlyWithFeiXingQi(EMLinXianZhen, cv::Point(618, 125)) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(EMLinXianZhen);
			}
			WalkWithOpenDiTu(EMLinXianZhen, DiTuLoc(185, 117));
			Sleep(500);
			while (isWalking(500));
			break;
		}
		else if (position == EMHuaGuoShan)
		{
			if (FlyWithFeiXingQi(EMAoLaiGuo, cv::Point(180, 460)) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(EMAoLaiGuo);
			}
			WalkWithOpenDiTu(EMAoLaiGuo, DiTuLoc(2, 2));
			Sleep(500);
			while (isWalking(500));
			break;
		}

	}
	if (tryTime == 0)
	{
		return OPERATOR_FAILED;
	}
	Sleep(100);
	return OPERATOR_SUCCESS;
}


int SwClient::XiulianTiJiaoRenWu(ETakType ett, cv::Mat& taskMat, cv::Point taskPoint)
{
	//先进行飞行处理,飞行后任务ui不会消失
	int tryTime = 3;
	int s4_ret(OPERATOR_SUCCESS);
	bool reTry = true;
	cv::Mat handleMat;
	cv::Point handlePoint;

	//捕捉宠物，寻物任务回复的目标的在任务回复处，其他的在任务目标处
	if (ett == ETTBuZhuoChongWu || ett == ETTXunWu_DP ||
		ett == ETTXunWu_JSZX || ett == ETTXunWu_JYZX)
	{
		handlePoint = GetRenWuHuiFuMat(taskMat, taskPoint, handleMat);
	}
	else
	{
		handlePoint = GetRenWuTargetMat(taskMat, taskPoint, handleMat);
	}

	for (; reTry && tryTime > 0; tryTime--)
	{
		reTry = false;
		EMap position = RenWuGetTargetPosition(handleMat);
		//
		if (position == EMDefault)
		{
			break;
		}
		if (position == EMJinYueYuan)
		{
			position = EMChangAnCheng;
		}
		if (position >= EMChangAnCheng && position <= EMNvErGuo)
		{
			cv::Point huifuTargetPoint;
			if (ett == ETTZhanDou)
			{
				DiTuLoc targetZB = FindGuaiWuTargetInRenWu(taskMat, taskPoint);
				if (position >= EMChangAnCheng && position <= EMNvErGuo)
				{
					//cv::Point flyPoint(zBiao.x,zBiao.y);
					//FlyWithFeiXingQi(targetMap, flyPoint);
					cv::Point point = TranformDiTuLocToPoint(position, targetZB);
					if (FlyWithFeiXingQi(position, point, true) != OPERATOR_SUCCESS)
					{
						continue;
					}
				}
				else {
					continue;
				}
			}
			else
			{
				//目标在城市中，找人，寻物，可以寻找到问号
				huifuTargetPoint = FindHuiFuTargetInDiTu(position);
				if (huifuTargetPoint.x == -1)
				{
					//查找问号失败
					FlyWithFeiXingQi(position);
				}
				else {
					if (FlyWithFeiXingQi(position, huifuTargetPoint + cv::Point(5, 5)) != OPERATOR_SUCCESS) {
						FlyWithFeiXingQi(position);
					};
				}
			}
		}
		else {
			CloseRenWu();
			GotoExceptCity(position);
		}
	}
	cv::Point blueClickP;
	blueClickP = Unitl::FindPixelOnTail(handleMat, 0, 253, 253, 2);
	blueClickP += handlePoint;
	blueClickP -= cv::Point(3, 3);
	for (tryTime = 5; tryTime > 0; tryTime--)
	{
		cv::Mat tempMat;
		ShotRenWuWithOpenUi(tempMat, ERCXiuLian);
		MouseMoveLBClick(blueClickP);
		Sleep(150);
		CloseRenWu();
		Sleep(1000);
		if (isWalking())
		{
			while (isWalking());
			break;
		}
		Sleep(1000);
		if (IsFighting())
		{
			Sleep(800);
			break;
		}
		if (IsDuiHuaKuangExists())
		{
			break;
		}
		////部分目标人物与飞行点比较近，导致跳过了自动寻路的环节，
		////检查任务是否改变了
		//cv::Point temp = GetRenWuMatWithOpenUi(checkMat);
		//CloseRenWu();
		////找不到原任务的图，即任务已经更新
		//if (Unitl::MatchMat(checkMat, handleMat, 0.99).x == -1)
		//{
		//	break;
		//}		
	}
	if (tryTime == 0)
	{
		return OPERATOR_FAILED;
	}

	//疯道人的还没处理玩，还需额外加流程处理
	if (ett == ETTFengDaoRen)
	{
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		if (IsDuiHuaKuangExists())
		{
			CloseDuiHuaKuang();
		}
	}
	//战斗
	if (ett == ETTZhanDou || ett == ETTFengDaoRen)
	{
		s4_ret = DefaultFight();
	}
	//任务完成的对话框
	if (IsDuiHuaKuangExists(2000))
	{
		MouseMoveClickLeftTop();
		cv::Rect shotRect(272, 284, 120, 140);
		cv::Mat mat;
		if (GetShotArea(shotRect + m_OriginPoint, mat))
		{
			cv::Point maskClickP = Unitl::FindPixel(mat, 0, 253, 253, 4);
			if (maskClickP.x != -1)
			{
				s4_ret = RICHANGFINISHED;
			}
		}
	}
	CloseDuiHuaKuang();
	CloseRenWu();
DONE:
	return s4_ret;
}


ETakType SwClient::XiulianDoRenWuPre(cv::Mat taskMat, cv::Point taskPoint)
{
	ETakType ett = ETTDefault;
	BOOL isComplete = false;
	cv::Mat taskListMat, targetMat;
	cv::Point targetPoint;
	//判断任务列表是否有完成的bmp
	GetRenWuListMat(taskMat, taskPoint, taskListMat);
	if (Unitl::MatchMat(taskMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\wancheng.bmp", 0.98).x != -1)
	{
		isComplete = true;
	}
	targetPoint = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\zhandou.bmp", 0.98).x != -1)
	{
		ett = ETTZhanDou;
	}

	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\xunwu.bmp", 0.98).x != -1)
	{
		if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\3-5ling.bmp", 0.98).x != -1)
		{
			//购买3-5装备之灵
			if (HandleRenWuBuyGoodsInSWB() == OPERATOR_SUCCESS)
			{
				ett = ETTXunWu_JYZX;
			}
		}
		else {
			if (Unitl::MatchMat(targetMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\jiaoyizhongxin.bmp", 0.98).x != -1)
			{
				if (isComplete || HandleRenWuBuyGoodsInCashStore() == OPERATOR_SUCCESS)
				{
					ett = ETTXunWu_JSZX;
				}
			}
			else {
				if (isComplete || HandleRenWuBuyGoodsInDianPu(targetMat, targetPoint) == OPERATOR_SUCCESS)
				{
					ett = ETTXunWu_DP;
				}
			}
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\buzhuochongwu.bmp", 0.98).x != -1)
	{
		if (isComplete || HandleBuZhuoChongWu() == OPERATOR_SUCCESS)
		{
			ett = ETTBuZhuoChongWu;
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\zhaoren.bmp", 0.98).x != -1)
	{
		ett = ETTZhaoRen;
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\xiulian\\fengdaoren.bmp", 0.98).x != -1)
	{
		ett = ETTFengDaoRen;
	}
	return ett;
}

int SwClient::RiChangXiulian()
{
	cv::Mat targetMat;
	cv::Point targetPoint;
	int s4_ret(OPERATOR_SUCCESS);
	ETakType renwuType;
	int doCount = 0;
	int getRenWenTimes = 0;
	if (IsRiChangFinish(ERCXiuLian))
	{
		s4_ret = OPERATOR_SUCCESS;
		goto DONE;
	}
	Sleep(150);
	for (; doCount < 50; )
	{
		if (IsRiChangFinish(ERCXiuLian))
		{
			s4_ret = OPERATOR_SUCCESS;
			goto DONE;
		}
		cv::Mat taskMat;
		cv::Point taskPoint = ShotRenWuWithOpenUi(taskMat, ERCXiuLian);
		if (taskPoint.x == -1)
		{
			//去领取修炼任务?
			if (getRenWenTimes > 3)
			{
				s4_ret = OPERATOR_FAILED;
				goto DONE;
			}
			getRenWenTimes++;
			FlyWithFeiXingQi(EMLinXianZhen, cv::Point(256, 212));
			WalkWithOpenDiTu(EMLinXianZhen, DiTuLoc(52, 91));
			while (isWalking());
			Sleep(800);
			if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
			{
				CloseDuiHuaKuang();
				CloseDiTu();
				continue;
			}
			Sleep(800);
			if (!IsDuiHuaKuangExists())
			{
				CloseDiTu();
				continue;
			}
			CloseDuiHuaKuang();
			CloseDiTu();
			taskPoint = ShotRenWuWithOpenUi(taskMat, ERCXiuLian);
			if (taskPoint.x == -1)
			{
				continue;
			}
		}
		targetPoint = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		if (targetPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		CloseRenWu();
		//做任务
		renwuType = XiulianDoRenWuPre(taskMat, taskPoint);
		if (renwuType == ETTDefault)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		//回复任务
		s4_ret = XiulianTiJiaoRenWu(renwuType, taskMat, taskPoint);
		if (s4_ret == RICHANGFINISHED)
		{
			s4_ret = OPERATOR_SUCCESS;
			goto DONE;
		}
		else if (s4_ret != OPERATOR_SUCCESS)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
	}
DONE:
	return s4_ret;
}


BOOL SwClient::isWalking(int checkTime)
{
	cv::Mat ltZuoBiao[2];
	cv::Rect shotRect(56, 28, 82, 12);
	GetShotArea(shotRect + m_OriginPoint, ltZuoBiao[0]);
	cv::inRange(ltZuoBiao[0], cv::Scalar(251, 251, 251), cv::Scalar(254, 254, 254), ltZuoBiao[0]);
	int dataSize = ltZuoBiao[0].cols*ltZuoBiao[0].rows*ltZuoBiao[0].channels();
	Sleep(checkTime);
	GetShotArea(shotRect + m_OriginPoint, ltZuoBiao[1]);
	cv::inRange(ltZuoBiao[1], cv::Scalar(251, 251, 251), cv::Scalar(254, 254, 254), ltZuoBiao[1]);
	if (memcmp(ltZuoBiao[0].data, ltZuoBiao[1].data, dataSize) == 0)
	{
		return false;
	}
	return true;
}

int SwClient::GetNormalZhuangTaiMat(cv::Mat& ztMat)
{
	cv::Rect shotRect(663, 55, 135, 60);
	if (GetShotArea(shotRect + m_OriginPoint, ztMat))
	{
		return OPERATOR_SUCCESS;
	}
	return OPERATOR_FAILED;
}


cv::Point SwClient::FindGuaiWuTargetInScene()
{
	cv::Mat shotMat, binMat, dst;
	cv::Rect shotRect(0, 0, 800, 600);
	cv::Point targetP(-1, -1), mchPoint;
	cv::Mat structureElement;
	cv::Mat mchMat(cv::Size(50, 10), CV_8UC1, cv::Scalar(255));
	int s = 3 * 2 + 1;
	//MouseMoveAbs(518, 28);
	//GetShotArea(shotRect + m_OriginPoint, shotMat);
	//cv::inRange(shotMat, cv::Scalar(0, 0, 254), cv::Scalar(0, 0, 254), binMat);

	////创建结构元，结构元形状，结构元Size，结构元锚点[参考点(-1, -1)代表中心像素为锚点]
	//structureElement = cv::getStructuringElement(cv::MORPH_RECT,
	//	cv::Size(s * 2, s), cv::Point(-1, -1));
	////调用膨胀API src,dst,结构元，(-1,-1)中心，膨胀次数
	//cv::dilate(binMat, binMat, structureElement, cv::Point(-1, -1), 1);

	////cv::imshow("targetgrayMat", grayMat(cv::Rect(380,70,28,36)));
	////cv::imwrite(Unitl::GetAppPathS() + SWPICTUREDIR + "\\zhuogui\\scenetarget.bmp",
	////	grayMat(cv::Rect(380, 70, 28, 36)));
	//mchPoint = Unitl::MatchMat(binMat, mchMat, 0.98);
	//if (mchPoint.x != -1) {
	//	mchPoint += shotRect.tl() + cv::Point(mchMat.cols / 2 + s * 2, mchMat.rows / 2 + s);
	//	//MouseMoveAbs(mchPoint);
	//	//MouseLeftBtnClick();
	//}
	shotRect = cv::Rect(0, 0, 800, 600);
	GetShotArea(shotRect + m_OriginPoint, shotMat);
	cv::inRange(shotMat, cv::Scalar(51, 215, 234), cv::Scalar(51, 215, 234), binMat);
	s = 3 * 5 + 1;
	structureElement = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(s * 2, s), cv::Point(-1, -1));
	//调用膨胀API src,dst,结构元，(-1,-1)中心，膨胀次数
	cv::dilate(binMat, binMat, structureElement, cv::Point(-1, -1), 1);
	mchMat = cv::Mat(cv::Size(130, 8), CV_8UC1, cv::Scalar(255));
	mchPoint = Unitl::MatchMat(binMat, mchMat, 0.999);
	cv::imshow("binMat", binMat);
	cv::waitKey(23232233);
	if (mchPoint.x != -1) {
		targetP = mchPoint + shotRect.tl() + cv::Point(mchMat.cols / 2, -30);
	}
	/*cv::imshow("targetgrayMat", binMat);
	cv::waitKey(232323);*/
	return targetP;
}


cv::Point SwClient::GetFuJinNPCMat(cv::Mat& retMat)
{
	int tryTime = 3;
	cv::Point retPoint(-1, -1);
	for (; tryTime > 0; tryTime--)
	{
		cv::Point hyPoint, markPoint;
		cv::Mat shotMat;
		if (OpenUi(EUiMark::EUMHaoYou, hyPoint) != OPERATOR_SUCCESS)
		{
			return hyPoint;
		}
		cv::Rect shotRect(260, 0, 550, 600);
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{

			markPoint = Unitl::MatchMat(shotMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\liaotian.bmp", 0.98);
			if (markPoint.x != -1)
			{
				markPoint += shotRect.tl();
				markPoint += cv::Point(3, 3);
				Sleep(100);
				MouseMoveRBClick(markPoint);
				Sleep(300);
				CloseUi(EUiMark::EUMHaoYou);
				tryTime++;
				continue;
			}
			markPoint = Unitl::MatchMat(shotMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\zhouweiliebiaojihuo.bmp", 0.98);
			if (markPoint.x == -1)
			{
				markPoint = Unitl::MatchMat(shotMat,
					Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\zhouweiliebiaoweijihuo.bmp", 0.98);
				if (markPoint.x == -1)
				{
					continue;
				}
				markPoint += shotRect.tl();
				markPoint += cv::Point(3, 3);
				Sleep(100);
				MouseMoveLBClick(markPoint);
				Sleep(300);
				CloseUi(EUiMark::EUMHaoYou);
			}
			else
			{
				markPoint = Unitl::MatchMat(shotMat,
					Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\zhouweinpc.bmp", 0.99);
				if (markPoint.x == -1)
				{
					markPoint = hyPoint;
					markPoint += cv::Point(29, 115);
					Sleep(100);
					MouseMoveLBClick(markPoint);
					Sleep(300);
					continue;
				}
				markPoint += cv::Point(-5, 15);
				retMat = shotMat(cv::Rect(markPoint.x, markPoint.y, shotRect.width - markPoint.x,
					shotRect.height - markPoint.y));
				retPoint = markPoint + shotRect.tl();
				break;
			}
		}
	}
	if (tryTime == 0)
	{
		return cv::Point(-1, -1);
	}
	return retPoint;
}


int SwClient::RiChangZhuoGui()
{
	int s4_ret(OPERATOR_SUCCESS);
	bool isQianNian = false;
	while (IsFighting());
	Sleep(500);
	MouseMoveLBClick(CLIENTWIDTH / 2, CLIENTHEIGHT / 2);
	for (; ;)
	{
		cv::Mat taskMat, targetMat;
		cv::Point taskPoint, targetPoint;
		cv::Point flyPoint(271, 209);
		EMap targetMap;
		DiTuLoc targetZB;
		int wt = 20000;
		PingBiRenWu();
		if (!isQianNian)
		{
			if (FlyWithFeiXingQi(EMChangAnCheng, flyPoint) != OPERATOR_SUCCESS)
			{
				continue;
			}
			Sleep(500);
			/*WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(162, 225));
			for (; wt > 0; )
			{
				if (!isWalking(500))
				{
					break;
				}
				wt -= 500;
			}
			if (wt == 0)
			{
				continue;
			}*/
			MouseMoveLBClick(276, 211);
			MouseMoveLBClick(276, 211);
			if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
			{
				continue;
			}
			CloseDuiHuaKuang();
			CloseDiTu();
		}
		/*if (IsDuiHuaKuangExists())
		{
			MouseMoveLBClick(CLIENTWIDTH / 2, CLIENTHEIGHT / 2);
		}
		Sleep(500);*/
		taskPoint = ShotRenWuWithOpenUi(taskMat, ERCZhuoGui);
		CloseRenWu();
		if (taskPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		targetPoint = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		targetMap = RenWuGetTargetPosition(targetMat);
		targetZB = FindGuaiWuTargetInRenWu(taskMat, taskPoint);
		//任务目标是否在城市中
		if (targetMap >= EMChangAnCheng && targetMap <= EMNvErGuo)
		{
			//cv::Point flyPoint(zBiao.x,zBiao.y);
			//FlyWithFeiXingQi(targetMap, flyPoint);
			cv::Point point = TranformDiTuLocToPoint(targetMap, targetZB);
			if (FlyWithFeiXingQi(targetMap, point, true) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(targetMap);
			}
		}
		else {
			if (GotoExceptCity(targetMap) != OPERATOR_SUCCESS)
			{
				continue;
			}
		}
		PingBiRenWu();
		Sleep(500);
		//怪物在实际地图的位置
		//targetPoint = FindGuaiWuTargetInScene();
		targetPoint = cv::Point(-1, -1);
		if (targetPoint.x != -1)
		{
			Sleep(100);
			targetPoint += cv::Point(8, 0);
			MouseMoveLBClick(targetPoint);
		}
		else
		{
			WalkWithOpenDiTu(targetMap, targetZB);
			Sleep(100);
			CloseDiTu();
			wt = 60000;
			for (; wt > 0; )
			{
				/*if (!isWalking(500))
				{
					break;
				}*/

				cv::Mat npcListMat;
				cv::Point targetInUiPoint, npcListPoint;
				npcListPoint = GetFuJinNPCMat(npcListMat);
				if (npcListPoint.x != -1)
				{
					if (isQianNian)
					{
						targetInUiPoint = Unitl::MatchMat(npcListMat,
							Unitl::GetAppPathS() + SWPICTUREDIR + "\\zhuogui\\zhouweinpcqiannianlaoyao.bmp", 0.98);
					}
					else
					{
						targetInUiPoint = Unitl::MatchMat(npcListMat,
							Unitl::GetAppPathS() + SWPICTUREDIR + "\\zhuogui\\zhouweinpcgui.bmp", 0.98);
					}
					if (targetInUiPoint.x != -1)
					{
						targetInUiPoint += npcListPoint;
						targetInUiPoint += cv::Point(0, 5);
						MouseMoveLBClick(targetInUiPoint);
						Sleep(100);
						MouseMoveLBClick(targetInUiPoint);
						Sleep(100);
						MouseMoveLBClick(targetInUiPoint);
						Sleep(100);
						MouseMoveLBClick(targetInUiPoint);
						Sleep(100);
						CloseUi(EUMHaoYou);
						break;
					}
				}
				wt -= 500;
				Sleep(500);
				//CloseUi(EUMHaoYou);				
			}
			if (wt == 0)
			{
				continue;
			}
			/*PingBiRenWu();
			Sleep(500);*/
			if (isQianNian)
			{
				/*isQianNian = false;
				MouseMoveLBClick(cv::Point(CLIENTWIDTH / 2, CLIENTHEIGHT / 2));*/
			}
			else
			{
				/*targetPoint = FindGuaiWuTargetInScene();
				if (targetPoint.x != -1)
				{
					Sleep(100);
					targetPoint += cv::Point(8, -8);
					MouseMoveLBClick(targetPoint);
				}*/

				/*cv::Mat npcListMat;
				cv::Point targetInUiPoint, npcListPoint;
				npcListPoint = GetFuJinNPCMat(npcListMat);
				if (npcListPoint.x != -1)
				{
					targetInUiPoint = Unitl::MatchMat(npcListMat,
						Unitl::GetAppPathS() + SWPICTUREDIR + "\\zhuogui\\zhouweinpcgui.bmp", 0.98);
					if (targetInUiPoint.x != -1)
					{
						targetInUiPoint += npcListPoint;
						targetInUiPoint += cv::Point(0, 5);
						MouseMoveLBClick(targetInUiPoint);
						CloseUi(EUMHaoYou);
					}
				}
				else
				{
					continue;
				}*/

			}
		}
		if (!IsFighting(8000))
		{
			goto DONE;
		}
		int flaghtTimeOut = 300000;
		bool huifuFlag = false;
		Sleep(1000);
		int huiheCount = 0;
		for (; flaghtTimeOut > 0; flaghtTimeOut -= 100)
		{
			if (!IsFighting())
			{
				isQianNian = false;
				//千年老妖
				if (ChoiceDuiHuaKuangFirstOption() == OPERATOR_SUCCESS)
				{
					isQianNian = true;
				}
				else
				{
					Sleep(300);
					MouseMoveLBClick(CLIENTWIDTH / 2, CLIENTHEIGHT / 2);
					Sleep(100);
				}
				break;
			}
			else
			{
				if (!huifuFlag)
				{
					huifuFlag = true;
					ZhuangTaiHuiFu();
				}
			}
			Sleep(100);
		}
		if (flaghtTimeOut <= 0)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		//bool debugPause = true;
	}
DONE:
	return s4_ret;
}

BOOL SwClient::PingBiRenWu()
{
	int tryTimes = -1;
	int tryMaxTime = 10000;
	cv::Mat shotMat;
	BOOL b_ret = false;
	DWORD startTime = GetTickCount();
	cv::Point markPoint(-1, -1);
	int keyBtnRecyclTime = 600;
	int a = -1;
	MouseMoveClickLeftTop();
	for (; ;)
	{
		a++;
		cv::Rect shotRect(55, 55, 120, 8);
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			cv::inRange(shotMat, cv::Scalar(254, 254, 254), cv::Scalar(254, 254, 254), shotMat);
			markPoint = Unitl::MatchMat(shotMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\pingbirenwu.bmp", 0.98);
			/*if (a == 1)
			{
				cv::imshow("shotMat", shotMat);
				cv::imshow("rawMat", cv::imread(Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark\\pingbirenwu.bmp"));
				cv::waitKey(555555);
			}*/
			if (markPoint.x != -1)
			{
				b_ret = true;
				Sleep(100);
				goto DONE;
			}
		}
		else {
			b_ret = false;
			goto DONE;
		}
		DWORD useTime = GetTickCount() - startTime;
		if (useTime > tryMaxTime)
		{
			b_ret = false;
			goto DONE;
		}
		else
		{
			int timeTemp = useTime / keyBtnRecyclTime;	//表达式直接放进if比较错误，原因未知
			int useStartTime = GetTickCount();
			if (timeTemp > tryTimes)
			{
				tryTimes = timeTemp;
				KeyBtnClick(KEYBTN_F9);
			}
			//按键用的时间不算入检测时长
			startTime = startTime - (GetTickCount() - useStartTime);
		}
		Sleep(250);
	}
DONE:
	return b_ret;
}

int SwClient::RiChangQingLong()
{

	return OPERATOR_SUCCESS;
}


int SwClient::RiChangXuanWu()
{
	for (;;)
	{
		//KeyBtnClick(KEYBTN_F4);
		MouseMoveLBClick(523, 248);
		MouseMoveLBClick(523, 248);
		Sleep(300);
		MouseMoveLBClick(307, 354);
		Sleep(400);
		MouseMoveLBClick(384, 160);
		MouseMoveLBClick(384, 160);
		Sleep(400);
		MouseMoveLBClick(318, 370);
		CloseDuiHuaKuang();
		cv::Mat taskMat, targetMat;
		cv::Point taskPoint, taskTargetP;
		taskPoint = ShotRenWuWithOpenUi(taskMat, ERCDefault);
		CloseRenWu();
		taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\bangpai\\xunlianshouhushou.bmp", 0.98).x != -1)
		{
			MouseMoveLBClick(384, 160);
			MouseMoveLBClick(384, 160);
			Sleep(400);
			MouseMoveLBClick(316, 385);
			Sleep(400);

			MouseMoveLBClick(523, 248);
			MouseMoveLBClick(523, 248);
			Sleep(300);
			MouseMoveLBClick(462, 351);
			Sleep(400);

			MouseMoveLBClick(382, 225);
			MouseMoveLBClick(382, 225);
			Sleep(300);
			int subTryTime;
			bool huifuFlag = false;
			for (subTryTime = 240; subTryTime > 0; subTryTime--)
			{
				int fightRet = WaitForNextHuiHe();
				if (fightRet == FIGHTOVER)
				{
					CloseDuiHuaKuang();
					break;
				}
				else if (fightRet == WAITFIGHTCOMMAND)
				{
					KeyBtnClickWithAlt('Q');
					KeyBtnClickWithAlt('Q');
					KeyBtnClickWithAlt('Q');
					if (!huifuFlag)
					{
						ZhuangTaiHuiFu();
						huifuFlag = true;
					}
				}
				else {
					break;
				}
				Sleep(100);
			}
			if (subTryTime == 0)
			{
				continue;
			}
			MouseMoveLBClick(382, 225);
			MouseMoveLBClick(382, 225);
			Sleep(300);
			MouseMoveLBClick(551, 350);
			MouseMoveLBClick(551, 350);
			Sleep(300);
		}
		else
		{
			bool debugPause = true;
		}

	}



	return OPERATOR_SUCCESS;
}


int SwClient::HandleBaoTuStep1(cv::Mat taskMat, cv::Point taskPoint)
{
	int tryTime = 3;
	int s4_ret(OPERATOR_FAILED);
	cv::Mat targetMat;
	cv::Point taskTargetP;
	taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\jiaoxunelang.bmp", 0.98).x == -1 &&
		Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\zhaohuiaichong.bmp", .98).x == -1)
	{
		return OPERATOR_FAILED;
	}
	cv::Point maskClickP = Unitl::FindPixelOnTail(targetMat, 0, 253, 253, 2);
	maskClickP += taskTargetP;
	maskClickP += cv::Point(-3, -3);
	int subTime = 3;
	BOOL huifuFlag = false;
	EMap position = RenWuGetTargetPosition(targetMat);
	for (tryTime; tryTime > 0; tryTime--)
	{
		if (GotoExceptCity(position) != OPERATOR_SUCCESS)
		{
			continue;
		}
		else {
			break;
		}
	}
	if (tryTime == 0)
	{
		s4_ret = OPERATOR_FAILED;
		goto DONE;
	}
	for (subTime = 3; subTime > 0; subTime--)
	{
		OpenRenWu();
		MouseMoveLBClick(maskClickP);
		CloseRenWu();
		Sleep(300);
		if (isWalking())
		{
			while (isWalking());
			break;
		}
		else
		{
			if (IsFighting())
			{
				break;
			}
			else
			{
				continue;
			}
		}
	}
	if (subTime == 0)
	{
		s4_ret = OPERATOR_FAILED;
		goto DONE;
	}

	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\jiaoxunelang.bmp", 0.98).x != -1)
	{
		for (subTime = 240; subTime > 0; subTime--)
		{
			int fightRet = WaitForNextHuiHe();
			if (fightRet == FIGHTOVER)
			{
				s4_ret = OPERATOR_SUCCESS;
				goto DONE;
			}
			else if (fightRet == WAITFIGHTCOMMAND)
			{
				KeyBtnClickWithAlt('Q');
				KeyBtnClickWithAlt('Q');
				KeyBtnClickWithAlt('Q');
				if (!huifuFlag)
				{
					ZhuangTaiHuiFu();
					huifuFlag = true;
				}
			}
			else {
				s4_ret = OPERATOR_FAILED;
				break;
			}
			Sleep(DELAYMORETIMR + 100);
		}
		if (subTime == 0)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\zhaohuiaichong.bmp", .98).x != -1)
	{
		cv::Mat shotMat;
		cv::Rect shotRect(0, 0, 500, 300);
		CloseDuiHuaKuang();
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			for (subTime = 240; subTime > 0; subTime--)
			{
				int fightRet = WaitForNextHuiHe();
				if (fightRet == FIGHTOVER)
				{
					s4_ret = OPERATOR_SUCCESS;
					goto DONE;
				}
				else {
					cv::inRange(shotMat, cv::Scalar(200, 252, 248), cv::Scalar(200, 252, 248), shotMat);
					cv::Point mchPoint = Unitl::MatchMat(shotMat,
						Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\mappaokuang.bmp", 0.999);
					if (mchPoint.x != -1)
					{
						mchPoint += cv::Point(12, 55);
						MouseMoveLBClick(mchPoint);
						MouseMoveLBClick(mchPoint);
						MouseMoveLBClick(mchPoint);
						MouseMoveLBClick(mchPoint);
					}
				}
			}
		}
	}
	else
	{
		int other;
	}
DONE:
	if (s4_ret == OPERATOR_SUCCESS)
	{
		CloseDuiHuaKuang();
	}
	return s4_ret;
}

int SwClient::HandleBaoTuBaoZangLieRen(cv::Mat taskMat, cv::Point taskPoint)
{

	int tryTime = 3;
	int s4_ret(OPERATOR_SUCCESS);
	cv::Mat gonglvMat, targetMat;
	cv::Point gonglvPoint, taskTargetP;
	EMap position;
	DiTuLoc targetZB;
	gonglvPoint = GetRenWuHuiFuMat(taskMat, taskPoint, gonglvMat);
	if (gonglvPoint.x == -1)
	{
		s4_ret = OPERATOR_FAILED;
		goto DONE;
	}
	position = RenWuGetTargetPosition(gonglvMat);
	if (position == EMPDefaule)
	{
		s4_ret = OPERATOR_FAILED;
		goto DONE;
	}
	targetZB = FindGuaiWuTargetInRenWu(taskMat, taskPoint);
	for (tryTime; tryTime > 0; tryTime--)
	{
		if (tryTime != 3)
		{
			CloseDuiHuaKuang();
			CloseRenWu();
		}
		//任务目标是否在城市中
		if (position >= EMChangAnCheng && position <= EMNvErGuo)
		{
			cv::Point point = TranformDiTuLocToPoint(position, targetZB);
			if (FlyWithFeiXingQi(position, point, true) != OPERATOR_SUCCESS)
			{

				FlyWithFeiXingQi(position);
			}
		}
		else {
			if (GotoExceptCity(position) != OPERATOR_SUCCESS)
			{
				continue;
			}
		}
		taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		cv::Point maskClickP = Unitl::FindPixelOnTail(targetMat, 0, 253, 253, 2);
		maskClickP += taskTargetP;
		maskClickP += cv::Point(-3, -3);
		int subTime = 3;
		for (subTime = 3; subTime > 0; subTime--)
		{
			OpenRenWu();
			MouseMoveLBClick(maskClickP);
			CloseRenWu();
			Sleep(200);
			if (isWalking())
			{
				while (isWalking());
				break;
			}
			else {
				if (IsDuiHuaKuangExists(100))
				{
					break;
				}
				else {
					continue;
				}
			}
		}
		if (subTime == 0)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		CloseDuiHuaKuang();
		s4_ret = OPERATOR_SUCCESS;
		goto DONE;
	}
DONE:
	return s4_ret;
}


int SwClient::HandleBaoTuBaoDaDangJia(cv::Mat taskMat, cv::Point taskPoint)
{

	int s4_ret(OPERATOR_SUCCESS);
	cv::Mat targetMat, huifuMat;
	cv::Point taskTargetP;
	EMap position;
	DiTuLoc targetZB;
	int tryTime = 3;
	taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
	if (taskTargetP.x == -1)
	{
		return OPERATOR_FAILED;
	}
	if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\yiwuhuiyou.bmp", 0.98).x != -1)
	{
		int subTime = 3;
		BOOL huifuFlag = false;
		position = RenWuGetTargetPosition(targetMat);
		targetZB = FindGuaiWuTargetInRenWu(taskMat, taskPoint);

		//任务目标是否在城市中
		if (position >= EMChangAnCheng && position <= EMNvErGuo)
		{
			cv::Point point = TranformDiTuLocToPoint(position, targetZB);
			if (FlyWithFeiXingQi(position, point, true) != OPERATOR_SUCCESS)
			{
				FlyWithFeiXingQi(position);
			}
		}
		else {
			if (GotoExceptCity(position) != OPERATOR_SUCCESS)
			{
				//continue;
			}
		}
		cv::Point maskClickP = Unitl::FindPixelOnTail(targetMat, 0, 253, 253, 2);
		if (maskClickP.x == 1)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		maskClickP += taskTargetP;
		maskClickP -= cv::Point(3, 3);
		for (subTime = 3; subTime > 0; subTime--)
		{
			if (subTime != 3)
			{
				CloseDuiHuaKuang();
				CloseRenWu();
			}
			OpenRenWu();
			MouseMoveLBClick(maskClickP);
			CloseRenWu();
			Sleep(500);
			if (isWalking())
			{
				while (isWalking());
			}
			if (IsFighting())
			{
				break;
			}
		}
		if (subTime == 0)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		if (IsFighting())
		{
			Sleep(100);
			for (subTime = 240; subTime > 0; subTime--)
			{
				int fightRet = WaitForNextHuiHe();
				if (fightRet == FIGHTOVER)
				{
					s4_ret = OPERATOR_SUCCESS;
					Sleep(500);
					goto DONE;
				}
				else if (fightRet == WAITFIGHTCOMMAND)
				{
					KeyBtnClickWithAlt('Q');
					KeyBtnClickWithAlt('Q');
					KeyBtnClickWithAlt('Q');
					if (!huifuFlag)
					{
						ZhuangTaiHuiFu();
						huifuFlag = true;
					}
				}
				else
				{
					s4_ret = OPERATOR_FAILED;
					goto DONE;
				}
				Sleep(DELAYMORETIMR + 300);
			}
			if (subTime == 0)
			{
				s4_ret = OPERATOR_FAILED;
				goto DONE;
			}
			s4_ret = OPERATOR_FAILED;
		}
		else
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
	}
	else if (Unitl::MatchMat(targetMat,
		Unitl::GetAppPathS() + SWPICTUREDIR + "\\baotu\\lishangwanglai.bmp", 0.98).x != -1)
	{
		//for package is fill;
		s4_ret = OPERATOR_FAILED;
		return s4_ret;

		if (HandleRenWuBuyGoodsInCashStore() != OPERATOR_SUCCESS)
		{
			s4_ret = OPERATOR_FAILED;
			return s4_ret;
		}
		int subTime = 3;
		BOOL huifuFlag = false;
		taskTargetP = GetRenWuHuiFuMat(taskMat, taskPoint, huifuMat);
		position = RenWuGetTargetPosition(huifuMat);

		//任务目标是否在城市中
		if (position >= EMChangAnCheng && position <= EMNvErGuo)
		{
			FlyWithFeiXingQi(position);
		}
		else {
			if (GotoExceptCity(position) != OPERATOR_SUCCESS)
			{
				//continue;
			}
		}
		cv::Point maskClickP = Unitl::FindPixelOnTail(huifuMat, 0, 253, 253, 2);
		if (maskClickP.x == 1)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		maskClickP += taskTargetP;
		maskClickP -= cv::Point(3, 3);
		for (subTime = 3; subTime > 0; subTime--)
		{
			OpenRenWu();
			MouseMoveLBClick(maskClickP);
			CloseRenWu();
			Sleep(500);
			if (isWalking())
			{
				while (isWalking());
			}
			if (IsDuiHuaKuangExists(1000))
			{
				break;
			}
		}
		if (subTime == 0)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		if (IsDuiHuaKuangExists(1000))
		{
			ChoiceDuiHuaKuangFirstOption();
			CloseDuiHuaKuang();
			Sleep(500);

		}
		s4_ret = OPERATOR_SUCCESS;
	}
	else
	{
		int other = 1;
		s4_ret = OPERATOR_FAILED;
		goto DONE;
	}
DONE:
	CloseDuiHuaKuang();//关闭烦人的对话框
	return s4_ret;
}

int SwClient::RiChangBaoTu()
{
	cv::Mat taskMat, targetMat;
	cv::Point taskPoint, taskTargetP;
	int s4_ret(OPERATOR_SUCCESS);
	int tryTime = 3;
	int max = 99;
	for (;;)
	{
		if (IsRiChangFinish(ERCBaoTu) || max <= 0)
		{
			s4_ret = OPERATOR_SUCCESS;
			goto DONE;
		}
		max--;
		if (FlyWithFeiXingQi(EMChangAnCheng, cv::Point(442, 367)) != OPERATOR_SUCCESS)
		{
			FlyWithFeiXingQi(EMChangAnCheng);
		}
		WalkWithOpenDiTu(EMChangAnCheng, DiTuLoc(293, 96));
		while (isWalking());
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
		{
			continue;
		}
		CloseDuiHuaKuang();//关闭烦人的对话框
		CloseDiTu();
		taskPoint = ShotRenWuWithOpenUi(taskMat, ERCBaoTu);
		CloseRenWu();
		if (taskPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			break;
		}
		taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		if (HandleBaoTuStep1(taskMat, taskPoint) != OPERATOR_SUCCESS)
		{
			//test
			s4_ret = OPERATOR_FAILED;
			//break;
		}
		taskPoint = ShotRenWuWithOpenUi(taskMat, ERCBaoTu);
		CloseRenWu();
		if (taskPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			break;
		}
		taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		//找宝藏猎人
		if (HandleBaoTuBaoZangLieRen(taskMat, taskPoint) != OPERATOR_SUCCESS)
		{
			//test
			s4_ret = OPERATOR_FAILED;
			//break;
		}
		taskPoint = ShotRenWuWithOpenUi(taskMat, ERCBaoTu);
		CloseRenWu();
		if (taskPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			break;
		}
		taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
		//大当家，类型有战斗、收集物资
		if (HandleBaoTuBaoDaDangJia(taskMat, taskPoint) != OPERATOR_SUCCESS)
		{
			s4_ret = OPERATOR_FAILED;
			break;
		}
	}
DONE:
	return s4_ret;

}

//非自动幻境，普通挂机
int SwClient::HuanJing()
{
	bool huifuFlag = false;
	cv::Mat shotMat;
	cv::Rect shotRect(260, 70, 300, 300);
	srand(GetTickCount());
	for (;;)
	{
		if (IsFighting())
		{
			if (!huifuFlag)
			{
				ZhuangTaiHuiFu();
				ClientGetFous();
				huifuFlag = true;
			}
			SetCommandAutoFight();
		}
		else {
			Sleep(500);
			huifuFlag = false;
			if (GetShotArea(shotRect + m_OriginPoint, shotMat))
			{
				cv::Point mchPoint = Unitl::MatchMat(shotMat,
					Unitl::GetAppPathS() + SWPICTUREDIR + "\\huanjing\\touzi.bmp", 0.98);
				if (mchPoint.x != -1)
				{
					Sleep(rand() % 3000);
					mchPoint += shotRect.tl();
					MouseMoveLBClick(mchPoint);
					Sleep(200);
					ClientGetFous();
					Sleep(5000);
					mchPoint += cv::Point(-20, 0);
					MouseMoveRBClick(mchPoint);
				}
			}
		}
		Sleep(800);
	}
}

/*
*	centerPoint相对于客户端原点的偏移值
*/
int SwClient::FlyWithFeiXingQi(EMap city, cv::Point centerPoint, bool isUiOpened)
{
	int tryTime = 3;
	int tryMax = 3;
	cv::Point mchPoint(-1, -1);
	cv::Point flyPoint(-1, -1);
	cv::Mat mat;
	cv::Rect shotRect(344, 7, 130, 50);
	MouseMoveClickLeftTop();
	for (; tryTime > 0; tryTime--)
	{
		OpenUi(EUMFeiXingQi, mchPoint);
		KeyBtnClick(0x30 + city);
		Sleep(200);
		flyPoint = FindFeiXingQiFlagNearPoint(centerPoint);
		if (flyPoint.x == -1)
		{
			return OPERATOR_FAILED;
		}
		MouseMoveLBClick(flyPoint);
		Sleep(200);//等待ui过渡
		//飞行完毕，Ui会消失
		if (GetShotArea(shotRect + m_OriginPoint, mat))
		{
			mchPoint = Unitl::MatchMat(mat,
				m_MatMMap["UiMark"][EUMFeiXingQi], 0.98);
			if (mchPoint.x == -1)
			{
				Sleep(DELAYMORETIMR + 800);//等待ui过渡
				return OPERATOR_SUCCESS;
			}
		}
	}
	if (tryTime == 0)
	{
		return OPERATOR_FAILED;
	}
	return OPERATOR_SUCCESS;
}


/*
*	按一次，50ms间隔检查，检查大概耗时80ms;
*/
int SwClient::OpenUi(EUiMark um, cv::Point& markPoint)
{
	int tryTimes = -1;
	int tryMaxTime = 10000;
	cv::Mat shotMat;
	int s4_ret(OPERATOR_SUCCESS);
	DWORD startTime = GetTickCount();
	markPoint = cv::Point(-1, -1);
	int keyBtnRecyclTime = 600;
	MouseMoveClickLeftTop();
	for (; ;)
	{
		cv::Rect shotRect(0, 0, 474, 300);
		if (um == EUiMark::EUMHaoYou)
		{
			shotRect = cv::Rect(530, 500, 270, 100);
		}
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			markPoint = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][um], 0.98);
			if (markPoint.x != -1)
			{
				markPoint += shotRect.tl();
				s4_ret = OPERATOR_SUCCESS;
				goto DONE;
			}
		}
		else {
			s4_ret = SHOT_ERROR;
			goto DONE;
		}
		DWORD useTime = GetTickCount() - startTime;
		if (useTime > tryMaxTime)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		else
		{
			int timeTemp = useTime / keyBtnRecyclTime;	//表达式直接放进if比较错误，原因未知
			int useStartTime = GetTickCount();
			if (timeTemp > tryTimes)
			{
				tryTimes = timeTemp;
				switch (um)
				{
				case EUMDiTu:
					KeyBtnClick(0x09);
					break;
				case EUMJiShouZhongXin:
					keyBtnRecyclTime = DELAYMORETIMR + 800;
					KeyBtnClickWithAlt('J');
					break;
				case EUMJiaoYiZhongXin:
					keyBtnRecyclTime = DELAYMORETIMR + 800;
					KeyBtnClickWithAlt('O');
					break;
				case EUMRiCheng:
					KeyBtnClickWithAlt('Y');
					break;
				case EUMRenWu:
					KeyBtnClickWithAlt('Q');
					break;
				case EUMWuPin:
					KeyBtnClickWithAlt('E');
					break;
				case EUMFeiXingQi:
					KeyBtnClick(KEYBTN_F2);
					break;
				case EUMHaoYou:
					KeyBtnClickWithAlt('F');
				default:
					break;
				}
			}
			//按键用的时间不算入检测时长
			startTime = startTime - (GetTickCount() - useStartTime);
		}
		Sleep(30);
	}
DONE:
	return s4_ret;
}


/*
*	按一次，50ms间隔检查，检查大概耗时80ms;右键单击关闭UI！！！
*/
int SwClient::CloseUi(EUiMark um, bool useKey)
{
	int tryTimes = -1;
	int tryMaxTime = 3000;
	cv::Mat shotMat;
	int s4_ret(OPERATOR_SUCCESS);
	MouseMoveClickLeftTop();
	DWORD startTime = GetTickCount();
	cv::Point markPoint = cv::Point(-1, -1);
	for (; ;)
	{
		cv::Rect shotRect(0, 0, 474, 300);
		if (um == EUiMark::EUMHaoYou)
		{
			shotRect = cv::Rect(530, 30, 270, 200);
		}
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			markPoint = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][um], 0.98);
			if (markPoint.x == -1)
			{
				s4_ret = OPERATOR_SUCCESS;
				goto DONE;
			}
		}
		else {
			s4_ret = SHOT_ERROR;
			goto DONE;
		}
		DWORD useTime = GetTickCount() - startTime;
		if (useTime > tryMaxTime)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		else
		{
			int timeTemp = useTime / 350;	//表达式直接放进if比较错误，原因未知
			int useStartTime = GetTickCount();
			if (timeTemp > tryTimes)
			{
				tryTimes = timeTemp;
				if (useKey)
				{
					switch (um)
					{
					case EUMDiTu:
						KeyBtnClick(0x09);
						break;
					case EUMJiShouZhongXin:
						KeyBtnClickWithAlt('J');
						break;
					case EUMJiaoYiZhongXin:
						KeyBtnClickWithAlt('O');
						break;
					case EUMRiCheng:
						KeyBtnClickWithAlt('Y');
						break;
					case EUMRenWu:
						KeyBtnClickWithAlt('Q');
						break;
					case EUMWuPin:
						KeyBtnClickWithAlt('E');
						break;
					case EUMHaoYou:
						KeyBtnClickWithAlt('F');
						break;
					case EUMFeiXingQi:
						KeyBtnClick(KEYBTN_F2);
						break;
					default:
						break;
					}
				}
				else
				{
					markPoint += shotRect.tl();
					if (um == EUMDiTu)
					{
						markPoint += cv::Point(-8, 24);
						MouseMoveRBClick(markPoint);
					}
					else {
						MouseMoveRBClick(markPoint);
					}
					MouseMoveClickLeftTop();
				}
			}
			//按键用的时间不算入检测时长
			startTime = startTime - (GetTickCount() - useStartTime);
		}
		Sleep(30);
	}
DONE:
	return s4_ret;
}

/*
*	打开飞行棋界面，把地图上的坐标转换为客户端屏幕坐标,误差±2
*/
cv::Point SwClient::TranformDiTuLocToPoint(EMap city, DiTuLoc zuobiao, bool isUiOpened)
{
	int tryTime = 3;
	int tryMax = 3;
	int s4_ret(OPERATOR_SUCCESS);
	cv::Point mchPoint(-1, -1);
	cv::Point retPoint(-1, -1);
	cv::Mat shotMat;
	MouseMoveClickLeftTop();
	ClientGetFous();
	for (; tryTime > 0; tryTime--)
	{
		LOG_MSG(DINFO, "Start.");
		OpenUi(EUMFeiXingQi, mchPoint);
		if (mchPoint.x == -1)
		{
			continue;
		}
		cv::Rect shotRect(0, 0, 132, 64);
		shotRect += mchPoint;
		shotRect += cv::Point(250, 452);
		if (!GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			goto DONE;
		}
		mchPoint = Unitl::MatchMat(shotMat, m_MatMMap["UiMark"][EUMQFeiXingQiDiTu], 0.98);
		if (mchPoint.x != -1)
		{
			mchPoint += shotRect.tl();
			mchPoint += cv::Point(22, -4);
			int width = 2 * mchPoint.x - CLIENTWIDTH;
			int height = CLIENTHEIGHT - (CLIENTHEIGHT - mchPoint.y) * 2;
			height -= 13;//飞行棋ui内的地图上下margin不均等，需补齐
			if (SwComm::DiTuZuoBiaoMax.find(city) == SwComm::DiTuZuoBiaoMax.end())
			{
				goto DONE;
			}
			double widthPixelScale = SwComm::DiTuZuoBiaoMax[city].x / (double)width;
			double heightPixelScale = SwComm::DiTuZuoBiaoMax[city].y / (double)height;
			retPoint = mchPoint + cv::Point(-width, 0);	///地图上的（0,0）
			retPoint.x += (double)zuobiao.x / widthPixelScale;
			retPoint.y -= (double)zuobiao.y / heightPixelScale;
			/*MouseMoveAbs((int)retPoint.x, (int)retPoint.y);
			Sleep(10232320);*/
			break;
		}
		else {
			goto DONE;
		}
	}
DONE:
	return retPoint;
}

int SwClient::FlyWithFeiXingQi(EMap city, bool isUiOpened)
{
	cv::Point flyPoint(-1, -1);
	int tryTime = 3;
	int tryMax = 3;
	cv::Mat mat;
	cv::Rect shotRect(344, 7, 130, 50);
	cv::Point mchPoint(-1, -1);
	switch (city)
	{
	case EMChangAnCheng:
		flyPoint = SwComm::CADefaultFlyPoint;
		break;
	case EMAoLaiGuo:
		flyPoint = SwComm::ALDefaultFlyPoint;
		break;
	case EMQingHeZhen:
		flyPoint = SwComm::QHDefaultFlyPoint;
		break;
	case EMLinXianZhen:
		flyPoint = SwComm::LXDefaultFlyPoint;
		break;
	case EMNvErGuo:
		flyPoint = SwComm::NEDefaultFlyPoint;
		break;
	default:
		break;
	}
	if (flyPoint.x == -1)
	{
		return OPERATOR_FAILED;
	}
	for (; tryTime > 0; tryTime--)
	{
		if (tryTime != tryMax)
		{
			if (GetShotArea(shotRect + m_OriginPoint, mat))
			{
				mchPoint = Unitl::MatchMat(mat,
					m_MatMMap["UiMark"][EUMFeiXingQi], 0.98);
				if (mchPoint.x == -1)
				{
					KeyBtnClick(KEYBTN_F2);
					Sleep(DELAYMORETIMR + 200);
				}
				isUiOpened = true;
			}
		}
		//第一次不检查飞行棋ui
		if (!isUiOpened)
		{
			KeyBtnClick(KEYBTN_F2);
			Sleep(DELAYMORETIMR + 200);
			KeyBtnClick(0x30 + city);
			Sleep(200);
		}
		MouseMoveLBClick(flyPoint);
		Sleep(300);//等待ui过渡
		//飞行完毕，Ui会消失
		if (GetShotArea(shotRect + m_OriginPoint, mat))
		{
			mchPoint = Unitl::MatchMat(mat,
				m_MatMMap["UiMark"][EUMFeiXingQi], 0.98);
			if (mchPoint.x == -1)
			{
				Sleep(DELAYMORETIMR + 800);//等待ui过渡
				return OPERATOR_SUCCESS;
			}
		}
	}
	if (tryTime == 0)
	{
		return OPERATOR_FAILED;
	}
	return OPERATOR_SUCCESS;
}




int SwClient::RiChangShiMen()
{
	int s4_ret(OPERATOR_SUCCESS);
	cv::Mat taskMat, targetMat;
	cv::Point taskPoint, taskTargetP;
	int startTickCount;
	if (IsRiChangFinish(ERCShiMen))
	{
		goto DONE;
	}
	RiChangShiMenSeeMenPaiMaster();
	startTickCount = GetTickCount();
	for (;;)
	{
		if (ChoiceDuiHuaKuangFirstOption() != OPERATOR_SUCCESS)
		{
			s4_ret = OPERATOR_SUCCESS;
			CloseDuiHuaKuang(3000);
			goto DONE;
		};
		CloseDuiHuaKuang(3000);
		//30分钟内未完成20师门，返回失败
		if (GetTickCount() - startTickCount > 1800000)
		{
			s4_ret = OPERATOR_FAILED;
			goto DONE;
		}
		Sleep(100);
		taskPoint = ShotRenWuWithOpenUi(taskMat, ERCShiMen);
		if (taskPoint.x == -1)
		{
			s4_ret = OPERATOR_FAILED;
			break;
		}
		taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);

		if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\menpaixunluo.jpg", 0.98).x != -1)
		{
			KeyBtnClick(KEYBTN_F1);
			Sleep(250);
			//传的是任务目标Mat
			HandleMenPaiXunLuo(targetMat, taskTargetP);
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\buzhuochongwu.jpg", 0.98).x != -1)
		{
			CloseRenWu();
			HandleBuZhuoChongWu();
			if (RiChangShiMenSeeMenPaiMaster() != OPERATOR_SUCCESS)
			{
				return OPERATOR_FAILED;
			}
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\shoujiwuzi.jpg", 0.98).x != -1)
		{
			//传的是任务Mat
			ShiMenHandleShouJiWuZi(taskMat, taskPoint);
		}
		else if (Unitl::MatchMat(targetMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\shimen\\tiaozhan.bmp", 0.98).x != -1)
		{
			//传的是任务Mat
			ShiMenHandleTiaoZhan(targetMat, taskTargetP);
		}
		//ChoiceDialogFirstOption();
	}
DONE:
	CloseRenWu();
	return OPERATOR_SUCCESS;
}


/*
*	以长安图大小检测，需要先打开飞行棋？？？？
*/
cv::Point SwClient::FindHuiFuTargetInDiTu(EMap city)
{
	cv::Mat dituGrayMat;
	cv::Rect shotRect(73, 114, 685, 385);
	cv::Point dituTargetP(-1, -1);
	if (city<EMChangAnCheng || city>EMNvErGuo)
	{
		return dituTargetP;
	}
	KeyBtnClick(KEYBTN_F2);
	Sleep(DELAYMORETIMR + 200);
	KeyBtnClick(0x30 + city);
	Sleep(350);
	GetShotArea(shotRect + m_OriginPoint, dituGrayMat);
	cv::inRange(dituGrayMat, cv::Scalar(0, 200, 200), cv::Scalar(0, 255, 255), dituGrayMat);
	dituTargetP = Unitl::MatchMat(dituGrayMat,
		"D:\\Project\\SWTest\\SwPorject\\Debug\\bmpSource\\xiulian\\ditumubiaodian.bmp", 0.95);
	if (dituTargetP.x != -1) {
		dituTargetP += cv::Point(73, 114);
	}
	return dituTargetP;
}

void FindGuaiWuTargetInDiTuSaveNumber(cv::Mat numBinMat)
{
	cv::inRange(numBinMat, cv::Scalar(0, 0, 50), cv::Scalar(255, 255, 255), numBinMat);
	for (int len = 0; len < numBinMat.cols;)
	{
		int cutStartCol = -1, cutEndCol = -1;
		for (int i = len; i < numBinMat.cols; i++)
		{
			int j = 0;
			for (; j < numBinMat.rows; j++)
			{
				//存在0黑色，i=截取的x坐标
				if (numBinMat.data[j*numBinMat.cols + i] == 0)
				{
					cutStartCol = i;
					break;
				}
			}
			if (j != numBinMat.rows)
			{
				break;
			}
		}

		if (cutStartCol == -1)break;
		if (cutStartCol != -1)
		{
			for (int i = cutStartCol + 1; i < numBinMat.cols; i++)
			{
				int j = 0;
				for (; j < numBinMat.rows; j++)
				{
					//有黑色，跳出
					if (numBinMat.data[j*numBinMat.cols + i] == 0)
					{
						break;
					}
				}
				//全为白，寻找结束
				if (j == numBinMat.rows)
				{
					cutEndCol = i;
					break;
				}
			}
		}
		if (cutEndCol == -1)break;
		int cutLength = cutEndCol - cutStartCol;
		if (cutLength != 0)
		{
			//保存图片
			cv::Rect cutRect(cutStartCol, 0, cutLength, numBinMat.rows);
			static int saveMarkdsdsdasd = 0;
			std::string sFileName = Unitl::GetAppPathS() + SWPICTUREDIR + "\\tasknumber\\"
				+ std::to_string(GetTickCount() + saveMarkdsdsdasd) + ".bmp";
			cv::imwrite(sFileName, numBinMat(cutRect));
			saveMarkdsdsdasd++;
		}
		len = cutEndCol;
	}
	return;
}


DiTuLoc SwClient::GetMatDiTuZuoBiao(cv::Mat &zbMat)
{
	DiTuLoc zuoBiao(-1, -1);
	std::vector<std::pair<int, cv::Point>> numPoint;
	int pointIndex = -1;
	int frontX;
	int *val = NULL;
	int num = 0;
	int dBit = 0;
	cv::Point tempPoint(0, 0);
	if (zbMat.empty())
	{
		goto DONE;
	}
	cv::inRange(zbMat, cv::Scalar(0, 0, 50), cv::Scalar(255, 255, 255), zbMat);
	for (int i = 0; i < 11; i++)
	{
		std::vector<cv::Point> pointVecRet = Unitl::MatchMat_VecRet(zbMat,
			m_MatMMap["TaskNumber"][ETNum0 + i], 0.99);
		for (auto point : pointVecRet)
		{
			if (point.x != 0)
			{
				numPoint.push_back(std::pair<int, cv::Point>(i, point));
			}
		}
	}
	for (auto np : numPoint)
	{
		if (np.first == ETPoint)
		{
			pointIndex = np.first;
			break;
		}
	}
	if (numPoint.size() == 0 || pointIndex == -1)
	{
		//vec大小为0 或者 找不到逗号分隔符，失败
		goto DONE;
	}
	std::sort(numPoint.begin(), numPoint.end(), [](std::pair<int, cv::Point> &nplhs,
		std::pair<int, cv::Point> &nprhs) {
		return nplhs.second.x < nprhs.second.x;
	});
	val = &tempPoint.x;
	frontX = numPoint[0].second.x;
	if (numPoint.size() != 0)
	{
		for (auto np : numPoint)
		{
			//字符间隔超出20px，非坐标数字
			if (np.second.x - frontX > 20)
			{
				break;
			}
			frontX = np.second.x;
			if (np.first == ETPoint)
			{
				//是时候计算y坐标了
				dBit = 0;
				val = &tempPoint.y;
				continue;
			}
			else {

			}
			(*val) = (*val) * 10 + np.first;
			dBit++;
		}
	}
	if (tempPoint.x != 0 && tempPoint.y != 0)
	{
		zuoBiao = tempPoint;
	}
DONE:
	return zuoBiao;
}

/*
*	这里只是点击行走，是否行走到指定点，需要额外判断
*/
int SwClient::WalkWithOpenDiTu(EMap ditu, DiTuLoc targetPoint)
{
	//72,109
	cv::Point shijiePoint = OpenDiTu();
	//地图宽高(实际大小，并非地图坐标的值)
	int width = CLIENTWIDTH - shijiePoint.x * 2 + 2;
	int height = CLIENTHEIGHT - shijiePoint.y * 2 + 2;
	if (SwComm::DiTuZuoBiaoMax.find(ditu) == SwComm::DiTuZuoBiaoMax.end())
	{
		bool error = true;
		return OPERATOR_FAILED;
	}
	double widthPixelScale = SwComm::DiTuZuoBiaoMax[ditu].x / (double)width;
	double heightPixelScale = SwComm::DiTuZuoBiaoMax[ditu].y / (double)height;
	///地图上的（0,0）
	cv::Point_<float> dituOriginPoint = shijiePoint + cv::Point(0, height);
	//MouseMoveAbs((int)dituOriginPoint.x, (int)dituOriginPoint.y);
	//Sleep(10232320);
	dituOriginPoint.x += (double)targetPoint.x / widthPixelScale;
	dituOriginPoint.y -= (double)targetPoint.y / heightPixelScale;
	MouseMoveLBClick((int)dituOriginPoint.x, (int)dituOriginPoint.y);
	return OPERATOR_SUCCESS;
}
DiTuLoc SwClient::FindGuaiWuTargetInRenWu(cv::Mat& taskMat, cv::Point taskPoint)
{
	cv::Mat targetMat;
	GetRenWuTargetMat(taskMat, taskPoint, targetMat);
	cv::Point zuokuohao = Unitl::MatchMat(targetMat,
		"D:\\Project\\SWTest\\SwPorject\\Debug\\bmpSource\\tasknumber\\zuokuohao_caise.bmp", 0.99);
	cv::Rect zuobiaoRect(zuokuohao.x, zuokuohao.y,
		targetMat.cols - zuokuohao.x, 15);
	cv::Mat tempMat;
	if (zuokuohao.x != -1)
	{
		tempMat = targetMat(zuobiaoRect);
		//FindGuaiWuTargetInDiTuSaveNumber(tempMat);
		return GetMatDiTuZuoBiao(tempMat);
	}
	return DiTuLoc(-1, -1);
}

cv::Point SwClient::FindFeiXingQiFlagNearPoint(cv::Point point)
{
	cv::Point mchHongPoint(-1, -1);
	cv::Point mchLanPoint(-1, -1);
	cv::Mat mat;
	cv::Point centerPoint(150, 150);
	cv::Rect rect = cv::Rect(point.x - centerPoint.x, point.y - centerPoint.y, 300, 300);
	Sleep(100);//等待ui
	if (GetShotArea(rect + m_OriginPoint, mat)) {
		//匹配距离中心点最近的飞行棋
		mchHongPoint = Unitl::MatchMatWithCenter(mat, m_MatMMap["UiMark"][EUMFeiXingQiHongSe], 0.98,
			centerPoint);
		mchLanPoint = Unitl::MatchMatWithCenter(mat, m_MatMMap["UiMark"][EUMFeiXingQiLanSe], 0.98,
			centerPoint);
		if (mchHongPoint.x != -1 && mchLanPoint.x != -1)
		{
			cv::Point temp = mchHongPoint - centerPoint;
			int calcDistance = sqrt(temp.x* temp.x + temp.y* temp.y);
			temp = mchLanPoint - centerPoint;
			if (calcDistance < sqrt(temp.x* temp.x + temp.y* temp.y))
			{
				return mchHongPoint + rect.tl();
			}
			else {
				return mchLanPoint + rect.tl();
			}
		}
		if (mchHongPoint.x != -1)return mchHongPoint + rect.tl();
		if (mchLanPoint.x != -1)return mchLanPoint + rect.tl();
	}
	return cv::Point(-1, -1);
}

cv::Point SwClient::FindFeiXingQiFlagInAreaRect(cv::Rect shotRect)
{
	cv::Point mchPoint(-1, -1);
	cv::Mat mat;
	//Sleep(200);//等待ui
	if (GetShotArea(shotRect + m_OriginPoint, mat)) {
		//匹配距离中心点最近的飞行棋
		mchPoint = Unitl::MatchMatWithCenter(mat, m_MatMMap["UiMark"][EUMFeiXingQiHongSe], 0.98,
			cv::Point(shotRect.x / 2, shotRect.y / 2));
		/*cv::imshow("mat", mat);
		cv::waitKey(23232);*/
		if (mchPoint.x != -1)
		{
			mchPoint += shotRect.tl();
		}
	}
	return mchPoint;
}

cv::Mat src, dst;                   // 全局变量
int element_size = 1;      //全局变量
int max_size = 21;           // 全局变量


void CallBack_func(int, void*)
{
	int s = 3 * 2 + 1;
	//创建结构元，结构元形状，结构元Size，结构元锚点[参考点(-1, -1)代表中心像素为锚点]
	cv::Mat structureElement = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(s * 2, s), cv::Point(-1, -1));
	//调用膨胀API src,dst,结构元，(-1,-1)中心，膨胀次数
	cv::dilate(src, dst, structureElement, cv::Point(-1, -1), 1);
	//cv::cvtColor(dst, dst, cv::COLOR_RGB2BGR);
	//cv::imshow("膨胀操作后：", dst);
	int channel = dst.channels();
	cv::Mat temp;
	cv::inRange(dst, cv::Scalar(200, 200, 0), cv::Scalar(255, 255, 100), temp);
	//cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
	//cv::threshold(dst, dst, 200, 255, cv::THRESH_BINARY);
	cv::imshow("取区间值：", temp);

	int g_nThresh = 80;
	int g_nThresh_max = 255;
	cv::Mat g_cannyMat_output;
	std::vector<std::vector<cv::Point> > g_vContours;
	std::vector<cv::Vec4i> g_vHierarchy;

	cv::RNG g_rng(12345);

	cv::Canny(temp, g_cannyMat_output, g_nThresh, g_nThresh * 2, 3);
	//寻找轮廓
	findContours(g_cannyMat_output, g_vContours, g_vHierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	cv::Mat drawing = cv::Mat::zeros(g_cannyMat_output.size(), CV_8UC3);
	for (int i = 0; i < g_vContours.size(); i++)
	{
		cv::Scalar color = cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255));
		drawContours(drawing, g_vContours, i, color, 2, 8, g_vHierarchy, 0, cv::Point());
		double a = cv::contourArea(g_vContours[i]);
		LOG_MSG(DINFO, "area%d Count:%f", i, a);
	}
	imshow("轮廓检测", drawing);

}

cv::Point SwClient::Test_ChoiceOption()
{
	cv::Size ksize(13, 7);
	std::vector<std::vector<cv::Point> > g_vContours;
	std::vector<cv::Vec4i> g_vHierarchy;
	cv::Mat shotMat;
	int g_nThresh = 80;
	cv::Point firstOptPoint(-1, -1);
	cv::Mat g_cannyMat_output;
	cv::Rect shotRect(m_OriginPoint.x + 260, m_OriginPoint.y + 280, 400, 150);
	if (!GetShotArea(shotRect, shotMat))
	{
		return firstOptPoint;
	}
	//创建结构元，结构元形状，结构元Size，结构元锚点[参考点(-1, -1)代表中心像素为锚点]
	cv::Mat structureElement = cv::getStructuringElement(cv::MORPH_RECT,
		ksize, cv::Point(-1, -1));
	//调用膨胀API src,dst,结构元，(-1,-1)中心，膨胀次数
	cv::dilate(shotMat, dst, structureElement, cv::Point(-1, -1), 1);
	//取颜色在蓝色区间的点

	cv::inRange(dst, cv::Scalar(200, 200, 0), cv::Scalar(255, 255, 100), dst);
	cv::Canny(dst, g_cannyMat_output, g_nThresh, g_nThresh * 2, 3);

	cv::imshow("膨胀操作后：", dst);
	int chjdsad = dst.channels();
	int matchColCount = 0;
	int matchRowCount = 0;
	bool isMatch = false;
	int startMchCol = 0;
	int startMchRow = 0;
	//列扫描
	for (int i = 0; !isMatch && i < dst.cols; i++)
	{
		for (int j = 0; j < dst.rows; j++)
		{
			if (dst.data[(j*dst.cols + i)] == 0xff)
			{
				startMchCol = i + 3;
				startMchRow = j + 3;
				isMatch = true;
				break;
			}
		}
	}
	int notMchCount = 0;
	int i = startMchRow;
	for (; i < dst.rows; i++)
	{
		//比较列，计算高度
		if (dst.data[(i*dst.cols + startMchCol)] == 0xff)
		{
			notMchCount = 0;
		}
		else {
			notMchCount++;
		}
		if (notMchCount == 5)
		{
			break;
		}
	}
	int optHeight = i - startMchRow;

	////匹配成功
	//isMatch = true;
	//firstOptPoint = cv::Point(
	//	shotRect.x - m_OriginPoint.x + j,
	//	shotRect.y - m_OriginPoint.y + i);
	//LOG_MSG(DINFO, "find area,start point(%d,%d)",
	//	firstOptPoint.x, firstOptPoint.y);
	return firstOptPoint;


}



void SwClient::Test_Opencv()
{
	cv::Size ksize(13, 7);
	std::vector<std::vector<cv::Point> > g_vContours;
	std::vector<cv::Vec4i> g_vHierarchy;
	cv::Mat shotMat;
	cv::Point firstOptPoint(-1, -1);
	cv::Mat g_cannyMat_output;
	//73,114,685,385
	cv::Rect shotRect(73, 114, 685, 385);
	GetShotArea(shotRect + m_OriginPoint, shotMat);
	cv::Mat grayMat;
	//cv::cvtColor(shotMat, grayMat,cv::COLOR_BGR2GRAY);
	cv::inRange(shotMat, cv::Scalar(0, 200, 200), cv::Scalar(0, 255, 255), grayMat);
	Unitl::MatchMat(grayMat,
		"D:\\Project\\SWTest\\SwPorject\\Debug\\bmpSource\\xiulian\\ditumubiaodian.bmp", 0.8);

	//cv::imshow("gray", grayMat);
	//cv::imwrite("D:\\Project\\SWTest\\SwPorject\\Debug\\bmpSource\\xiulian\\ditumubiaodian.bmp",
	//	grayMat(cv::Rect(238, 295, 18, 28)));
//MouseMoveAbs(maxLoc);
	cv::waitKey();
	int  a = 1;
}

void SwClient::CustomEvent1()
{
	cv::Point mchPoint(-1, -1);
	cv::Mat shotMat;
	cv::Rect shotRect(700, 0, 50, 50);

	//Sleep(200);//等待ui
	for (;;)
	{
		cv::Rect shotRect(50, 50, 500, 500);
		if (GetShotArea(shotRect + m_OriginPoint, shotMat)) {
			continue;
			//匹配距离中心点最近的飞行棋
			mchPoint = Unitl::MatchMat(shotMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\other\\activateEvent1.bmp", 0.999);
			if (mchPoint.x != -1)
			{
				//KeyBtnClick(KEYBTN_F4);
				MouseMoveLBClick(523, 248);
				MouseMoveLBClick(523, 248);
				Sleep(300);
				MouseMoveLBClick(307, 354);
				Sleep(400);
				MouseMoveLBClick(384, 160);
				MouseMoveLBClick(384, 160);
				Sleep(400);
				MouseMoveLBClick(318, 370);
				CloseDuiHuaKuang();
				cv::Mat taskMat, targetMat;
				cv::Point taskPoint, taskTargetP;
				taskPoint = ShotRenWuWithOpenUi(taskMat, ERCDefault);
				CloseRenWu();
				taskTargetP = GetRenWuTargetMat(taskMat, taskPoint, targetMat);
				if (Unitl::MatchMat(targetMat,
					Unitl::GetAppPathS() + SWPICTUREDIR + "\\bangpai\\xunlianshouhushou.bmp", 0.98).x != -1)
				{
					MouseMoveLBClick(384, 160);
					MouseMoveLBClick(384, 160);
					Sleep(400);
					MouseMoveLBClick(316, 385);
					Sleep(400);

					MouseMoveLBClick(523, 248);
					MouseMoveLBClick(523, 248);
					Sleep(300);
					MouseMoveLBClick(462, 351);
					Sleep(400);

					MouseMoveLBClick(382, 225);
					MouseMoveLBClick(382, 225);
					Sleep(300);
					int subTryTime;
					bool huifuFlag = false;
					for (subTryTime = 240; subTryTime > 0; subTryTime--)
					{
						int fightRet = WaitForNextHuiHe();
						if (fightRet == FIGHTOVER)
						{
							CloseDuiHuaKuang();
							break;
						}
						else if (fightRet == WAITFIGHTCOMMAND)
						{
							KeyBtnClickWithAlt('Q');
							KeyBtnClickWithAlt('Q');
							KeyBtnClickWithAlt('Q');
							if (!huifuFlag)
							{
								ZhuangTaiHuiFu();
								huifuFlag = true;
							}
						}
						else {
							break;
						}
						Sleep(100);
					}
					if (subTryTime == 0)
					{
						continue;
					}
					MouseMoveLBClick(382, 225);
					MouseMoveLBClick(382, 225);
					Sleep(300);
					MouseMoveLBClick(551, 350);
					MouseMoveLBClick(551, 350);
					Sleep(300);
				}
				else
				{
					bool debugPause = true;
				}
			}
		}
		else
		{
			bool debugPause = true;
		}

	}
}

void SwClient::QiangMie()
{
	while (true)
	{
		KeyBtnClickWithAlt('A');
	}

}
void SwClient::CustomAction()
{
	cv::Point originPoint[3];
	Start(0, 0, 800, 1080);
	originPoint[0] = m_OriginPoint;
	Start(800, 0, 800, 1080);
	originPoint[1] = m_OriginPoint;
	Start(1920 - 800, 0, 800, 1080);
	originPoint[2] = m_OriginPoint;
	for (int i = 0; true; i++)
	{
		m_OriginPoint = originPoint[i % 3];
		ClientGetFous();
		Sleep(500);
		ZhuangTaiHuiFu();
		Sleep(500);
		KeyBtnClickWithCtrl('A');
		Sleep(5000);
	}
}
void SwClient::FuBen()
{

	cv::Point startPoint[3] =
	{
		{0,0},{800,0},{1600,0}
	};
	cv::Point originPoint[3];
	for (int i = 0; i < 3; i++)
	{
		if (Start(startPoint[i].x, startPoint[i].y, 800, 1080) != OPERATOR_SUCCESS)
		{
			originPoint[i] = cv::Point(-1, -1);
		}
		else
		{
			originPoint[i] = m_OriginPoint;
		}
	}
	for (int i = 0; true; i++)
	{
		m_OriginPoint = originPoint[i % 3];
		if (m_OriginPoint.x == -1)
		{
			continue;
		}
		ClientGetFous();
		if (i % 30 > 25 && i % 30 < 29)
		{
			Sleep(200);
			ZhuangTaiHuiFu();
		}
		if (i % 50 > 45 && i % 50 < 49)
		{
			Sleep(200);
			KeyBtnClickWithCtrl('A');
		}
		cv::Point mchPoint;
		ShotSwClient();
		mchPoint = Unitl::MatchMat(m_ShotMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\fuben\\queding.bmp", 0.98);
		if (mchPoint.x != -1)
		{
			Sleep(200);
			MouseMoveLBClick(mchPoint);
		}
		else
		{
			mchPoint = Unitl::MatchMat(m_ShotMat,
				Unitl::GetAppPathS() + SWPICTUREDIR + "\\fuben\\tongyi.bmp", 0.98);
			if (mchPoint.x != -1)
			{
				Sleep(200);
				MouseMoveLBClick(mchPoint);
			}
		}
		mchPoint = Unitl::MatchMat(m_ShotMat,
			Unitl::GetAppPathS() + SWPICTUREDIR + "\\fuben\\baoxiang.bmp", 0.98);
		if (mchPoint.x != -1)
		{
			Sleep(200);
			MouseMoveLBClick(CLIENTWIDTH / 2, CLIENTHEIGHT / 2);
			Sleep(500);
			MouseMoveRBClick(CLIENTWIDTH / 2, CLIENTHEIGHT / 2 - 120);
		}
		Sleep(1000);
	}
}

void SwClient::YuanGuYaoGuai()

{
	while (true)
	{
		cv::Rect shotRect(203, 389, 70, 70);
		cv::Mat shotMat;
		if (GetShotArea(shotRect + m_OriginPoint, shotMat))
		{
			cv::Point mchClickP = Unitl::MatchMat(shotMat,
				m_MatMMap["UiMark"][EUMDuiHuaKuang], 0.98);
			if (mchClickP.x != -1)
			{
				cv::Rect shotRect(272, 284, 120, 140);
				cv::Mat mat;
				Sleep(100);
				if (GetShotArea(shotRect + m_OriginPoint, mat))
				{

					cv::Point maskClickP = Unitl::FindPixel(mat, 0, 253, 253, 4);
					if (maskClickP.x != -1)
					{
						for (int i = 0; i < 10; i++)
						{
							MouseMoveAbsLBClickQuickly(shotRect.x + maskClickP.x,
								shotRect.y + maskClickP.y);
						}
					}
				}
				Sleep(1000);
				CloseDuiHuaKuang();
			}
		}
		Sleep(50);
	}
}

int SwClient::DoRiChangMultiClient(bool needShutDown)
{
	int s4_ret(OPERATOR_SUCCESS);
	bool rePlaceWindow = true;
	int checkClientMax = 1;
	if (needShutDown)
	{
		system("shutdown -a");
		system("shutdown -s -t 14800");
		//system("shutdown -s -t 3600");
	}
	Sleep(1000);

	cv::Point originPoint[3];
	cv::Point placeMidPoint(5, 850);
	int foundClientCount = 0;
	for (int i = 0; i < checkClientMax; i++)
	{
		if (Start(0, 0, 4095, 2048) != OPERATOR_SUCCESS)
		{
			originPoint[i] = cv::Point(-1, -1);
			continue;
		}
		MouseOperator srcMo(MoveAbs, m_OriginPoint.x + 50, m_OriginPoint.y - 12);
		MouseOperator midMo(MoveAbs, placeMidPoint.x, placeMidPoint.y);
		Sleep(100);
		int s4_ret = SendMouseAction(srcMo);
		Sleep(100);
		s4_ret += MouseLeftBtnDown();
		Sleep(100);
		s4_ret = SendMouseAction(midMo);
		Sleep(100);
		s4_ret += MouseLeftBtnUp();
		m_OriginPoint += midMo.GetCvPoint() - srcMo.GetCvPoint();
		originPoint[i] = m_OriginPoint;

		foundClientCount++;
		placeMidPoint.y += 30;
	}

	placeMidPoint.y -= foundClientCount * 30;
	cv::Point placeDstPoint(55, 3 * 80);

	for (int i = 0; i < foundClientCount; i++)
	{
		MouseOperator srcMo(MoveAbs, placeMidPoint.x, placeMidPoint.y);
		MouseOperator dstMo(MoveAbs, placeDstPoint.x, placeDstPoint.y);

		Sleep(100);
		int s4_ret = SendMouseAction(srcMo);
		Sleep(100);
		s4_ret += MouseLeftBtnDown();
		Sleep(100);
		s4_ret = SendMouseAction(dstMo);
		Sleep(100);
		s4_ret += MouseLeftBtnUp();
		originPoint[i] += dstMo.GetCvPoint() - srcMo.GetCvPoint();

		placeDstPoint.x += 25;
		placeDstPoint.y -= 80;
		//偏移
		placeMidPoint.y += 30;
		LOG_MSG(DINFO, "DoRiChangMultiClient OriginPoint(%d,%d)", originPoint[i].x, originPoint[i].y);
	}

	for (int i = 0; i < foundClientCount; i++)
	{
		//重新设置原点坐标
		m_OriginPoint = originPoint[i];
		ClientGetFous();
		Sleep(500);
		if (Start(m_OriginPoint.x - 25, m_OriginPoint.y - 25, CLIENTWIDTH + 50, CLIENTHEIGHT + 50) != OPERATOR_SUCCESS)
		{
			LOG_MSG(DINFO, "OriginPoint ReSet Error");
			continue;
		}
		originPoint[i] = m_OriginPoint;
	}

	for (int i = 0; i < foundClientCount; i++)
	{
		/*m_OriginPoint = originPoint[i];
		ClientGetFous();
		Sleep(500);
		ShotSwClient(cv::Rect(0, 0, 800, 600));
		cv::imshow("", m_ShotMat);
		cv::waitKey(222222);
		Sleep(500);*/
	}
	DWORD startTime = GetTickCount();
	DWORD tempTime, cycleStartTime;
	DWORD useSec = 0;
	int tryTime = 3;

	for (; tryTime > 0; tryTime--)
	{
		cycleStartTime = GetTickCount();
		for (int i = 0; i < foundClientCount; i++)
		{
			if (i == 0)
			{
				m_CashGoodsPricesMax = 500000;
			}
			else {
				m_CashGoodsPricesMax = 200000;
			}
			m_OriginPoint = originPoint[i];
			if (m_OriginPoint.x == -1)
			{
				continue;
			}
			Sleep(1000);
			ClientGetFous();
			Sleep(100);
			ClientGetFous();
			tempTime = GetTickCount();

			CloseDuiHuaKuang();
			CloseDiTu();
			CloseRenWu();
			CloseRiChang();
			tempTime = GetTickCount();
			s4_ret += RiChangShiMen();
			useSec = (GetTickCount() - tempTime) / 1000;
			LOG_MSG(DINFO, "DoRiChangMultiClient RiChangShiMen,Use:%dh%dm", useSec / 3600, (useSec - (useSec / 3600) * 3600) / 60);


			CloseDuiHuaKuang();
			CloseDiTu();
			CloseRenWu();
			CloseRiChang();
			tempTime = GetTickCount();
			s4_ret += RiChangLingHu();

			CloseDuiHuaKuang();
			CloseDiTu();
			CloseRenWu();
			CloseRiChang();
			tempTime = GetTickCount();
			s4_ret += RiChangXiulian();
			useSec = (GetTickCount() - tempTime) / 1000;
			LOG_MSG(DINFO, "DoRiChangMultiClient RiChangXiulian,Use:%dh%dm", useSec / 3600, (useSec - (useSec / 3600) * 3600) / 60);
		}
		useSec = (GetTickCount() - cycleStartTime) / 1000;
		LOG_MSG(DINFO, "DoRiChangMultiClient One Cycle,Use:%dh%dm", useSec / 3600, (useSec - (useSec / 3600) * 3600) / 60);
	}

	/*for (int i = 0; i < foundClientCount; i++)
	{
		m_OriginPoint = originPoint[i % 3];
		if (m_OriginPoint.x == -1)
		{
			continue;
		}
		CloseDuiHuaKuang();
		CloseDiTu();
		CloseRenWu();
		CloseRiChang();
		tempTime = GetTickCount();
		s4_ret += RiChangBaoTu();
		useSec = (GetTickCount() - tempTime) / 1000;
		LOG_MSG(DINFO, "DoRiChangMultiClient RiChangBaoTu,Use:%dh%dm", useSec / 3600, (useSec - (useSec / 3600) * 3600) / 60);
	}
	*/

	useSec = (GetTickCount() - startTime) / 1000;
	LOG_MSG(DINFO, "DoRiChangMultiClient Over,Total Use:%dh%dm", useSec / 3600, (useSec - (useSec / 3600) * 3600) / 60);
	if (needShutDown)
	{
		system("shutdown -a");
		system("shutdown -s -t 30");
	}
	return OPERATOR_SUCCESS;
}

void SwClient::Test()
{
	cv::Mat tempMat, taskMat;
	cv::Point taskPoint, taskTargetP;
	int tryTime = 0;
	int subTryTime = 0;
	int s4_ret(OPERATOR_SUCCESS);
	//WalkWithOpenDiTu(EMChangAnCheng, cv::Point(454, 191));
	//cv::Point retPoint = FindGuaiWuTargetInRenWu();
	Sleep(DELAYMORETIMR + 100);
	ShotSwClient();
	//GetNormalZhuangTaiMat(sdsd);
	//OpencvTest();
	//MatchDiTu(EMap::LingHuLeYuan);

	//bool isFight = false;
	//RiChangXuanWu();
	//CustomAction();
	//YuanGuYaoGuai();

	//YuanGuYaoGuai();
	//s4_ret = RiChangZhuoGui();
	//YuanGuYaoGuai();
	//RiChangZhuoGui();
	//s4_ret = RiChangXiulian();
	//RiChangShiMen();
	//RiChangZhuoGui();
	//RiChangBaoTu();
	//PingBiRenWu();

	DoRiChangMultiClient(false);
	//RiChangXiulian();
	//FuBen();
	return;

	//CustomEvent1();
	s4_ret = RiChangShiMen();
	if (s4_ret != OPERATOR_SUCCESS)
	{
		LOG_MSG(DINFO, "RiChangShiMen Failed.");
	}
	s4_ret = RiChangXiulian();
	if (s4_ret != OPERATOR_SUCCESS)
	{
		LOG_MSG(DINFO, "RiChangXiulian Failed.");
	}

	s4_ret = RiChangLingHu();
	if (s4_ret != OPERATOR_SUCCESS)
	{
		LOG_MSG(DINFO, "RiChangLingHu Failed.");
	}
	DiTuLoc zbiao(498, 172);
	TranformDiTuLocToPoint(EMChangAnCheng, zbiao);


	taskPoint = OpenRenWu();
	if (taskPoint.x == -1)
	{
		return;
	}
	if (!ShotSwClient())
	{
		return;
	}
	if (tryTime > 3)
	{
		return;
	}

	cv::Rect rect(538, 156, 210, 30);
	m_ShotMat(rect).copyTo(tempMat);
	int channels = tempMat.channels();
	cv::imshow("cut", tempMat);
	cv::imwrite(Unitl::GetAppPathS() + SWPICTUREDIR + "\\wordTest.jpg", tempMat);
	//Unitl::OCRPixelByteToString("chi_sim",tempMat);
	//cv::imwrite(Unitl::GetAppPathS() + SWPICTUREDIR + "\\uiMark", tempMat);
	cv::waitKey(1024000);


	cv::imshow("second", m_ShotMat);
	cv::waitKey(1024000);

	return;
}

/*
*	需要 m_ShotMat 先赋值
*/
cv::Mat SwClient::GetGoodsIcon(cv::Mat& goodsPageMat, int row, int col)
{
	cv::Mat mat_ret;
	cv::Rect cutRect;
	cutRect.x = 0;
	cutRect.y = 0;
	cutRect.x += (SwComm::GoodsGridWidth + SwComm::GoodsGridColSpace)*col;
	cutRect.y += (SwComm::GoodsGridHeight + SwComm::GoodsMoneyGridRowSpace +
		SwComm::GoodsMoneyGridHeight + SwComm::GoodsGridRowSpace)*row;
	cutRect.height = SwComm::GoodsGridHeight;
	cutRect.width = SwComm::GoodsGridWidth;
	goodsPageMat(cutRect).copyTo(mat_ret);
DONE:
	return mat_ret;;
}


int SwClient::GetGoodsPrice(cv::Mat& goodsPageMat, int row, int col)
{
	int  price = INT_MAX;
	cv::Rect cutRect(0, 0, 0, 0);
	cv::Mat cutMat;
	cutRect.y += SwComm::GoodsGridHeight + SwComm::GoodsMoneyGridRowSpace;
	cutRect.x += (SwComm::GoodsMoneyGridWidth + SwComm::GoodsGridColSpace)*col;
	cutRect.y += (SwComm::GoodsMoneyGridHeight
		+ SwComm::GoodsGridRowSpace
		+ SwComm::GoodsGridHeight
		+ SwComm::GoodsMoneyGridRowSpace)*row;

	cutRect.height = SwComm::GoodsMoneyGridHeight;
	cutRect.width = SwComm::GoodsMoneyGridWidth;
	goodsPageMat(cutRect).copyTo(cutMat);
	std::string priceStr = Unitl::OCRPixelByteToString(SwComm::GetSwCashStorePriceUseFont(), cutMat);
	std::remove_if(priceStr.begin(), priceStr.end(),
		std::bind2nd(std::equal_to<char>(), 'k'));
	priceStr.erase(std::remove_if(priceStr.begin(), priceStr.end(), [](char ch) {
		return ch == ',' || ch == ' ' || ch == '\n';
	}), priceStr.end());
	if (!priceStr.empty()) {
		std::istringstream(priceStr) >> price;
	}
	return price;
}

void SwClient::GoodsPrePage()
{
	MouseMoveLBClick(m_cashStorePoint.x + 216, m_cashStorePoint.y + 360);
}

void SwClient::GoodsNextPage()
{
	MouseMoveLBClick(m_cashStorePoint.x + 383, m_cashStorePoint.y + 360);
}

void SwClient::BuyCashStoreGoods(int row, int col)
{
	if (m_cashStorePoint.x == -1)return;
	int x, y;
	x = m_cashStorePoint.x;
	y = m_cashStorePoint.y;
	x += 172;
	y -= 22;
	x += (SwComm::GoodsGridWidth + SwComm::GoodsGridColSpace)*col;
	y += (SwComm::GoodsGridHeight + SwComm::GoodsMoneyGridRowSpace +
		SwComm::GoodsMoneyGridHeight + SwComm::GoodsGridRowSpace)*row;
	x += SwComm::GoodsGridWidth / 2;
	y += SwComm::GoodsGridHeight / 2;
	MouseMoveLBClick(x, y);
	Sleep(200);
	KeyBtnClick(13);
	//MouseMoveAbs(m_cashStorePoint.x + 405, m_cashStorePoint.y + 410);
	//Sleep(DELAYMORETIMR+3);
	//MouseLeftBtnClick();
}

cv::Mat& SwClient::AddMaskForGoods(cv::Mat& src)
{
	int height = src.rows;
	int width = src.cols;
	double k = (double)height / width;
	int channels = src.channels();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (i < -k * j + height || i > -k * j + height + 13)
			{
				src.data[channels * (i*width + j)] = 0;
				src.data[channels * (i*width + j) + 1] = 0;
				src.data[channels * (i*width + j) + 2] = 0;
			}
		}
	}
	return src;
}

int SwClient::SaveGoodsIconsToBmp(std::string dirPath)
{
	cv::Size iSize;
	int s4_ret = 0;
	/////
	/*if (!ShotSwClient())
	{
		s4_ret = SHOT_ERROR;
		goto DONE;
	}
	for (int r = 0; r < 5; r++)
	{
		for (int c = 0; c < 5; c++)
		{
			cv::Mat matTemp = GetGoodsIcon(r, c);
			cv::Mat goodsMat;
			matTemp.copyTo(goodsMat);


			if (goodsMat.empty()) {
				s4_ret = NOT_MATCH;
				goto DONE;
			}
			std::string strFileName(dirPath);
			time_t nowTimeSec;
			tm *now_tm;
			char temp[MAX_PATH];
			time(&nowTimeSec);
			now_tm = localtime(&nowTimeSec);
			sprintf(temp, "\\%d%d%d%d%d-%d-%d.jpg", now_tm->tm_mon, now_tm->tm_mday, now_tm->tm_hour,
				now_tm->tm_min, now_tm->tm_sec, r, c);
			strFileName.append(temp);
			std::vector<std::string> fileNameVec;
			Unitl::GetDirFiles(dirPath.c_str(), fileNameVec);
			AddMaskForGoods(matTemp);
			bool isExists = false;
			for (std::string fname : fileNameVec)
			{
				cv::Mat mch = cv::imread(dirPath + "\\" + fname);

				if (mch.empty())continue;
				AddMaskForGoods(mch);
				if (Unitl::MatchMat(matTemp, mch, 0.9).x != -1)
				{
					isExists = true;
				}
			}
			if (isExists)continue;
			cv::imwrite(strFileName, goodsMat);
		}
	}*/
DONE:
	return s4_ret;
}

/*
*	根据 Mat 匹配查找价格最小的物品
*/
int SwClient::GetMinPriceGood(cv::Point jszxPoint, cv::Mat& src, int scanfPageSize, int& priceMin, int& page, int& row, int& col)
{
	CashStoreGoods goods[25];
	int priceMinPage = 0;
	int curPage = 1;
	int priceVec[255];
	int pageVec[255];
	int length = 0;
	priceMin = INT_MAX;
	for (int i = scanfPageSize; i > 0; i--)
	{
		cv::Mat pageMat;
		cv::Rect shotRect(0, 0, 260, 400);
		shotRect += jszxPoint;
		shotRect += cv::Point(172, -21);
		if (!GetShotArea(shotRect + m_OriginPoint, pageMat))
		{
			return SHOT_ERROR;
		}
		if (curPage == 10)
		{
			int a = 11;
		}
		if (GetPageGoodsInfo(pageMat, goods, src) == OPERATOR_SUCCESS)
		{
			for (int j = 0; j < 25; j++)
			{
				if (goods[j].GoodsEnum == ECSGTDefault) //Mat匹配成功项
				{
					if (goods[j].prices <= priceMin)
					{
						priceMin = goods[j].prices;
						priceMinPage = curPage;
						page = curPage;
						row = j / 5;
						col = j % 5;
					}
					priceVec[length] = goods[j].prices;
					pageVec[length] = curPage;
					length++;
				}
			}
		}
		else {
			LOG_MSG(DINFO, "GetPageGoodsInfo Error");
		}
		///
		if (i > 1) {
			GoodsNextPage();
		}
		Sleep(DELAYMORETIMR + 500);
		curPage++;
	}

	for (int i = 0; i < length; i++)
	{
		//LOG_MSG(DINFO, "price=%d,page=%d", priceVec[i], pageVec[i]);
	}
	//LOG_MSG(DINFO, "Min price=%d,page=%d", priceMin, priceMinPage);
	if (priceMin == INT_MAX)
	{
		return NOT_MATCH;
	}
	return OPERATOR_SUCCESS;
}

/*
*	根据指定的类型查找价格最小的物品
*/
int SwClient::GetMinPriceGood(ECashStoreGoodsType type, int subType, int scanfPageSize, int& page, int& row, int& col)
{
	/*CashStoreGoods goods[25];
	int priceMin = INT_MAX;
	int priceMinPage = 0;
	int curPage = 1;
	int priceVec[255];
	int pageVec[255];
	int length = 0;
	for (int i = scanfPageSize; i > 0; i--)
	{
		if (GetPageGoodsInfo(goods, type) == OPERATOR_SUCCESS)
		{
			for (int j = 0; j < 25; j++)
			{
				if (goods[j].GoodsEnum == subType)
				{
					if (goods[j].prices < priceMin)
					{
						priceMin = goods[j].prices;
						priceMinPage = curPage;
						page = curPage;
						row = j / 5;
						col = j % 5;
					}
					priceVec[length] = goods[j].prices;
					pageVec[length] = curPage;
					length++;
				}
			}
		}
		else {
			LOG_MSG(DINFO, "GetPageGoodsInfo Error");
		}
		///
		if (i > 1) {
			GoodsNextPage();
		}
		Sleep(DELAYMORETIMR + 400);
		curPage++;
	}
	for (int i = 0; i < length; i++)
	{
		LOG_MSG(DINFO, "price=%d,page=%d", priceVec[i], pageVec[i]);
	}
	LOG_MSG(DINFO, "Min price=%d,page=%d", priceMin, priceMinPage);*/
	return OPERATOR_SUCCESS;
}

void SwClient::SetCommandAutoFight()
{
	KeyBtnClickWithCtrl('A');
}


int SwClient::CustomFightCommand(int shortcutCode, cv::Point point)
{
	KeyBtnClick(shortcutCode);
	MouseMoveLBClick(point.x, point.y);
	KeyBtnClickWithAlt('Q');
	return OPERATOR_SUCCESS;
}

int SwClient::GetShotArea(cv::Rect rect, cv::Mat& mat_dest)
{
	while (pause)
	{
		Sleep(50);
	}
	return Unitl::GetShotArea(rect, mat_dest);
}

int SwClient::SendMouseAction(MouseOperator opt)
{
	return MyUsbMgr::GetInstance()->SendMouseOperator(opt);
}

int SwClient::SendKeyBoardAction(EKeyBoardEventOperator eop, int code)
{
	return MyUsbMgr::GetInstance()->SendKeyBoardOperator(eop, code);
}


/*
*	此处没有给usbMutex加锁
*/
void SwClient::GetFocusDelayLess()
{
	int x = 9, y = 42;
	UsbCommDuration(60);
	SendMouseAction(MouseOperator(MoveAbs, m_OriginPoint.x + x, m_OriginPoint.y + y));
	UsbCommDuration(60);
	SendMouseAction(MouseOperator(LeftBtn_Down));
	UsbCommDuration(60);
	SendMouseAction(MouseOperator(LeftBtn_Up));
}

int SwClient::KeyBtnDown(int code)
{
	UsbCommDuration(65);
	return SendKeyBoardAction(KeyDown, code);
}

int SwClient::KeyBtnUp(int code)
{
	UsbCommDuration(65);
	return SendKeyBoardAction(KeyUp, code);
}

int SwClient::MouseLeftBtnDown()
{
	UsbCommDuration(60);
	MouseOperator mo(LeftBtn_Down);
	return SendMouseAction(mo);
}

int SwClient::MouseLeftBtnUp()
{
	UsbCommDuration(60);
	MouseOperator mo(LeftBtn_Up);
	return SendMouseAction(mo);
}
int SwClient::MouseRightBtnDown()
{
	UsbCommDuration(65);
	MouseOperator mo(RightBtn_Down);
	return SendMouseAction(mo);
}
int SwClient::MouseRightBtnUp()
{
	UsbCommDuration(65);
	MouseOperator mo(RightBtn_Up);
	return SendMouseAction(mo);
}












int SwClient::KeyBtnClick(int code)
{
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
#ifdef MULTICLIENT
	GetFocusDelayLess();
#endif
	KeyBtnDown(code);
	int ret = KeyBtnUp(code);
	usbMutex.unlock();
	return ret;
	}

int SwClient::MouseLeftBtnClick()
{
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
#ifdef MULTICLIENT
	GetFocusDelayLess();
#endif
	MouseLeftBtnDown();
	int ret = MouseLeftBtnUp();
	usbMutex.unlock();
	return ret;
	}

int SwClient::MouseRightBtnClick()
{
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
#ifdef MULTICLIENT
	GetFocusDelayLess();
#endif
	MouseRightBtnDown();
	int ret = MouseRightBtnUp();
	usbMutex.unlock();
	return ret;
	}

int SwClient::MouseMoveAbsLBClickQuickly(int x, int y)
{
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
	MouseOperator mo(MoveAbs, m_OriginPoint.x + x, m_OriginPoint.y + y);
	int s4_ret = 0;
	s4_ret = SendMouseAction(mo);
	mo = MouseOperator(LeftBtn_Click);
	s4_ret += SendMouseAction(mo);
	Sleep(10);
	mo = MouseOperator(MoveAbs, m_OriginPoint.x + x, m_OriginPoint.y + y);
	s4_ret += SendMouseAction(mo);
	mo = MouseOperator(LeftBtn_Click);
	s4_ret += SendMouseAction(mo);
	usbMutex.unlock();
	return s4_ret == 0 ? 0 : 1;
}

int SwClient::MouseMoveAbsLBClick(int x, int y)
{
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
	UsbCommDuration(60);
	MouseOperator mo(MoveAbs, x, y);
	int s4_ret = SendMouseAction(mo);
	s4_ret += MouseLeftBtnDown();
	s4_ret += MouseLeftBtnUp();
	usbMutex.unlock();
	return s4_ret == 0 ? 0 : 1;
}


int SwClient::MouseMoveAbsRBClick(int x, int y)
{
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
	UsbCommDuration(65);
	MouseOperator mo(MoveAbs, x, y);
	int s4_ret = SendMouseAction(mo);
	s4_ret += MouseRightBtnDown();
	s4_ret += MouseRightBtnUp();
	usbMutex.unlock();
	return s4_ret == 0 ? 0 : 1;
}

/*
*	界面的左上角为原点
*/
int SwClient::MouseMoveLBClick(int x, int y)
{
	return MouseMoveAbsLBClick(m_OriginPoint.x + x, m_OriginPoint.y + y);
}

int SwClient::MouseMoveLBClick(cv::Point point)
{
	int ret = MouseMoveLBClick(point.x, point.y);
	return ret;
}



int SwClient::MouseMoveRBClick(int x, int y)
{
	return MouseMoveAbsRBClick(m_OriginPoint.x + x, m_OriginPoint.y + y);
}

int SwClient::MouseMoveRBClick(cv::Point point)
{
	int ret = MouseMoveRBClick(point.x, point.y);
	return ret;
}

int SwClient::KeyBtnClickWithAlt(int code)
{

	int s4_ret = 0;
	if (code < 'A' || code>'Z')return 1;
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
#ifdef MULTICLIENT
	GetFocusDelayLess();
#endif
	UsbCommDuration(20);
	s4_ret += SendKeyBoardAction(KeyDown, KP_ALT);
	Sleep(35);
	s4_ret += SendKeyBoardAction(KeyDown, code);
	Sleep(35);
	s4_ret += SendKeyBoardAction(KeyUp, code);
	Sleep(65);
	s4_ret += SendKeyBoardAction(KeyUp, KP_ALT);
	m_LastKeyEventTime = GetTickCount();
	usbMutex.unlock();
	return s4_ret == 0 ? 0 : 1;
	}

int SwClient::KeyBtnClickWithCtrl(int code)
{
	int s4_ret = 0;
	if (code < 'A' || code>'Z')return 1;
	while (pause)
	{
		Sleep(50);
	}
	usbMutex.lock();
#ifdef MULTICLIENT
	GetFocusDelayLess();
#endif
	UsbCommDuration(60);
	s4_ret += SendKeyBoardAction(KeyDown, KP_CTRL);
	Sleep(10);
	s4_ret += SendKeyBoardAction(KeyDown, code);
	Sleep(10);
	s4_ret += SendKeyBoardAction(KeyUp, code);
	Sleep(65);
	s4_ret += SendKeyBoardAction(KeyUp, KP_CTRL);
	m_LastKeyEventTime = GetTickCount();
	usbMutex.unlock();
	return s4_ret == 0 ? 0 : 1;
	}


void SwClient::UsbCommDuration(int delayMs)
{
	int duration = GetTickCount() - m_LastKeyEventTime;
	if (duration < delayMs)
	{
		Sleep(delayMs - duration);
	}
	m_LastKeyEventTime = GetTickCount();
}
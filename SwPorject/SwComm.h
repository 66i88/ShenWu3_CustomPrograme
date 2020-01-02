#pragma once
#include <iostream>
#include <map>

#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\opencv.hpp>
#include <opencv2\imgproc.hpp>

#define SWPICTUREDIR		"\\bmpSource\\"
#define CLIENTWIDTH			800
#define CLIENTHEIGHT		600

#define DELAYMORETIMR		100

enum ECharacterState {
	Free,
	Busy,
	Fighting,
};

enum EUiMark {
	EUMTaiYang,
	EUMYueLiang,
	EUMDiTu,
	EUMJiShouZhongXin,
	EUMJiaoYiZhongXin,
	EUMRiCheng,
	EUMRenWu,
	EUMWuPin,
	EUMHaoYou,
	EUMLingHuLeYuan_WanCheng,
	EUMLingHuLeYuan_WeiWanCheng,
	EUMShiMenRenWu_WanCheng,
	EUMShiMenRenWu_WeiWanCheng1,
	EUMShiMenRenWu_WeiWanCheng2,
	EUMBaoTuRenWu_WanCheng,
	EUMBaoTuRenWu_WeiWanCheng1,
	EUMBaoTuRenWu_WeiWanCheng2,
	EUMXiuLian_WanCheng,
	EUMXiuLian_WeiWanCheng,
	EUMZiDongXunLu,
	EUMXinYuGouMai,
	EUMXianJinGouMai,
	EUMFeiXingQi,
	EUMDuiHuaKuang,
	EUMFeiXingQiHongSe,
	EUMFeiXingQiLanSe,
	EUMQuMoXiang,
	EUMQFeiXingQiDiTu,

};

enum ETaskNum {
	ETNum0,
	ETNum1,
	ETNum2,
	ETNum3,
	ETNum4,
	ETNum5,
	ETNum6,
	ETNum7,
	ETNum8,
	ETNum9,
	ETPoint,
};

enum ERiChang {
	ERCDefault,
	ERCShiMen,
	ERCLingHuLeYuan,
	ERCXiuLian,
	ERCBaoTu,
	ERCZhuoGui,
};

enum EMap {
	EMDefault = -1,
	EMChangAnCheng = 0,
	EMAoLaiGuo,
	EMQingHeZhen,
	EMLinXianZhen,
	EMNvErGuo,
	EMLingXiaoDian,
	EMBangPai,
	EMLingHuLeYuan,
	EMDaTangGuoJing,
	EMJinYueYuan,
	EMJinLuanDian,
	EMKunLunShan,
	EMChangAnChengWai,
	EMQingHeZhenWai,
	EMDaTangJingWai,
	EMWuSiZang,
	EMHuaGuoShan,
	//ÃÅÅÉ
	EMFoMen,
	EMTianCe,
	EMQiXingFangCun,
	EMTianMoLi,

	EMLingXiaoTianGong,
	EMDongHaiLongGong,
	EMNanHaiPuTuo,
	EMZhenYuanWuZhuang,

	EMYouMingDiFu,
	EMLMoWangShan,
	EMWanShouLing,
	EMPanSiLing,

	EMWuMingGu,
};

enum ENormalStoreType {
	ENSTWuQiDian,
	ENSTYaoDian,
	ENSTFuShiDian
};

enum ETakType {
	ETTDefault,
	ETTZhaoRen,
	ETTBuZhuoChongWu,
	ETTXunWu_JYZX,
	ETTXunWu_DP,
	ETTXunWu_JSZX,
	ETTZhanDou,
	ETTFengDaoRen
};

enum ECashStoreGoodsType {
	ECSGTDefault,
	ECSGTFurniture,
	ECSGTCourtyardDecoration,
	ECSGTAntique,
	ECSGTCooking,
	ECSGTThirdDrugs,
	ECSGTSecondDrugs,
	ECSGTFood,
	ECSGTFlower,
	ECSGTUnSeeWeapon,
	ECSGTMonsterManual,
	ECSGTTransformationalCard,
	ECSGTEquipment
};
enum EMonster {
	Defaule,
	HuXiaoMeng
};
enum EMenPai {
	EMPDefaule,
	EMPWS,
	EMPMW,
	EMPDF,
	EMPPS,
	EMPLG,
	EMPPT,
	EMPTG,
	EMPWZ,
	EMPTC,
	EMPFM,
	EMPTM,
	EMPFC
};

enum ECashStoreCooking {
	DouF,
	JiaoHJ,
	BaBZ,
	ZhenZWZ,
	CuiPKZ,
	XiangLX,
	BaiWJ,
	FoTQ,
	ChangSM,
	TaXYW,
	NvEH,
	ZhenLJ,
	SheDJ,
	ZuiSMS,
	WuWG,
	YuQJ,
	YuRG
};

enum  ECashStoreThirdDrugs {
	JinCY,	//½ð²Ô
	XueHD,	//Ñª»¹µ¤
	JiSW,	//¼ÃÉúÍè
	JvYD,	//¾ÛÔªµ¤
	HuanLS,	//»¹ÁéÉ¢
	YuCW,	//Óñó¸Íè
	JiuZHHD,	//¾Å×ª
	GuiHL,	//¹é»êÂ¶
	XiaoYS,	//åÐÒ£É¢
	QuMD,	//ÇýÄ§µ¤
	LingXW,	//ÁéÐÄÍè
	QianKD	//Ç¬À¤µ¤
};

enum  ECashStoreSecondDrugs {
	LvSG,
	BeiMH,
	ChiSZ,
	ZiHL,
	LongWG,
	QiYG,
	ZiMG,
	QiXC,
	YueYH,
	MaH,
	LingZ,
	ChangSG,
	SheD,
	YeSZ,
	XueMN,
	YeBT,
	ShanCQ,
	LvLS,
	YaoSH,
	SongJ
};

typedef struct {
	int GoodsEnum;
	int prices;
}CashStoreGoods, *PCashStoreGoods;
typedef struct DiTuLoc {
	int x;
	int y;
	DiTuLoc() {};
	DiTuLoc(int x, int y) {
		this->x = x;
		this->y = y;
	}
	DiTuLoc& operator=(cv::Point& cvP)
	{
		this->x = cvP.x;
		this->y = cvP.y;
		return *this;
	}
	DiTuLoc& operator+(DiTuLoc loc)
	{
		this->x += loc.x;
		this->y += loc.y;
		return *this;
	}
};
typedef std::map<int, std::string> EFileNameMap;

class SwComm
{
public:
	SwComm();
	inline static char *GetSwCashStorePriceUseFont() { return (char *)"num"; };

	static const int GoodsTypeHeight = 24;
	static const int GoodsTypeSpaceHeight = 4;

	static const int GoodsGridWidth = 46;
	static const int GoodsGridHeight = 46;

	static const int GoodsGridRowSpace = 9;
	static const int GoodsGridColSpace = 7;

	static const int GoodsMoneyGridRowSpace = 4;
	static const int GoodsMoneyGridWidth = 46;
	static const int GoodsMoneyGridHeight = 15;

	static EFileNameMap UiMarkNameM;
	static EFileNameMap TaskNumberM;
	static EFileNameMap DiTuNameM;
	static EFileNameMap CookingIconNameM;
	static EFileNameMap ThirdDrugsIconNameM;
	static EFileNameMap SecondDrugsIconNameM;
	static std::string GetECSGT_SName(ECashStoreGoodsType csg);

	static cv::Point CADefaultFlyPoint;
	static cv::Point ALDefaultFlyPoint;
	static cv::Point QHDefaultFlyPoint;
	static cv::Point LXDefaultFlyPoint;
	static cv::Point NEDefaultFlyPoint;


	static std::map<EMap, cv::Point> DiTuZuoBiaoMax;

};

#define OPERATOR_SUCCESS		0
#define OPERATOR_FAILED			0xfffff
#define SHOT_ERROR				0xc0001
#define ALLOC_ERROR				0xc0002
#define GET_IMAGR_SIZE_OUTOF	0xc0003
#define PARSE_PIXELBYTE_ERROR	0xc0004
#define FILE_FORMAT_ERROR		0xc0005
#define SIZE_NOT_MATCH_ERROR	0xc0006
#define SIZE_ERROR				0xc0007
#define NOT_MATCH				0xc0008
#define MATCH_SUCCESS			0xc0009
#define OPENDIR_ERROR			0xc0010

#define FIGHTOVER				0xc0101
#define WAITFIGHTCOMMAND		0xc0102
#define RICHANGFINISHED			0xc0103
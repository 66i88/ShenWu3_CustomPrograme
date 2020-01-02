#include "stdafx.h"
#include "SwComm.h"


SwComm::SwComm()
{

}

EFileNameMap SwComm::UiMarkNameM = {
	{ EUiMark::EUMTaiYang, "taiyang.bmp" },
	{ EUiMark::EUMYueLiang, "yueliang.bmp" },
	{ EUiMark::EUMJiShouZhongXin, "jishouzhongxin.bmp" },
	{ EUiMark::EUMJiaoYiZhongXin, "jiaoyizhongxin.bmp" },
	{ EUiMark::EUMDiTu, "ditu.bmp"},
	{ EUiMark::EUMRiCheng, "richeng.bmp" },
	{ EUiMark::EUMRenWu, "renwu.jpg" },
	{ EUiMark::EUMWuPin, "shuxing.bmp" },
	{ EUiMark::EUMHaoYou, "haoyou.bmp" },
	{ EUiMark::EUMLingHuLeYuan_WanCheng, "linghuleyuan_wancheng.bmp" },
	{ EUiMark::EUMLingHuLeYuan_WeiWanCheng, "linghuleyuan_weiwancheng.bmp" },
	{ EUiMark::EUMShiMenRenWu_WanCheng, "shimenrenwu_wancheng.bmp" },
	{ EUiMark::EUMShiMenRenWu_WeiWanCheng1, "shimenrenwu_weiwancheng1.bmp" },
	{ EUiMark::EUMShiMenRenWu_WeiWanCheng2, "shimenrenwu_weiwancheng2.bmp" },
	{ EUiMark::EUMBaoTuRenWu_WanCheng, "baoturenwu_wancheng.bmp" },
	{ EUiMark::EUMBaoTuRenWu_WeiWanCheng1, "baoturenwu_weiwancheng1.bmp" },
	{ EUiMark::EUMBaoTuRenWu_WeiWanCheng2, "baoturenwu_weiwancheng2.bmp" },
	{ EUiMark::EUMXiuLian_WanCheng, "xiulianrenwu_wancheng.bmp" },
	{ EUiMark::EUMXiuLian_WeiWanCheng, "xiulianrenwu_weiwancheng.bmp" },
	{ EUiMark::EUMZiDongXunLu,"zidongxunlu.bmp" },
	{ EUiMark::EUMXinYuGouMai,"xinyugoumai.bmp" },
	{ EUiMark::EUMXianJinGouMai,"xianjingoumai.bmp" },
	{ EUiMark::EUMFeiXingQi,"feixingqi.bmp" },
	{ EUiMark::EUMDuiHuaKuang,"duihuakuang.bmp" },
	{ EUiMark::EUMFeiXingQiHongSe,"feixingqi_hongse.bmp" },
	{ EUiMark::EUMFeiXingQiLanSe,"feixingqi_lanse.bmp" },
	{ EUiMark::EUMQuMoXiang,"qumoxiang.bmp" },
	{ EUiMark::EUMQFeiXingQiDiTu,"feixingqiditu.bmp" },
};

EFileNameMap SwComm::TaskNumberM = {
	{ ETaskNum::ETNum0,"num0.bmp" },
	{ ETaskNum::ETNum1,"num1.bmp" },
	{ ETaskNum::ETNum2,"num2.bmp" },
	{ ETaskNum::ETNum3,"num3.bmp" },
	{ ETaskNum::ETNum4,"num4.bmp" },
	{ ETaskNum::ETNum5,"num5.bmp" },
	{ ETaskNum::ETNum6,"num6.bmp" },
	{ ETaskNum::ETNum7,"num7.bmp" },
	{ ETaskNum::ETNum8,"num8.bmp" },
	{ ETaskNum::ETNum9,"num9.bmp" },
	{ ETaskNum::ETPoint,"point.bmp" },
};

EFileNameMap SwComm::DiTuNameM = {
	{ EMap::EMChangAnCheng,"changancheng.bmp" },
	{ EMap::EMAoLaiGuo, "aolaiguo.bmp"         },
	{ EMap::EMQingHeZhen, "qinghezhen.bmp" },
	{ EMap::EMLinXianZhen, "linxianzhen.bmp" },
	{ EMap::EMNvErGuo, "nverguo.bmp" },
	{ EMap::EMLingXiaoTianGong, "lingxiaotiangong.bmp" },
	{ EMap::EMLingXiaoDian, "lingxiaodian.bmp" },
	{ EMap::EMBangPai, "bangpai.bmp" },
	{ EMap::EMLingHuLeYuan, "linghuleyuan.bmp" },
	{ EMap::EMFoMen, "fomen.bmp" },
	{ EMap::EMLMoWangShan, "mowangshan.bmp" }
};

EFileNameMap SwComm::CookingIconNameM = {
	{ ECashStoreCooking::DouF,"" },
	{ ECashStoreCooking::JiaoHJ,"jiaohuaji.bmp" },
	{ ECashStoreCooking::BaBZ,"babaozhou.bmp" },
	{ ECashStoreCooking::ZhenZWZ,"zhenzhuwangzi.bmp" },
	{ ECashStoreCooking::CuiPKZ,"cuipiruzhu.bmp" },
	{ ECashStoreCooking::XiangLX,"xianglaxie.bmp" },
	{ ECashStoreCooking::BaiWJ,"baiweijiu.bmp" },
	{ ECashStoreCooking::FoTQ,"fotiaoqiang.bmp" },
	{ ECashStoreCooking::ChangSM,"changshoumian.bmp" },
	{ ECashStoreCooking::TaXYW,"taxueyanwo.bmp" },
	{ ECashStoreCooking::NvEH,"nverhong.bmp" },
	{ ECashStoreCooking::ZhenLJ,"zhenlujiu.bmp" },
	{ ECashStoreCooking::SheDJ,"shedanjiu.bmp" },
	{ ECashStoreCooking::ZuiSMS,"zuishengmengsi.bmp" },
	{ ECashStoreCooking::WuWG,"wuweigeng.bmp" },
	{ ECashStoreCooking::YuQJ,"yuqiongjiang.bmp" },
	{ ECashStoreCooking::YuRG,"" }
};

EFileNameMap SwComm::ThirdDrugsIconNameM = {
	{ ECashStoreThirdDrugs::JinCY,"jincangyao.jpg" },
	{ ECashStoreThirdDrugs::XueHD,"xuehuandan.jpg" },
	{ ECashStoreThirdDrugs::JiSW,"jishengwan.jpg" },
	{ ECashStoreThirdDrugs::JvYD,"jvyuandan.jpg" },
	{ ECashStoreThirdDrugs::HuanLS,"huanlingsan.jpg" },
	{ ECashStoreThirdDrugs::YuCW,"yuchanwan.jpg" },
	{ ECashStoreThirdDrugs::JiuZHHD,"jiuzhuanhuanhundan.jpg" },
	{ ECashStoreThirdDrugs::GuiHL,"guihunlu.jpg" },
	{ ECashStoreThirdDrugs::XiaoYS,"xiaoyaosan.jpg" },
	{ ECashStoreThirdDrugs::QuMD,"qumodan.jpg" },
	{ ECashStoreThirdDrugs::LingXW,"lingxinwan.jpg" },
	{ ECashStoreThirdDrugs::QianKD,"qiankuandan.jpg" }
};

EFileNameMap SwComm::SecondDrugsIconNameM = {
	{ ECashStoreSecondDrugs::LvSG,"" },
	{ ECashStoreSecondDrugs::BeiMH,"beimuhua.jpg" },
	{ ECashStoreSecondDrugs::ChiSZ,"chishizhi.jpg" },
	{ ECashStoreSecondDrugs::ZiHL,"zihualan.jpg" },
	{ ECashStoreSecondDrugs::LongWG,"longweigu.jpg" },
	{ ECashStoreSecondDrugs::QiYG,"qiyiguo.jpg" },
	{ ECashStoreSecondDrugs::ZiMG,"zimingguo.jpg" },
	{ ECashStoreSecondDrugs::QiXC,"qixingcao.jpg" },
	{ ECashStoreSecondDrugs::YueYH,"yueyuehua.jpg" },
	{ ECashStoreSecondDrugs::MaH,"mahuang.jpg" },
	{ ECashStoreSecondDrugs::LingZ,"lingzhi.jpg" },
	{ ECashStoreSecondDrugs::ChangSG,"changshouguo.jpg" },
	{ ECashStoreSecondDrugs::SheD,"shedan.jpg" },
	{ ECashStoreSecondDrugs::YeSZ,"yeshangzhu.jpg" },
	{ ECashStoreSecondDrugs::XueMN,"xuemanao.jpg" },
	{ ECashStoreSecondDrugs::YeBT,"yebaitou.jpg" },
	{ ECashStoreSecondDrugs::ShanCQ,"shanchangqing.jpg" },
	{ ECashStoreSecondDrugs::LvLS,"lvliansan.jpg" },
	{ ECashStoreSecondDrugs::YaoSH,"yaoshenhua.jpg" },
	{ ECashStoreSecondDrugs::SongJ,"songjie.jpg" }
};


std::string SwComm::GetECSGT_SName(ECashStoreGoodsType csg)
{
	std::string str;
	switch (csg)
	{
	case ECSGTDefault:
		str = "Default";
		break;
	case ECSGTFurniture:
		str = "Furniture";
		break;
	case ECSGTCourtyardDecoration:
		str = "CourtyardDecoration";
		break;
	case ECSGTAntique:
		str = "Antique";
		break;
	case ECSGTCooking:
		str = "Cooking";
		break;
	case ECSGTThirdDrugs:
		str = "ThirdDrugs";
		break;
	case ECSGTSecondDrugs:
		str = "SecondDrugs";
		break;
	case ECSGTFood:
		str = "Food";
		break;
	case ECSGTFlower:
		str = "Flower";
		break;
	case ECSGTUnSeeWeapon:
		str = "UnSeeWeapon";
		break;
	case ECSGTMonsterManual:
		str = "MonsterManual";
		break;
	case ECSGTTransformationalCard:
		str = "TransformationalCard";
		break;
	case ECSGTEquipment:
		str = "Equipment";
		break;
	default:
		break;
	}
	return str;
}

cv::Point SwComm::CADefaultFlyPoint = cv::Point(540, 380);
cv::Point SwComm::ALDefaultFlyPoint = cv::Point(430, 357);
cv::Point SwComm::QHDefaultFlyPoint = cv::Point(270, 425);
cv::Point SwComm::LXDefaultFlyPoint = cv::Point(376, 328);
cv::Point SwComm::NEDefaultFlyPoint = cv::Point(416, 313);

std::map<EMap, cv::Point> SwComm::DiTuZuoBiaoMax = {
	{EMap::EMChangAnCheng,cv::Point(549, 325)},
	{EMap::EMAoLaiGuo,cv::Point(160, 120)},
	{EMap::EMQingHeZhen,cv::Point(160, 120)},
	{EMap::EMLinXianZhen,cv::Point(200, 120)},
	{EMap::EMNvErGuo,cv::Point(159, 120)},
	{EMap::EMChangAnChengWai,cv::Point(159, 90)},
	{EMap::EMQingHeZhenWai,cv::Point(120, 90) },
	{EMap::EMDaTangGuoJing,cv::Point(200, 180) },
	{EMap::EMDaTangJingWai,cv::Point(306, 173) },
	{EMap::EMWuSiZang,cv::Point(240, 120) },
	{EMap::EMQingHeZhenWai,cv::Point(119, 90) },
	{EMap::EMTianCe,cv::Point(169, 130) },
	{EMap::EMLingHuLeYuan,cv::Point(179, 135) },
	{EMap::EMHuaGuoShan,cv::Point(199, 120) },
};

#pragma once
#include <iostream>
#include <atlstr.h>
#include <string>
#include <vector>

#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\opencv.hpp>
#include <opencv2\imgproc.hpp>

class Unitl
{
public:
	Unitl();

	static BOOL GetShotArea(cv::Rect rect, cv::Mat& mat_dest);

	static int GetDirFiles(const char* dirPath, std::vector<std::string> &_fileNameVec);

	static std::string GetAppPathS();
	static std::wstring GetAppPathWS();
	static BITMAPINFOHEADER MakeBitMapInfoHeader(BITMAP &bitMap);
	static BITMAPFILEHEADER MakeBitMapFileHeader(BITMAPINFOHEADER& bitMapInfoHeader);
	static std::vector<cv::Point> MatchMat_VecRet(cv::Mat& src, cv::Mat& mch, double matchRate, cv::TemplateMatchModes match_method = cv::TM_SQDIFF_NORMED);
	static void SaveMatAreaToPictureDir(cv::Mat& src, cv::Rect rect);
	static void SaveMatAreaToPictureDir(cv::Mat& src, cv::Rect rect, std::string relativeAppDir);
	static void SaveMatAreaToPictureDir(cv::Mat& src, cv::Rect rect, std::string relativeAppDir, std::string fileName);
	static cv::Point MatchMat(cv::Mat& src, std::string mchfilePath, double matchRate, cv::TemplateMatchModes match_method = cv::TM_SQDIFF_NORMED);
	static cv::Point MatchMat(cv::Mat& src, cv::Mat& mch, double matchRate, cv::TemplateMatchModes match_method = cv::TM_SQDIFF_NORMED);
	static cv::Point MatchMatWithCenter(cv::Mat& src, cv::Mat& mch, double matchRate, cv::Point centerPoint, cv::TemplateMatchModes match_method = cv::TM_SQDIFF_NORMED);

	static cv::Point FindPixel(cv::Mat& src, int r, int g, int b, int deviation = 0);
	static cv::Point FindPixelOnTail(cv::Mat& src, int r, int g, int b, int deviation);
	static std::string OCRPixelByteToString(const char* language, cv::Mat& wordMat);

	static int SetMouseHook();
};


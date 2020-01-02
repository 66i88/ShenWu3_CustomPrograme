#include "stdafx.h"
#include "unitl.h"
#include <io.h>
#include <atlconv.h>
#include <string>
#include <codecvt>

#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\opencv.hpp>
#include <opencv2\imgproc.hpp>

#include <tesseract/strngs.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#pragma comment(lib,"GdiPlus.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "libtesseract302.lib")
#pragma comment(lib, "liblept168.lib")

Unitl::Unitl()
{

}

/*
*	以桌面左上角(0,0)为原点
*	像素点颜色顺序依次为 B -> G -> R
*/
BOOL Unitl::GetShotArea(cv::Rect rect, cv::Mat& mat_dest)
{
	HWND pDesktop;
	HDC pDC, hdcCompatible = NULL;
	BOOL b_ret = true;
	BITMAP bmp;
	WORD    cClrBits;
	HBITMAP hbmScreen = NULL;
	int biSizeImage;
	PBYTE pDest = NULL;
	pDesktop = GetDesktopWindow();
	pDC = GetDC(pDesktop);
	hdcCompatible = CreateCompatibleDC(pDC);
	/*rect.x = rect.x < deskTopRect.left ? deskTopRect.left : rect.x;
	rect.y = rect.y < deskTopRect.top ? deskTopRect.top : rect.y;
	if (rect.width > deskTopRect.right - rect.x) {
		rect.width = deskTopRect.right - rect.x;
	}
	if (rect.height > deskTopRect.bottom - rect.y) {
		rect.height = deskTopRect.bottom - rect.y;
	}*/
	hbmScreen = CreateCompatibleBitmap(pDC, rect.width, rect.height);

	if (hbmScreen == NULL)
	{
		b_ret = false;
		goto DONE;
	}
	// Select the bitmaps into the compatible DC.
	if (!SelectObject(hdcCompatible, hbmScreen))
	{
		b_ret = false;
		goto DONE;
	}
	//Copy color data for the entire display into a 
	//bitmap that is selected into a compatible DC. 
	if (!BitBlt(hdcCompatible,
		0, 0,
		rect.width, rect.height,
		pDC,
		rect.x, rect.y,
		SRCCOPY))
	{
		b_ret = false;
		goto DONE;
	}
	// Retrieve the bitmap's color format, width, and height. 
	if (!GetObject(hbmScreen, sizeof(BITMAP), (LPSTR)&bmp))
	{
		b_ret = false;
		goto DONE;
	}

	// Convert the color format to a count of bits. 
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	{
		int sa = bmp.bmWidth;
	}
	biSizeImage = ((rect.width * cClrBits + 31) & ~31)
		/ 8 * rect.height;
	pDest = new BYTE[biSizeImage];
	if (!pDest)
	{
		b_ret = false;
		goto DONE;
	}
	BITMAPINFO bitMapInfo;
	bitMapInfo.bmiHeader = Unitl::MakeBitMapInfoHeader(bmp);
	if (!GetDIBits(pDC, hbmScreen, 0, rect.height, pDest, &bitMapInfo, DIB_RGB_COLORS))
	{
		//_T("GetDIBits() error"));
		b_ret = false;
		goto DONE;
	}

DONE:
	if (hbmScreen)
	{
		DeleteObject(hbmScreen);
	}
	if (hdcCompatible)
	{
		DeleteDC(hdcCompatible);
	}
	ReleaseDC(NULL, pDC);
	if (pDest)
	{
		mat_dest = cv::Mat(rect.height, rect.width, CV_8UC4, pDest);
		cv::flip(mat_dest, mat_dest, cv::ROTATE_90_CLOCKWISE);
		cv::cvtColor(mat_dest, mat_dest, cv::COLOR_BGRA2BGR);
		delete pDest;
	}
	return b_ret;
}

std::string GetRandomUniqueS()
{
	static bool IsFileNameRandomInit = false;
	std::ostringstream oss;
	time_t nowTimeSec;
	time(&nowTimeSec);
	tm *time;
	time = localtime(&nowTimeSec);
	if (!IsFileNameRandomInit)
	{
		DWORD now_ms = GetTickCount();
		srand(now_ms);
		IsFileNameRandomInit = true;
	}
	oss << time->tm_year + 1900 << "-" << time->tm_mon + 1 << "-" << time->tm_mday << "_" << time->tm_hour << "-" <<
		time->tm_min << "-" << time->tm_sec << "_";
	for (int i = 0; i < 10; i++)
	{
		oss << rand() % 10;
	}
	return oss.str();
}

void Unitl::SaveMatAreaToPictureDir(cv::Mat& src, cv::Rect rect, std::string relativeAppDir, std::string fileName)
{
	cv::Mat mat_tmp;
	src(rect).copyTo(mat_tmp);
	std::string filePath;
	filePath += GetAppPathS() + SWPICTUREDIR + "\\";
	if (!relativeAppDir.empty()) {
		filePath.append(relativeAppDir + "\\");
	}
	if (fileName.empty())
	{
		fileName = GetRandomUniqueS();
	}
	filePath.append(fileName);
	cv::imwrite(filePath, mat_tmp);
}

void Unitl::SaveMatAreaToPictureDir(cv::Mat& src, cv::Rect rect)
{
	SaveMatAreaToPictureDir(src, rect, "", "");
}

void Unitl::SaveMatAreaToPictureDir(cv::Mat& src, cv::Rect rect, std::string relativeAppDir)
{
	SaveMatAreaToPictureDir(src, rect, relativeAppDir, "");
}

cv::Point Unitl::FindPixel(cv::Mat& src, int r, int g, int b, int deviation)
{
	cv::Point mchPoint(-1, -1);
	int chl = src.channels();

	cv::Mat binMat;
	cv::inRange(src, cv::Scalar(b - deviation, g - deviation, r - deviation),
		cv::Scalar(b + deviation, g + deviation, r + deviation), binMat);
	for (int row = 0; row < binMat.rows; row++)
	{
		int col = 0;
		for (; col < binMat.cols; col++)
		{
			if (binMat.data[row*binMat.cols + col] == 0xff)
			{
				mchPoint = cv::Point(col, row);
				break;
			}
		}
		if (col != binMat.cols)
		{
			break;
		}
	}
	return mchPoint;
}

cv::Point Unitl::FindPixelOnTail(cv::Mat& src, int r, int g, int b, int deviation)
{
	cv::Point mchPoint(-1, -1);
	int chl = src.channels();

	cv::Mat binMat;
	cv::inRange(src, cv::Scalar(b - deviation, g - deviation, r - deviation),
		cv::Scalar(b + deviation, g + deviation, r + deviation), binMat);
	for (int row = 0; row < binMat.rows; row++)
	{
		for (int col = 0; col < binMat.cols; col++)
		{
			if (binMat.data[row*binMat.cols + col] == 0xff)
			{
				mchPoint = cv::Point(col, row);
			}
		}
	}
	return mchPoint;
}

cv::Point Unitl::MatchMat(cv::Mat& src, std::string mchfilePath, double matchRate, cv::TemplateMatchModes match_method)
{
	cv::Mat mch = cv::imread(mchfilePath, cv::IMREAD_UNCHANGED);
	if (mch.empty()) {
		return cv::Point(-1, -1);
	}
	return MatchMat(src, mch, matchRate, match_method);
}

std::vector<cv::Point> Unitl::MatchMat_VecRet(cv::Mat& src, cv::Mat& mch, double matchRate, cv::TemplateMatchModes match_method)
{
	int s4_ret = INT_MAX;

	int width, height;
	cv::Mat mat_ret;
	width = src.cols - mch.cols + 1;
	height = src.rows - mch.rows + 1;
	mat_ret.create(height, width, CV_32FC1);
	cv::matchTemplate(src, mch, mat_ret, match_method);
	cv::Point minPoint, maxPoint, tmpPoint;
	double min, max;
	cv::minMaxLoc(mat_ret, &min, &max, &minPoint, &maxPoint);
	double compare_ret = 0;
	if (match_method == cv::TM_SQDIFF_NORMED || match_method == cv::TM_SQDIFF)
	{
		tmpPoint = minPoint;
		compare_ret = 1 - min;
	}
	else {
		tmpPoint = maxPoint;
		compare_ret = max;
	}
	std::vector<cv::Point> mchPointVec;
	//LOG_MSG(DINFO, "Match Mat,rate=%s, Point(%d,%d)", std::string(std::to_string(compare_ret * 100), 0, 5).c_str(), tmpPoint.x, tmpPoint.y);
	//查找失败，返回空vec
	if (compare_ret < matchRate) {
		return mchPointVec;
	}
	double mchVal = mat_ret.ptr<float>(tmpPoint.y)[tmpPoint.x];
	for (int i = 0; i < mat_ret.rows; i++)
	{
		for (int j = 0; j < mat_ret.cols; j++)
		{

			if (mat_ret.ptr<float>(i)[j] >= mchVal - 0.02 &&
				mat_ret.ptr<float>(i)[j] <= mchVal + 0.02)
			{
				mchPointVec.push_back(cv::Point(j, i));
			}
		}
	}
	return mchPointVec;
}

std::string srcWd = "src";
std::string mchWd = "match";

cv::Point Unitl::MatchMatWithCenter(cv::Mat& src, cv::Mat& mch, double matchRate, cv::Point centerPoint, cv::TemplateMatchModes match_method)
{
	int s4_ret = INT_MAX;
	int width, height;
	cv::Mat mat_ret;
	width = src.cols - mch.cols + 1;
	height = src.rows - mch.rows + 1;
	mat_ret.create(height, width, CV_32FC1);
	cv::matchTemplate(src, mch, mat_ret, match_method);
	//值归一化, 摒弃
	//cv::normalize(mat_ret, mat_ret, 1, 0, cv::NORM_MINMAX, -1, cv::Mat());
	cv::Point minPoint, maxPoint, tmpPoint;
	double min, max;
	cv::minMaxLoc(mat_ret, &min, &max, &minPoint, &maxPoint);

	double compare_ret = 0;
	//TM_SQDIFF_NORMED，TM_CCORR_NORMED，TM_CCOEFF_NORMED是标准化的匹配，得到的最大值，最小值范围在0~1之间，
	//其它则需要自己对结果矩阵归一化。
	if (match_method == cv::TM_SQDIFF_NORMED || match_method == cv::TM_SQDIFF)
	{
		tmpPoint = minPoint;
		compare_ret = 1 - min;
	}
	else {
		tmpPoint = maxPoint;
		compare_ret = max;
	}
	//LOG_MSG(DINFO, "Match Mat,rate=%s, Point(%d,%d)", std::string(std::to_string(compare_ret * 100), 0, 5).c_str(), tmpPoint.x, tmpPoint.y);
	if (compare_ret < matchRate) {
		return cv::Point(-1, -1);
	}
	//double tttt = mat_ret.at<cv::Vec2i>[tmpPoint.y][tmpPoint.x];
	double mchVal = mat_ret.ptr<float>(tmpPoint.y)[tmpPoint.x];
	std::vector<cv::Point> mchPointVec;
	for (int i = 0; i < mat_ret.rows; i++)
	{
		for (int j = 0; j < mat_ret.cols; j++)
		{

			if (mat_ret.ptr<float>(i)[j] >= mchVal - 0.01
				&& mat_ret.ptr<float>(i)[j] <= mchVal + 0.01)
			{
				//LOG_MSG(DINFO, "Match Point(%d,%d)", j, i);
				mchPointVec.push_back(cv::Point(j, i));
			}
		}
	}
	int distance = INT_MAX;
	cv::Point retPoint(-1, -1);
	for (auto p : mchPointVec)
	{
		cv::Point temp = p - centerPoint;
		int calcDistance = sqrt(temp.x* temp.x + temp.y* temp.y);
		if (calcDistance < distance)
		{
			distance = calcDistance;
			retPoint = p;
		}
	}
	/*cv::rectangle(mch_dst, tmpLoc, cv::Point(tmpLoc.x + mch_tmp.cols, tmpLoc.y + mch_tmp.rows),
	cv::Scalar(0, 0, 255), 2);
	cv::rectangle(mat_ret, cv::Rect(tmpLoc.x, tmpLoc.y, mch_tmp.cols, mch_tmp.rows),
	cv::Scalar(0, 0, 255), 2);*/
	return retPoint;
}

cv::Point Unitl::MatchMat(cv::Mat& src, cv::Mat& mch, double matchRate, cv::TemplateMatchModes match_method)
{
	int s4_ret = INT_MAX;
	int width, height;
	cv::Mat mat_ret;
	width = src.cols - mch.cols + 1;
	height = src.rows - mch.rows + 1;
	mat_ret.create(height, width, CV_32FC1);
	cv::matchTemplate(src, mch, mat_ret, match_method);
	cv::Point minPoint, maxPoint, tmpPoint;
	double min, max;
	cv::minMaxLoc(mat_ret, &min, &max, &minPoint, &maxPoint);
	double compare_ret = 0;
	if (match_method == cv::TM_SQDIFF_NORMED || match_method == cv::TM_SQDIFF)
	{
		tmpPoint = minPoint;
		compare_ret = 1 - min;
	}
	else {
		tmpPoint = maxPoint;
		compare_ret = max;
	}
	//LOG_MSG(DINFO, "Match Mat,rate=%s, Point(%d,%d)", std::string(std::to_string(compare_ret * 100), 0, 5).c_str(), tmpPoint.x, tmpPoint.y);
	if (compare_ret > matchRate) {
		return tmpPoint;
	}
	return cv::Point(-1, -1);
}

int Unitl::GetDirFiles(const char* dirPath, std::vector<std::string> &_fileNameVec)
{
	intptr_t handle;
	_finddata_t findData;
	char temp[MAX_PATH];
	memset(temp, 0, MAX_PATH);
	sprintf(temp, "%s\\*", dirPath);
	if ((handle = _findfirst(temp, &findData)) == -1)
	{
		return OPENDIR_ERROR;
	}
	do
	{
		if (!(findData.attrib & _A_SUBDIR) && strcmp(findData.name, ".") != 0
			&& strcmp(findData.name, "..") != 0)    // 是否是子目录并且不为"."或".."
		{
			_fileNameVec.push_back(findData.name);
		}
	} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件
	_findclose(handle);
	return OPERATOR_SUCCESS;
}


BITMAPINFOHEADER Unitl::MakeBitMapInfoHeader(BITMAP &bitMap)
{
	BITMAPINFOHEADER bitMapInfo;
	// Convert the color format to a count of bits. 
	int cClrBits = (WORD)(bitMap.bmPlanes * bitMap.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;
	// Allocate memory for the BITMAPINFO structure. (This structure 
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
	// data structures.) 

	// Initialize the fields in the BITMAPINFO structure. 
	bitMapInfo.biSize = sizeof(BITMAPINFOHEADER);
	bitMapInfo.biWidth = bitMap.bmWidth;
	bitMapInfo.biHeight = bitMap.bmHeight;
	bitMapInfo.biPlanes = bitMap.bmPlanes;
	bitMapInfo.biBitCount = bitMap.bmBitsPixel;
	if (cClrBits < 24) {
		bitMapInfo.biClrUsed = (1 << cClrBits);
	}
	// If the bitmap is not compressed, set the BI_RGB flag. 
	bitMapInfo.biCompression = BI_RGB;
	// Compute the number of bytes in the array of color 

	// indices and store the result in biSizeImage. 
	bitMapInfo.biSizeImage = ((bitMapInfo.biWidth * cClrBits + 31) & ~31)
		/ 8 * bitMapInfo.biHeight;
	// Set biClrImportant to 0, indicating that all of the device colors are important. 
	bitMapInfo.biClrImportant = 0;
	return bitMapInfo;
}

BITMAPFILEHEADER Unitl::MakeBitMapFileHeader(BITMAPINFOHEADER &bitMapInfoHeader)
{
	BITMAPFILEHEADER header;
	header.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
								   // Compute the size of the entire file. 
	header.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		bitMapInfoHeader.biSize + bitMapInfoHeader.biClrUsed
		* sizeof(RGBQUAD) + bitMapInfoHeader.biSizeImage);
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	// Compute the offset to the array of color indices. 
	header.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		bitMapInfoHeader.biSize;
	return header;
}


std::string Unitl::OCRPixelByteToString(const char* language, cv::Mat& wordMat)
{
	char *outStr;
	std::string str;
	//tesseract::TessBaseAPI api;
	tesseract::TessBaseAPI *api = NULL;
	api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init(NULL, language)) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		return str;
	}
	api->SetImage(wordMat.data, wordMat.cols, wordMat.rows, wordMat.channels(), wordMat.cols* wordMat.channels());
	outStr = api->GetUTF8Text();

	USES_CONVERSION;
	WCHAR *test = A2W(outStr);
	std::wstring strTest(test);
	str = std::string(outStr);

DONE:
	api->Clear();
	api->End();
	return str;
}

//int Unitl::OCRPixelByteToInt(BITMAPINFOHEADER &bitMapInfoHeader, PBYTE bmpByte)
//{
//	char *ctStr = OCRPixelByteToString(SwComm::GetSwCashStorePriceUseFont(), bitMapInfoHeader, bmpByte);
//	CString cstr = ctStr;
//	for (int i = 0; i < cstr.GetLength(); i++) {
//		if (cstr.GetAt(i) < '0' || cstr.GetAt(i) > '9') {
//			cstr.Delete(i, 1);
//			i--;
//		}
//	}
//	int ret = _ttoi(cstr);
//	return ret;
//}


std::string Unitl::GetAppPathS()
{
	char selfModulePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, selfModulePath, MAX_PATH);
	int cutLen = std::string(selfModulePath).find_last_of('\\') + 1;
	std::string self(selfModulePath, cutLen);
	return self;
}

std::wstring Unitl::GetAppPathWS()
{
	wchar_t selfModulePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, selfModulePath, MAX_PATH);
	wchar_t *pstr = wcsrchr(selfModulePath, '\\');
	memset(pstr + 1, 0, 2);
	std::wstring strAppPath(selfModulePath);
	return strAppPath;
}

static LRESULT CALLBACK KeyHookFunc(int code, WPARAM wParam, LPARAM lParam)
{
	int aa = 1;
	return CallNextHookEx(NULL, code,wParam,lParam);
}

int Unitl::SetMouseHook()
{
	HHOOK hhook=0;
	hhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyHookFunc, (HINSTANCE)0, 0);
	return 0;
}
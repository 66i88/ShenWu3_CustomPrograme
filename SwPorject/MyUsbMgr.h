#pragma once
#include <Windows.h>
#include "stdafx.h"
#include <opencv2\opencv.hpp>

extern "C" {
#include "setupapi.h"
#include "hidsdi.h"
}

#define HIDVendorID					L"0483"
#define HIDProductID				L"5710"

#define CustromHID_ReportID			0x00
#define CustromHID_OutSize			22
#define CustromHID_InSize			22

#define MouseBufSize				6
#define SetMouseLimitBufSize		6
#define KPBufSize					3

#define KP_SHIFT			16
#define KP_CTRL				17
#define KP_ALT				18
#define KP_ESC				27

#define KEYBTN_F1			0x070
#define KEYBTN_F2			0x070+1
#define KEYBTN_F3			0x070+2
#define KEYBTN_F4			0x070+3
#define KEYBTN_F5			0x070+4
#define KEYBTN_F6			0x070+5
#define KEYBTN_F7			0x070+6
#define KEYBTN_F8			0x070+7
#define KEYBTN_F9			0x070+8


enum ERequestType {
	Req_SetMouseAbsLimit,
	Req_KP,
	Req_Mouse
};


typedef struct CustomHIDInData {
	ERequestType rType;
	BYTE data[CustromHID_OutSize];
};

enum EMouseEventOperator {
	LeftBtn_Down,
	LeftBtn_Up,
	LeftBtn_Click,
	RightBtn_Down,
	RightBtn_Up,
	RightBtn_Click,
	MoveAbs
};

enum EKeyBoardEventOperator {
	KeyDown,
	KeyUp,
	KeyClick
};

struct MouseOperator {
	EMouseEventOperator op;
	UINT16 x;
	UINT16 y;
	MouseOperator() {

	}
	MouseOperator(EMouseEventOperator op) {
		this->op = op;
	}
	MouseOperator(EMouseEventOperator op, UINT16 x, UINT16 y) {
		this->op = op;
		this->x = x;
		this->y = y;
	}
	MouseOperator(EMouseEventOperator op, cv::Point cp) {
		this->op = op;
		this->x = cp.x;
		this->y = cp.y;
	}
	cv::Point GetCvPoint()
	{
		return cv::Point(x, y);
	}
};

/*
*	***Mouse
*	0-1 : Button left right wheel
*	2-4 : X(0-1920)
*	5-7 : Y(0-1080)
*
*	***KeyBoard
*	0-1 : LeftControl - Right GUI
*	2-8 : Key
*/


class MyUsbMgr
{
private:
	HDEVINFO m_HHidGuid;
	HANDLE m_CHIDHandle;
	MyUsbMgr();
	int GetDevicePathVec(std::vector<std::wstring> &devicePathVec);
	int OpenDevice(std::vector<std::wstring> devicePathVec, std::wstring choiceVid, std::wstring choicePid);
	int AfterOpen(std::wstring dPathVec);
public:
	static MyUsbMgr* GetInstance()
	{
		static MyUsbMgr instance;
		return &instance;
	}
	~MyUsbMgr();
	int Init();
	int Start(UINT16 xMax = 0, UINT16 yMax = 0);
	int SendMouseOperator(MouseOperator opt);
	int SetMouseAbsLimit(UINT16 x, UINT16 y);
	int SendKeyBoardOperator(EKeyBoardEventOperator opt, int code);
	int SendKeyWithAlt(int code);
	int SendKeyWithCtrl(int code);
	BOOL Send(PBYTE data, int length);
};


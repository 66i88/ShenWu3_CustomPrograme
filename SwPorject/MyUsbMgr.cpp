#include "stdafx.h"
#include "MyUsbMgr.h"
#include <windows.h>
#include <Usbiodef.h>
#include <Usbioctl.h>
#include <devguid.h>
#include <winioctl.h>
#include <INITGUID.h>
#include <stdio.h>
#include <iostream>
#include <Dbt.h>
#include <map>
#include <wchar.h>
#include <iostream>
#include <atlconv.h>
#include <ctime>

extern "C" {
#include "setupapi.h"
#include "hidsdi.h"
}


#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

MyUsbMgr::MyUsbMgr()
{
}


MyUsbMgr::~MyUsbMgr()
{
}



int MyUsbMgr::GetDevicePathVec(std::vector<std::wstring> &devicePathVec)
{
	SP_DEVINFO_DATA deviceInfoData;
	SP_DEVICE_INTERFACE_DATA  deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	ULONG deviceInterfaceDetailDataSize;

	//PSP_DEVICE_INTERFACE_DETAIL_DATA test;
	int Count = 0; //Total number of devices found
	DWORD strSize = 0, requiredSize = 0;
	BOOL b_ret, b_getDetail;
	//SetupDiEnumDeviceInterfaces();
	std::wstring dPath = L"";
	b_ret = false;
	b_getDetail = false;
	GUID deviceId;
	HidD_GetHidGuid(&deviceId);
	m_HHidGuid = SetupDiGetClassDevs(&deviceId, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	do
	{
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		b_ret = SetupDiEnumDeviceInterfaces(
			m_HHidGuid,
			NULL, // IN PSP_DEVINFO_DATA  DeviceInfoData,  OPTIONAL
			&deviceId,
			Count,
			&deviceInterfaceData
		);
		//获得设备详细数据（初步）
		SetupDiGetDeviceInterfaceDetail(m_HHidGuid,
			&deviceInterfaceData,
			NULL,
			0,
			&strSize,
			NULL);
		requiredSize = strSize;
		deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		//再次获得详细数据
		b_getDetail = SetupDiGetDeviceInterfaceDetail(m_HHidGuid,
			&deviceInterfaceData,
			deviceInterfaceDetailData,
			strSize,
			&requiredSize,
			&deviceInfoData);

		//获得设备路径（最重要的部分）
		dPath = deviceInterfaceDetailData->DevicePath;
		devicePathVec.push_back(dPath);
		USES_CONVERSION;
		char *tstr = W2A(dPath.c_str());
		//LOG_MSG(DINFO, "%s", tstr);
		Count++;
	} while (b_ret);
	return 0;
}


int MyUsbMgr::OpenDevice(std::vector<std::wstring> devicePathVec, std::wstring choiceVid, std::wstring choicePid)
{
	HIDD_ATTRIBUTES devAttr;
	PHIDP_PREPARSED_DATA PreparsedData;
	HIDP_CAPS Capabilities;
	int readValue;
	int s4_ret(1);
	std::wstring choiceHidStr = L"vid_" + choiceVid + L"&pid_" + choicePid;
	int choiceCount = 0;
	for (int i = 0; i < devicePathVec.size(); i++)
	{
		int findIndex = devicePathVec[i].find(choiceHidStr);
		if (findIndex == -1)
		{
			continue;
		}
		//创建与HID通信的句柄
		m_CHIDHandle = CreateFile(
			devicePathVec[i].c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING, 0,
			NULL);
		if (m_CHIDHandle == INVALID_HANDLE_VALUE)
		{
			LOG_MSG(DINFO, "%s", "Invalide Device Path...");
			continue;
		}
		devAttr.Size = sizeof(HIDD_ATTRIBUTES);
		if (!HidD_GetAttributes(m_CHIDHandle, &devAttr))
		{
			CloseHandle(m_CHIDHandle);
			LOG_MSG(DINFO, "%s", "Cannot get the parameters of the HID...");
			continue;
		}
		LOG_MSG(DINFO, "Vendor ID: %d, Product ID:%d", devAttr.VendorID, devAttr.ProductID);
		if (!HidD_GetPreparsedData(m_CHIDHandle, &PreparsedData))
		{
			CloseHandle(m_CHIDHandle);
			LOG_MSG(DINFO, "%s", "Cannot get the Preparsed Data...");
			continue;
		}
		if (!HidP_GetCaps(PreparsedData, &Capabilities))
		{
			CloseHandle(m_CHIDHandle);
			LOG_MSG(DINFO, "%s", "Cannot get the Cap Data...");
			continue;
		}
		s4_ret = 0;
		break;
	}
DONE:
	return s4_ret;

}
int MyUsbMgr::AfterOpen(std::wstring dPathVec)
{
	int s4_ret(0);
	BOOL b_result(false);
	unsigned long numBytesReturned[1024];
	std::string recvBuf;
	BYTE CHID_OutBuf[CustromHID_OutSize + 1];
	BYTE CHID_InBuf[CustromHID_InSize + 1];
	DWORD wLen = 0;
	for (int i = 0; i < CustromHID_OutSize + 1; i++)
	{
		CHID_OutBuf[i] = i;
		numBytesReturned[i] = 0;
	}
	CHID_OutBuf[0] = CustromHID_ReportID;
	wLen = CustromHID_OutSize + 1;
	b_result = WriteFile(m_CHIDHandle, CHID_OutBuf, wLen, numBytesReturned, NULL);
	if (!b_result)
	{
		LOG_MSG(DINFO, "%s", "Write Data Error");
		CloseHandle(m_CHIDHandle);
		s4_ret = 1;
		goto DONE;
	}
	while (1)
	{
		b_result = ReadFile(m_CHIDHandle, CHID_InBuf, CustromHID_OutSize + 1, numBytesReturned, 0);
		//inbuffer;			
		//p->m_eDataRead=CString(inbuffer);
		//p->UpdateData(false);
		if (!b_result)
		{
			LOG_MSG(DINFO, "%s", "Cannot Read Data...");
			s4_ret = 1;
			goto DONE;
		}
		PSTR recvStr = new CHAR[127];
		memset(recvStr, 0, 127);
		for (int i = 0; i < CustromHID_InSize + 1; i++)
		{
			sprintf(recvStr, "%s%x ", recvStr, CHID_InBuf[i]);
		}
		//readValue = CHID_InBuf[1];
		LOG_MSG(DINFO, "recv:%s", CHID_InBuf);
	}
DONE:
	return s4_ret;
}

int MyUsbMgr::Init()
{
	return 0;
}
// 桌面鼠标(x,y)最大值,		(0,0)自适应，移动鼠标到右下角计算最大值
int MyUsbMgr::Start(UINT16 xMax, UINT16 yMax)
{
	int s4_ret(0);
	std::vector<std::wstring> dPathVec;

	GetDevicePathVec(dPathVec);
	if (dPathVec.size() <= 0) {
		LOG_MSG(DINFO, "%s", "Get Devices Path Error...");
		s4_ret = 1;
		goto DONE;
	}
	s4_ret = OpenDevice(dPathVec, HIDVendorID, HIDProductID);
	if (s4_ret == 0)
	{
		Sleep(50);
		if (xMax == 0 && yMax == 0)
		{
			POINT mouseTemp{ 0,0 };
			GetCursorPos(&mouseTemp);
			SetMouseAbsLimit(4095, 4095);
			POINT mouseP = { 0, 0 };
			POINT lastMouseP = { -1, -1 };
			MouseOperator opt(EMouseEventOperator::MoveAbs, 4095, 4095);
			for (int i = 0; i < 99; i++)
			{
				if (lastMouseP.x == mouseP.x && lastMouseP.y == mouseP.y)
				{
					break;
				}
				Sleep(15);
				SendMouseOperator(opt);
				lastMouseP = mouseP;
				GetCursorPos(&mouseP);
			}
			Sleep(15);
			SetMouseAbsLimit((UINT16)mouseP.x, (UINT16)mouseP.y);
			opt = MouseOperator(EMouseEventOperator::MoveAbs, (UINT16)mouseTemp.x, (UINT16)mouseTemp.y);
			Sleep(15);
			SendMouseOperator(opt);
		}
		else
		{
			SetMouseAbsLimit(xMax, yMax);
		}
	}
DONE:
	return s4_ret;
}


// x 0 - 4096 y 0 - 4096, 发送该指令后板子会重新初始化USB，程序需等待一段时间后再次连接USB设备
int MyUsbMgr::SetMouseAbsLimit(UINT16 x, UINT16 y)
{
	BOOL b_result(false);
	BYTE buf[SetMouseLimitBufSize];
	memset(buf, 0, SetMouseLimitBufSize);
	buf[0] = Req_SetMouseAbsLimit;
	buf[2] = x;
	buf[3] = x >> 8;
	buf[4] = y;
	buf[5] = y >> 8;
	b_result = Send(buf, SetMouseLimitBufSize);
	return b_result ? 0 : 1;
}

int MyUsbMgr::SendKeyBoardOperator(EKeyBoardEventOperator eop, int code)
{
	BOOL b_result(false);
	BYTE buf[KPBufSize];
	memset(buf, 0, KPBufSize);
	buf[0] = Req_KP;
	buf[1] = eop;
	buf[2] = code;
	b_result = Send(buf, KPBufSize);
	return b_result ? 0 : 1;
}

int MyUsbMgr::SendMouseOperator(MouseOperator opt)
{
	BOOL b_result(false);
	BYTE buf[MouseBufSize];
	memset(buf, 0, MouseBufSize);
	buf[0] = Req_Mouse;
	buf[1] = opt.op;
	if (opt.op == MoveAbs)
	{
		buf[2] = opt.x;
		buf[3] = opt.x >> 8;
		buf[4] = opt.y;
		buf[5] = opt.y >> 8;
		//LOG_MSG(DINFO,"MoveTo(%-4d,%4d)", opt.x, opt.y);
	}
	b_result = Send(buf, MouseBufSize);
	return b_result ? 0 : 1;
}


BOOL MyUsbMgr::Send(PBYTE data, int length)
{
	unsigned long numBytesReturned[255];
	BYTE buf[CustromHID_OutSize + 1] = { CustromHID_ReportID + 1 };
	if (length > CustromHID_OutSize + 1)
	{
		return false;
	}
	memcpy(buf + 1, data, length);
	return WriteFile(m_CHIDHandle, buf, CustromHID_OutSize + 1, numBytesReturned, NULL);
}
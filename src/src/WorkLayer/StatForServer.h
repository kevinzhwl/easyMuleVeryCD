/*
 * $Id: StatForServer.h 20483 2010-08-27 10:42:11Z huby $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#pragma once

#include "opcodes.h"


//type of rss stat report
#define OP_VC_RSS_STAT_REPORT			0x10

#define PROTOCOL_TYPE_ED		0x01
#define PROTOCOL_TYPE_HTTP		0x02
#define PROTOCOL_TYPE_FTP		0x03
#define PROTOCOL_TYPE_BT		0x04

#pragma warning(disable:4200)


//ADDED by VC-yavey on 2010-06-04	<begin>

// RSS统计汇报
typedef struct _STAT_RSS_REPORT
{
	BYTE	byOpType;
	DWORD	dwClientVersion;					// 客户端版本
	WORD	wFeedNumber;						// 订阅条目数量
	WORD	wStartRssTasks;						// 启动订阅任务数
	WORD	wDownloadedRssTasks;				// 已下载的订阅任务数
	BYTE	byRunningTime;						// 软件单次运行时长 ( half an hour is step )

	_STAT_RSS_REPORT()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
		byOpType = OP_VC_RSS_STAT_REPORT;
	}
} STAT_RSS_REPORT, *LPSTAT_RSS_REPORT;

// 发送的rss统计包
struct STAT_RSS_REPORT_PACKAGE
{
	BYTE			byOpCode;
	STAT_RSS_REPORT report;

	STAT_RSS_REPORT_PACKAGE()
		:
		byOpCode(OP_VC_STAT_HEADER2)
	{
	}
};

//ADDED by VC-yavey on 2010-06-04	<end>

class CUDPSocket;

class CStatForServer
{
public:
	CStatForServer(void);
	~CStatForServer(void);

	enum{RECORD_INTERVAL_MS = 60000};

	void	RecordCurrentRate(float uploadrate, float downloadrate);

	void	GetAverageSpeeds(ULONG &ulUpload, ULONG &ulDownload);
	void	ResetAverageSpeeds();
	void	GetCurrentSpeedLimit(ULONG &ulUpload, ULONG &ulDownload);

protected:
	DWORD	m_dwLastRecordTime;
	ULONG	m_ulRecordTimes_Upload;
	float	m_fAverageUploadSpeed;
	ULONG	m_ulRecordTimes_Download;
	float	m_fAverageDownloadSpeed;
public:
	//ADDED by VC-yavey on 2010-06-04	<begin>
	struct RssSataInfo 
	{
		WORD	wStartRssTasks;						// 启动任务数
		WORD	wDownloadedRssTasks;				// 已下载的任务数

		RssSataInfo()
		{
			Reset();
		}

		void Reset()
		{
			memset(this, 0, sizeof(*this));
		}
	} m_statRssInfo;
	//ADDED by VC-yavey on 2010-06-04	<end>	

protected:
	BOOL	IsAnyTaskRunning();

private:
	//ADDED by VC-yavey on 2010-06-04	<begin>

	// 发送rss统计数据包
	void	SendRssStatReport(CUDPSocket & udpSocket, uint32 uIp, uint16 uPort);
	//ADDED by VC-yavey on 2010-06-04	<end>
};

extern CStatForServer	theStatForServer;

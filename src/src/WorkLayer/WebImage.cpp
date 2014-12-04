#include "StdAfx.h"
#include "WebImage.h"

#include "StringConversion.h"
#include "GlobalVariable.h"

CWebImage::CWebImage(const CString& strImageUrl, IPosterDownloadNotify & notify)
{
	m_strImageUrl	=	strImageUrl;
	m_pWebImageReqSocket = NULL;
	m_pImgPoster	= NULL;
	m_pNotify = &notify;
	m_pAttachData = NULL;
}

CWebImage::~CWebImage(void)
{
}

void	CWebImage::ResolveResult(int nCode)
{
	m_pNotify->OnNotifyPosterDownload(*this, nCode,m_pImgPoster);
}

bool	 CWebImage::GetPoster()
{
	if (ParsURL())
	{
		if ( LoadImageFromLocal() != NULL )
		{
			ResolveResult(1);
			return true;
		}
		else
		{//get webImage
			StartGetWebImage();
		}
	}

	return false;
}

bool	CWebImage::ParsURL()
{
	if (!m_strImageUrl.IsEmpty())
	{
		if ( m_strImageUrl.Find('/') > 0 && m_strImageUrl.Find('-') > 0 && m_strImageUrl.Find('.') > 0 )
		{
			m_strServer = m_strImageUrl.Mid(m_strImageUrl.Find('-')+1,1);
			int nStart = m_strImageUrl.Find(_T("com/"))+4; int nEnd = m_strImageUrl.Find('(');
			if ( nStart>0 && nEnd>nStart )
				m_strHash = m_strImageUrl.Mid(nStart,nEnd-nStart);

			if (!m_strServer.IsEmpty() && !m_strHash.IsEmpty())
				return true;
		}
	}

	return false;
}

CString	 CWebImage::GetLocalImgUrl()
{
	if (m_strServer.IsEmpty() || m_strHash.IsEmpty())
		return _T("");

	CString strImageFileName;
	strImageFileName.Format(_T("%s.%s"),m_strServer,m_strHash);
	strImageFileName = thePrefs.GetMuleDirectory(EMULE_POSTER,true) + strImageFileName;

	if (!PathFileExists( thePrefs.GetMuleDirectory(EMULE_POSTER) ))
	{
		::CreateDirectory(thePrefs.GetMuleDirectory(EMULE_POSTER) ,NULL);
	}

	return	strImageFileName;
}

CxImage* CWebImage::LoadImageFromLocal()
{
	CString ImagePath = GetLocalImgUrl();

	if (ImagePath.IsEmpty())
		return NULL;

	CxImage* pImage = new CxImage();
	if(pImage->Load(ImagePath,CXIMAGE_FORMAT_JPG))
	{
		m_pImgPoster = pImage;
	}
	else
	{
		delete pImage;
		pImage = NULL;
	}

	return pImage;
}

bool CWebImage::StartGetWebImage()
{
	m_pWebImageReqSocket	=	new CWebImageReqSocket;

	return m_pWebImageReqSocket->SendRequest(this);
}

bool CWebImage::Stop( )
{
	return true;
}


CWebImageReqSocket::CWebImageReqSocket()
{
	m_pWebImage = NULL;
	m_pBuffer = NULL;
	m_nMaxSize = m_nLength = 0;
}

CWebImageReqSocket::~CWebImageReqSocket()
{
	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}
}

CStringA	CWebImageReqSocket::GetServer()
{
	CStringA strServer;
	strServer.Format("image-%s.verycd.com", m_pWebImage->m_strServer);
	return	strServer;
}

bool CWebImageReqSocket::SendRequest(CWebImage* pWebImage)
{
	m_pWebImage = pWebImage;

	if (!Create())
	{
		m_pWebImage->ResolveResult(0);
		return false;
	}

	CString strSearchUrl;
	strSearchUrl.Format(_T("http://image-%s.verycd.com/%s(100x100)/thumb.jpg"),pWebImage->m_strServer,pWebImage->m_strHash);
	m_szGetImageURL = CStringA(strSearchUrl);
	Connect(GetServer(), 80);
	return true;
}

void CWebImageReqSocket::OnConnect(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		if (thePrefs.GetVerbose())
		{
			AddDebugLogLine(false, _T("Get WebImage，但连接%s返回失败。"),CString(GetServer()));
		}
		m_pWebImage->ResolveResult(0);
		return;
	}

	CStringA strHttpRequest;
	strHttpRequest.AppendFormat("GET %s HTTP/1.0\r\n",m_szGetImageURL);
	strHttpRequest.AppendFormat("Host: %s\r\n", GetServer());
	strHttpRequest.AppendFormat("Accept: */*\r\n");
	strHttpRequest.AppendFormat("Accept-Encoding: none\r\n");
	strHttpRequest.Append("\r\n");

	if (thePrefs.GetVerbose())
	{
		AddDebugLogLine(false, _T("与服务器 %s 连接成功，准备发送:"), CString(GetServer()));	
	}

	CRawPacket* pHttpPacket = new CRawPacket(strHttpRequest);
	SendPacket(pHttpPacket);
	SetHttpState(HttpStateRecvExpected);
}

void CWebImageReqSocket::DataReceived(const BYTE* pucData, UINT uSize)
{
	CHttpClientReqSocket::DataReceived(pucData, uSize);
}

bool CWebImageReqSocket::ProcessHttpResponse()
{
	int iMajorVer, iMinorVer;
	int iResponseCode;
	char szResponsePhrase[1024];
	sscanf(m_astrHttpHeaders[0], "HTTP/%d.%d %d %s", &iMajorVer, &iMinorVer, &iResponseCode, szResponsePhrase);

	if (thePrefs.GetVerbose())
		AddDebugLogLine(false, _T("Receive search from server (http response code = %d)"), iResponseCode);

	if (200 != iResponseCode)
		return false;

	return true;
}

bool CWebImageReqSocket::ProcessHttpResponseBody(const BYTE* pucData, UINT size)
{
	if (size > m_nMaxSize - m_nLength)
	{
		int n = max(size,10240);
		int m = n + m_nMaxSize;
		BYTE * pData = (LPBYTE)malloc(m * sizeof(BYTE));
		if (m_pBuffer)
		{
			memcpy(pData,m_pBuffer,m_nLength);
			free(m_pBuffer);
		}

		memcpy(pData + m_nLength,pucData,size);
		m_nMaxSize = m;
		m_nLength += size;
		m_pBuffer = pData;
	}
	else
	{
		if (m_pBuffer)
		{
			memcpy(m_pBuffer + m_nLength,pucData,size);
			m_nLength += size;
		}
	}

	return true;
}

void CWebImageReqSocket::OnClose(int nErrorCode)
{
	LPBYTE pucData = m_pBuffer;
	int size = m_nLength;
 
	if (0 == nErrorCode && pucData && size > 0)
	{
		CxImage* pImage = new CxImage(pucData,size,CXIMAGE_FORMAT_JPG);
		CString ImagePath = m_pWebImage->GetLocalImgUrl();

		pImage->Save(ImagePath, CXIMAGE_FORMAT_JPG);
		m_pWebImage->m_pImgPoster	=	pImage;
	}

	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}

	if (0 == nErrorCode && pucData && size > 0)
		m_pWebImage->ResolveResult(1);
	else
		m_pWebImage->ResolveResult(0);
}
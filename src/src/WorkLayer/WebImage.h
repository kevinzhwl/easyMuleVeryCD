#ifndef _H_WEBIMAGE_H_
#define _H_WEBIMAGE_H_

#include "../zlib/zlib.h"
#include "ListenSocket.h"
#include "HttpClientReqSocket.h"

#include "../CxImage/xImage.h"

class CWebImage;

class CWebImageReqSocket : public CHttpClientReqSocket
{
public:
	CStringA	m_szGetImageURL;
	CWebImage* m_pWebImage;

	BYTE			*m_pBuffer;
	int			m_nMaxSize;
	int			m_nLength;

public:
	CWebImageReqSocket();
	virtual ~CWebImageReqSocket();
	bool		SendRequest(CWebImage* pWebImage);
	CStringA	GetServer();


protected:
	virtual void	OnConnect(int nErrorCode);
	virtual void	OnClose(int nErrorCode);
	virtual void	DataReceived(const BYTE* pucData, UINT uSize);
	virtual bool	ProcessHttpResponse();
	virtual bool	ProcessHttpResponseBody(const BYTE* pucData, UINT size);
};


/**
 * @brief 用于海报获取完成的通知接口
 */
struct IPosterDownloadNotify
{
	/// 用于海报下载完成的通知接口
	virtual void OnNotifyPosterDownload(CWebImage & webImage, const int & nCode, CxImage* pImage) = 0;
};

class CWebImage
{
public:
	CWebImage( const CString& strImageUrl, IPosterDownloadNotify & notify );
	~CWebImage(void);

public:
	bool GetPoster();
	CString		GetLocalImgUrl();
	void	ResolveResult(int nCode);

private:
	CxImage* LoadImageFromLocal();
	bool	ParsURL();
	bool StartGetWebImage();
	bool Stop();

public:
	CxImage*	m_pImgPoster;
	CString m_strImageUrl;
	CString m_strServer;
	CString m_strHash;
	void * m_pAttachData;

private:
	IPosterDownloadNotify* m_pNotify;
	CWebImageReqSocket* m_pWebImageReqSocket;
};

#endif //_H_WEBIMAGE_H_

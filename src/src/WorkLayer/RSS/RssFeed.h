/*
 * $Id: RssFeed.h 20761 2010-11-09 01:54:12Z gaoyuan $
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

#include <afxstr.h>
#include <atltime.h>
#include <map>
#include <vector>
#include <set>

class TiXmlElement;
class CRssFeedList;
class CRssFilter;
class CPartFile;
class CKnownFile;
class CED2KFileLink;
class CRssFeedManager;
class CxImage;
class CWebImage;
struct IPosterDownloadNotify;
class CRssFeedBase;

/// VC订阅集合
typedef std::set<CRssFeedBase*>		CRssFeedSet;

/// 分类信息结构
struct FeedCatalog
{
	static const UINT INVALID_CATALOG_ID = 0L;					///< 无效的catalog id值

	/// 子类集合
	typedef std::set<FeedCatalog*>	Childs;

	unsigned int	m_uId;			///< 分类id
	CString			m_strName;		///< 分类名
	CRssFeedSet		m_setFeeds;		///< 该分类下的所有订阅
	Childs			m_setChilds;	///< 该分类下的所有子分类
	FeedCatalog *	m_pParent;		///< 父类catalog

	/// 构造函数
	FeedCatalog()
		:
		m_uId(INVALID_CATALOG_ID),
		m_pParent(NULL)
	{
	}

	/// 是否叶子节点
	BOOL IsLeaf() const
	{
		// 目前只有2级
		return ( m_pParent != NULL );
	}

	/// 是否顶级节点
	BOOL IsTop() const
	{
		// 目前只有2级
		return ( m_pParent == NULL );
	}

	/// 取得该分类下所有订阅的数目
	unsigned int GetFeedCount() const
	{
		unsigned int iResult = static_cast<unsigned int>(this->m_setFeeds.size());
		for ( Childs::const_iterator it = m_setChilds.begin();
			  it != m_setChilds.end();
			  ++it
			)
		{
			ASSERT(*it != NULL);
			iResult += static_cast<unsigned int>((*it)->m_setFeeds.size());
		}
		return iResult;
	}

	/// 取得第一个分类
	CRssFeedBase * GetFirstFeed()
	{
		if ( !m_setFeeds.empty() )
		{
			return *m_setFeeds.begin();
		}

		for ( Childs::iterator it = m_setChilds.begin();
			  it != m_setChilds.end();
			  ++it
			)
		{
			FeedCatalog * pCatlog = *it;
			ASSERT(pCatlog != NULL);
			if ( !pCatlog->m_setFeeds.empty() )
			{
				return *pCatlog->m_setFeeds.begin();
			}
		}

		return NULL;
	}
};

/// VC分类信息集合
class CFeedCatalogs : public std::map<unsigned int, FeedCatalog>
{
private:
	CFeedCatalogs() {}
	~CFeedCatalogs() {}

public:
	/// 返回唯一的订阅类别信息集合
	static CFeedCatalogs & GetInstance()
	{
		static CFeedCatalogs s_mapCatalogs;
		return s_mapCatalogs;
	}

	/// 查找类别
	static CFeedCatalogs::iterator Find(unsigned int uId)
	{
		return GetInstance().find(uId);
	}

	/// 返回集合中第一个元素迭代器
	static CFeedCatalogs::iterator GetBegin()
	{
		return GetInstance().begin();
	}

	/// 返回集合末尾
	static CFeedCatalogs::iterator GetEnd()
	{
		return GetInstance().end();
	}

};

/**
 * @brief 代表某个订阅，它负责管理和维护订阅条目
 */
class CRssFeedBase
{
public:
	static const UINT UNDEFINED_VALUE = UINT(-1L);						///< 未定义的值
	static const UINT MANUAL_REFRESH_VALUE = UINT(-2L);					///< 手动刷新订阅

private:
	static const int NEW_PUBDATE_OVERDUE_SECOND	= 14 * 24 * 60 * 60;	///< 14天后不再认为是新发布资源

public:

	/// 订阅状态
	enum EFeedState
	{
		fsNormal		= 0x0001L,	///< 普通状态
		fsDisable		= 0x0002L,	///< 订阅已禁用
		fsRefresh		= 0x0004L,	///< xml文件下载中
	};

	/// 订阅条目状态
	enum EItemState
	{
		isRss = 0,		///< 未订阅
		isDownloading,	///< 已订阅，partfile下载中
		isDownloaded,	///< 已订阅，partfile下载完成
		isPaused,		///< 已订阅，partfile已暂停
		isStopped,		///< 已订阅，partfile已停止
		isHistory,		///< 已订阅，但partfile已经被删除

		isMaxState,		///< 条目状态枚举值个数
	};

	/// 代表某个订阅条目
	struct Item
	{
		//static const int COMPLETE_PARTFILE = -1;

		Item()
		{
			Clear();
		}

		//bool operator<(const Item & item) const
		//{
		//	return this->m_strGuid < item.m_strGuid;
		//}

		/// 指示当前条目是否可被下载
		bool CanDownload() const
		{
			switch ( GetState() )
			{
				case CRssFeedBase::isDownloading:	// 已订阅，partfile下载中
				case CRssFeedBase::isDownloaded:	// 已订阅，partfile下载完成
				case CRssFeedBase::isPaused:		// 已订阅，partfile已暂停
				case CRssFeedBase::isStopped:		// 已订阅，partfile已停止
					return false;

				case CRssFeedBase::isRss:			// 未订阅
				case CRssFeedBase::isHistory:		// 已订阅，但partfile已经被删除
				default:
					return true;
			}
		}

		/// 清除条目内容
		void Clear()
		{
			m_pFeed = NULL;
			m_pAttachItem = NULL;
			m_strTitle.Empty();
			m_strLink.Empty();
			m_strDescription.Empty();
			m_strCategory.Empty();
			m_strGuid.Empty();
			m_strEnclosure.Empty();
			m_strEnclosureType.Empty();
			m_strFilename.Empty();
			m_timePubDate = 0;
			m_uEnclosureLength = 0;
			m_bIsHistory = false;
			m_bIsNewPub = false;
			m_pPartFile = NULL;
		}

		/// 取得feed项目状态
		EItemState GetState() const;

		/// 设置partfile
		void SetPartFile(CPartFile * pPartFile)
		{
			m_pPartFile = pPartFile;
		}

		/// partfile将被删除时调用
		void OnDeletePartFile();

		/// 取得partfile
		CPartFile * GetPartFile() const
		{
			return m_pPartFile;
		}		

		/// 获取share file
		CKnownFile * GetShareFile() const
		{
			return CRssFeedBase::GetShareFile(m_strEnclosure);
		}

		/// 是否当前最新发布的资源
		bool IsLastPubItem() const;
		
		/// 是否当前最新发布的影片资源
		bool IsLastPubFilmItem() const;

		/// 是否为字幕
		bool IsSubtitle() const;

		/// 是否小型文件
		bool IsSmallFile() const
		{
			return m_uEnclosureLength > 0 && m_uEnclosureLength <= 10 * 1024 * 1024;
		}

		/// 返回下载完成时间
		void GetDownloadedTime(CTime & timeDownloaded) const;

		/// 重置下载已完成时间
		void ResetDownloadedTime()
		{
			m_timeDownloaded = 0;
		}

		CRssFeedBase *	m_pFeed;				///< 条目所属的订阅
		Item *			m_pAttachItem;			///< 条目所关联的字幕或影片
		CString			m_strTitle;				///< 条目标题
		CString			m_strLink;				///< 条目url link
		CString			m_strDescription;		///< 条目描述内容
		CString			m_strCategory;			///< 条目类别
		CString			m_strGuid;				///< 条目guid
		CString			m_strEnclosure;			///< 条目的下载url
		CString			m_strEnclosureType;		///< 条目资源类型
		CString			m_strFilename;			///< 条目文件名
		CTime			m_timePubDate;			///< 条目发布日期
		uint64			m_uEnclosureLength;		///< 条目资源长度
		bool			m_bIsHistory;			///< 条目是否曾经被下载过
		bool			m_bIsNewPub;			///< 条目是否为新发布的资源

	private:
		CPartFile *	m_pPartFile;			///< 条目关联的partfile

		mutable CTime	m_timeDownloaded;	///< 条目下载完成时间
	};

	typedef std::map<CString, Item> ItemList;				///< 条目列表
	typedef ItemList::const_iterator ItemConstIterator;		///< 只读条目迭代器
	typedef ItemList::iterator ItemIterator;				///< 可读写条目迭代器
	typedef std::vector<Item *>	ItemArray;					///< 条目数组
	typedef bool (*CompareItemFunc)(const Item *, const Item *);	///< 条目比较函数

public:
	CRssFeedBase();
	CRssFeedBase(const CString & strFeedUrl);
	virtual ~CRssFeedBase(void);

private:
	void Init();

public:
	/// 订阅比较函数
	bool operator<(const CRssFeedBase & feed) const
	{
		return this->m_strFeedUrl < feed.m_strFeedUrl;
	}

	CRssFeedBase& operator = (const CRssFeedBase& feed);

	/// 返回订阅是否被更新过
	bool IsUpdated() const
	{
		return (m_tmLastUpdate != 0);
	}

	/// 是否存在正在下载的项目
	bool ExistDownloadingItem() const;

	bool ExistCanResumeItem() const;
	bool ExistUnloadItem() const;

	/// 是否过期（过期后需要更新）
	bool IsOverdue(time_t tmOverdueSec) const;

	/// 刷新feed
	bool Refresh(CRssFeedManager * pManager, const char * lpszUrlStream, const CRssFeedList * pHistoryList);

	/// 清空全部
	void Clear();

	/// 查找一个feed item
	const Item * FindItem(const CString & strGuid) const;

	/// 查找一个feed item
	Item * FindItem(const CString & strGuid);

	//// 查找一个feed item
	//const Item * FindItem(const CPartFile * pPartFile) const;

	/// 查找一个feed item
	Item * FindItem(CRssFeedManager * pManager, const CPartFile * pPartFile);

	/// 添加一个feed item
	Item & AddFeedItem(const CString & strGuid);

	/// 添加一个feed item
	Item & AddFeedItem(const Item & item);

	/// 返回项目列表头
	ItemConstIterator GetItemIterBegin() const
	{
		return m_listItem.begin();
	}

	/// 返回项目列表尾
	ItemConstIterator GetItemIterEnd() const
	{
		return m_listItem.end();
	}

	/// 返回项目列表头
	ItemIterator GetItemIterBegin()
	{
		return m_listItem.begin();
	}

	/// 返回项目列表尾
	ItemIterator GetItemIterEnd()
	{
		return m_listItem.end();
	}

	/// 返回feed item数目
	size_t GetItemCount() const
	{
		return m_listItem.size();
	}

	/// 当前feed item是否为空
	bool IsEmpty() const
	{
		return m_listItem.empty();
	}

	/// 删除一个feed item
	void DeleteItem(const CString & strGuid)
	{
		m_listItem.erase(strGuid);
	}

	/// 返回可显示名字
	const CString & GetDisplayName() const
	{
		const CString & strDisplayName = (m_strAlias.IsEmpty() ? m_strTitle : m_strAlias);
		return ( strDisplayName.IsEmpty() ? m_strFeedUrl : strDisplayName );
	}

	/// 得到按照指定顺序排序的资源项目
	void GetOrderedItems(ItemArray & items, CompareItemFunc func);

	/// 取得新发布的项目数
	unsigned int GetNewPubCount() const;

	/// 取得RSS订阅合集size
	uint64 GetRssTotalSize() const;

	/// 取得RSS订阅已完成下载影片数（影片总数）
	unsigned int GetDownloadedFilmCount(unsigned int * pFilmCount) const;

	/// 取得新的已完成下载数
	unsigned int GetNewDownloadedCount() const;

	/// 设置最新的已完成时间
	void SetNewDownloadedTime();

	/// 更新间隔值是否定义
	bool UpdateIntervalIsDefined() const
	{
		return m_uUpdateInterval != UNDEFINED_VALUE;
	}

	/// 是否手动刷新订阅
	bool IsManualRefresh() const
	{
		return m_uUpdateInterval == MANUAL_REFRESH_VALUE;
	}

	//// 设置为手动刷新
	//void SetManualRefresh()
	//{
	//	m_uUpdateInterval = MANUAL_REFRESH_VALUE;
	//}

	/// 读取更新时间间隔
	UINT GetUpdateInterval() const
	{
		return m_uUpdateInterval;
	}

	/// 设置更新时间间隔
	void SetUpdateInterval(UINT uUpdateInterval)
	{
		m_uUpdateInterval = uUpdateInterval;
	}

	/// 自动下载值是否定义
	bool AutoDownloadIsDefined() const
	{
		return m_uAutoDownload != UNDEFINED_VALUE;
	}

	/// 取得是否自动下载
	UINT GetAutoDownload() const
	{
		return m_uAutoDownload;
	}

	/// 设置自动下载
	void SetAutoDownload(UINT uAutoDownload)
	{
		m_uAutoDownload = uAutoDownload;
	}


private:
	/// 解析一个item
	Item * ParseItem(CRssFeedManager * pManager, TiXmlElement & elItem, bool bIsUtf, const CRssFeedList * pHistoryList);

	/// 解析频道
	void ParseChannel(TiXmlElement & elChannel, bool bIsUtf, bool bAddToFeedCatalogs);

public:
	unsigned int	m_uCatalogId;		///< 所属类别ID，由catalog标记产生
	CString			m_strFeedUrl;		///< feed url
	CString			m_strAlias;			///< 别名
	CString			m_strTitle;			///< 标题
	CString			m_strCategory;		///< 订阅类别，由Category标记产生
	CString			m_strDescription;	///< 描述信息
	CString			m_strSaveDir;		///< 保存目录
	CTime			m_timeLastDownload;	///< 最后下载的资源发布日期
	CTime			m_timeLastPub;		///< 当前最新资源的发布日期
	CTime			m_timeLastFilmPub;	///< 当前最新影片的发布日期
	CTime			m_timeDownloaded;	///< 记住资源已下载的时间（处理已完成数）
	int				m_iState;			///< 状态
	time_t			m_tmLastUpdate;		///< 最近更新时间
	CRssFilter *	m_pFilter;			///< 使用的过滤器
	//bool			m_bHasNewComplete;	///< 是否有新完成的项目
	bool			m_bLastUpdateSucc;	///< 最近一次更新是否成功

private:
	ItemList		m_listItem;			///< 资源项目列表
	UINT			m_uUpdateInterval;	///< 更新间隔
	UINT			m_uAutoDownload;	///< 更新后是否自动下载订阅

public:

#if defined(_DEBUG)
	static void test();
#endif

public:
	/// 将订阅从订阅类别信息集合中删除
	static void RemoveFeedFromCatalogs(const CRssFeedBase & feed);

	/// 获取partfile
	static CPartFile * GetPartFile(const CString & strUrl);

	/// 获取share file
	static CKnownFile * GetShareFile(const CString & strUrl);

	/// 按项目日期升序比较
	static bool CompareItemWithDateA(const Item * pItem1, const Item * pItem2)
	{
		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if ( pItem1 == NULL || pItem2 == NULL )
		{
			return false;
		}

		return pItem1->m_timePubDate < pItem2->m_timePubDate;
	}

	/// 按项目日期降序比较
	static bool CompareItemWithDateD(const Item * pItem1, const Item * pItem2)
	{
		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if ( pItem1 == NULL || pItem2 == NULL )
		{
			return false;
		}

		return pItem1->m_timePubDate > pItem2->m_timePubDate;
	}

	/// 取得项目文件名
	static CString GetItemFileNameNoExt(const Item & item);

	/// 取得文件名
	static CString GetFileNameNoExt(const CString & strFileName);

private:
	/// 取得最后的写入时间
	static void GetLastWriteTime(CKnownFile & file, CTime & timeLastWrite);

	/// 获取partfile
	static CPartFile * GetPartFile(const CED2KFileLink & link);

	/// 获取share file
	static CKnownFile * GetShareFile(const uchar * pFileHash);


	static CString GetEncodeStr(const char * pXml);
	static CString GetElementText(const char * pText, bool bIsUtf);

	/// 由partfile得到项目状态
	static EItemState GetItemStateFromPartFile(const CPartFile & partfile);

	/// 由描述得到ed2k
	static void GetEd2kInfoFromDesc(Item & item);

	/// 字符串转时间
	static bool ConvertStringToTime(const CString & strTime, CTime & tmReturn);

	/// 从partfile中取得url
	static void GetEnclosureUrl(CString & strEnclosureUrl, const CPartFile * pPartFile);

};

/**
 * @brief 代表某个VeryCD订阅，增加了VeryCD特有的海报
 */
class CRssFeed : public CRssFeedBase
{
public:
	CRssFeed() 
	{
		Init();
	}

	CRssFeed(const CString & strFeedUrl)
		:
		CRssFeedBase(strFeedUrl)
	{
		Init();
	}

	CRssFeed(const CRssFeed & feed)
	{
		Init();
		*this = feed;
	}

	virtual ~CRssFeed(void);

public:
	/// 赋值操作
	CRssFeed & operator= (const CRssFeed & feed);

public:
	/// 尝试获取海报
	bool GetPoster(IPosterDownloadNotify & notify);

public:
	CxImage *		m_pPosterImage;		///< 海报图片
	CWebImage *		m_pWebImage;		///< 用于获取海报

private:
	void Init();

};

/**
 * @brief 维护一个订阅列表
 */
class CRssFeedList : public std::map<CString, CRssFeed>
{
public:
	CRssFeedList()
		//:
		//m_bModified(false)
	{
	}
	virtual ~CRssFeedList(void)
	{
	}

public:
	/// 增加一个feed
	CRssFeed & AddFeed(const CString & strFeedUrl)
	{
		CRssFeed & result = (*this)[strFeedUrl];
		if ( result.m_strFeedUrl.IsEmpty() )
		{
			result.m_strFeedUrl = strFeedUrl;
		}

		//m_bModified = true;
		return result;
	}

	/// 查找一个feed
	CRssFeed * FindFeed(const CString & strFeedUrl)
	{
		CRssFeedList::iterator it = find(strFeedUrl);
		if ( it != end() )
		{
			return &it->second;
		}
		return NULL;
	}

	/// 删除一个feed
	void DeleteFeed(const CString & strFeedUrl)
	{
		erase(strFeedUrl);
		//m_bModified = true;
	}

	//// 添加一个项目
	//void AddItem(const CRssFeedBase::Item & item, const CString & strFeedUrl)
	//{
	//	GetFeed(strFeedUrl).AddItem(item);
	//}

	/// 查找第一个指定guid的feed item
	const CRssFeed::Item * FindFirstItem(const CString & strGuid) const
	{
		for ( CRssFeedList::const_iterator it = this->begin();
			  it != this->end();
			  ++it
			)
		{
			if ( const CRssFeedBase::Item * item = it->second.FindItem(strGuid) )
			{
				return item;
			}
		}
		return NULL;
	}

	/// 根据partfile查找item
	CRssFeed::Item * FindFirstItem(CRssFeedManager * pManager, const CPartFile * pPartFile)
	{
		for ( CRssFeedList::iterator it = this->begin();
			  it != this->end();
			  ++it
			)
		{
			if ( CRssFeedBase::Item * item = it->second.FindItem(pManager, pPartFile) )
			{
				return item;
			}
		}
		return NULL;
	}

	/// 返回feed数目
	size_t GetFeedCount() const
	{
		return this->size();
	}


public:
	//bool	m_bModified;
};


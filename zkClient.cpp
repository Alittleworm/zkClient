#include <stdio.h>
#include <string.h>
#include "zkClient.h"
//#include "logmgr.h"


#ifndef errorln
#define errorln printf
#endif
#ifndef debugln
#define debugln printf
#endif

#ifndef infoln
#define infoln printf
#endif

#ifndef bootln
#define bootln printf
#endif


TZKClient::TZKClient()
{

}

TZKClient::~TZKClient()
{

}

//拆分string
int TZKClient::Split(const string& str, vector<string>& ret_, string sep)
{
	if (str.empty())
	{
		return 0;
	}

	string tmp;
	string::size_type pos_begin = str.find_first_not_of(sep);
	string::size_type comma_pos = 0;

	while (pos_begin != string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret_.push_back(tmp);
			tmp.clear();
		}
	}
	return 0;
}



// 获取指定节点的子节点列表
bool TZKClient::GetChildren(const char *pcNode, TListChildData &vecChild, IWatcher *pWatcher)
{
	if (NULL == pcNode || pcNode[0] == '\0')
	{
		return false;
	}

	const TZKConf &tConf = m_mgrZK.GetConf();
	string strAbsPath = "";
	if (pcNode[0] == '/')
	{
		strAbsPath = pcNode;
	}
	else
	{
		strAbsPath = tConf.m_strZKRoot + "/" + pcNode;
	}

	WatcherFunType funZKEvent = std::bind(&TZKClient::CallBackGetChildEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	// 得到所有的子节点，就是当前所有的FriendServer的IP列表,并且设置变化监控器
	ScopedStringVector strChild;
	//int32_t GetChildren(const std::string &path, ScopedStringVector &strings, std::shared_ptr<WatcherFunType> watcher_fun, Stat *stat = NULL);
	int ret = m_mgrZK.GetChildren(pcNode, strChild, std::make_shared<WatcherFunType>(funZKEvent));
	if (ret != ZOK)
	{
		errorln("for={zk get child error},absPath=%s,parent=%s,err=%s\n", strAbsPath.c_str(), pcNode, zerror(ret));
		return false;
	}
	else
	{
		int iSum = strChild.GetSize();
		for (int i = 0; i < iSum; ++i)
		{
			char *pcData = strChild.GetData(i);
			if (!pcData)
			{
				errorln("for={zk get child one error,key=null},parent=%s,index=%d,sum=%d\n", pcNode, i, iSum);
				continue;
			}
			vecChild.push_back(pcData);
			debugln("for={zk get child one},parent=%s,sum=%d,str=%s\n", pcNode, iSum, pcData ? pcData : "");
		}

		debugln("for={zk get child win},absPath=%s,parent=%s,sum=%zu\n", strAbsPath.c_str(), pcNode, vecChild.size());
	}
	// 保存watcher
	if (m_mapWatcher.find(strAbsPath) == m_mapWatcher.end())
	{
		TNodeData tAdd;
		tAdd.m_iValueSize = 256;
		tAdd.m_pWatcher = pWatcher;
		tAdd.m_strNodeName = pcNode;
		m_mapWatcher[strAbsPath] = tAdd;
	}

	return true;
}

// 获取指定节点值
bool TZKClient::Get(const char *pcNode, string &strValue, IWatcher *pWatcher, int iMaxSize)
{
	strValue.clear();

	if (NULL == pcNode || pcNode[0] == '\0')
	{
		return false;
	}

	const TZKConf &tConf = m_mgrZK.GetConf();
	string strAbsPath = "";
	if (pcNode[0] == '/')
	{
		strAbsPath = pcNode;
	}
	else
	{
		strAbsPath = tConf.m_strZKRoot + "/" + pcNode;
	}

	WatcherFunType funZKEvent = std::bind(&TZKClient::CallBackGetEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	
	char *pcGet = new char[iMaxSize];
	memset(pcGet, 0, iMaxSize);
	int iGetLen = iMaxSize - 1;
	Stat zkState;
	// int32_t Get(const std::string &path, char *buffer, int* buflen, Stat *stat, std::shared_ptr<WatcherFunType> watcher_fun);
	// 从ZK获取上报节点的信息,并且设置变化监控器
	int ret = m_mgrZK.Get(pcNode, pcGet, &iGetLen, &zkState, std::make_shared<WatcherFunType>(funZKEvent));
	if (ret != ZOK)
	{
		errorln("for={zk get  error},absPath=%s,node=%s,err=%s\n", strAbsPath.c_str(), pcNode, zerror(ret));
		delete []pcGet;
		return false;
	}
	else
	{
		strValue = pcGet;
	}

	delete []pcGet;

	debugln("for={zk get win},absPath=%s,node=%s,value=%s,maxSize=%d\n", strAbsPath.c_str(), pcNode, strValue.c_str(), iMaxSize);

	// 保存watcher
	if (m_mapWatcher.find(strAbsPath) == m_mapWatcher.end())
	{
		TNodeData tAdd;
		tAdd.m_iValueSize = iMaxSize;
		tAdd.m_pWatcher = pWatcher;
		tAdd.m_strNodeName = pcNode;
		// 保存watcher
		m_mapWatcher[strAbsPath] = tAdd;
	}

	return true;
}


// 设置指定节点值
bool TZKClient::Set(const char *pcNode, const char *strValue)
{
	if (NULL == pcNode || pcNode[0] == '\0')
	{
		return false;
	}

	int iLen = strlen(strValue);

	Stat zkState;
	// 节点是否存在
	if (m_mgrZK.Exists(pcNode, &zkState) == ZOK)
	{
		int ret = m_mgrZK.Set(pcNode, strValue, iLen, zkState.version);
		if (ret != ZOK)
		{
			errorln("for={zk set node error},node=%s,err=%s\n", pcNode, zerror(ret));
			return false;
		}
		else
		{
			debugln("for={zk set node win},node=%s,value=%s\n", pcNode, strValue);
		}
	}
	else
	{
		errorln("for={zk node no exist},node=%s\n", pcNode);
		// 创建根节点
		int ret = m_mgrZK.Create(pcNode, strValue, iLen, NULL);
		if (ret != ZOK)
		{
			errorln("for={zk create node error},node=%s,err=%s\n", pcNode, zerror(ret));
			return false;
		}
		else
		{
			debugln("for={zk create node and set win},node=%s,value=%s\n", pcNode, strValue);
		}

	}

	return true;
}


// [zk]
bool TZKClient::Init(const char *strParentNode, int iServerId, int iListenPort, const char *pcConfPath)
{
	if (strParentNode[0] == '\0')
	{
		return false;
	}
	if (iListenPort <= 0)
	{
		return false;
	}
	// 初始化zookeeper,设置zk ip列表
	int ret = m_mgrZK.InitFromConf(pcConfPath);
	if (ZOK != ret)
	{
		errorln("for={zk read conf error},err=%s\n", zerror(ret));
		return false;
	}
	char strNodeName[128] = { 0 };
	char strNodeValue[256] = { 0 };

	TZKConf &tConf = m_mgrZK.GetConf();
	tConf.m_strZKParent = strParentNode;

	snprintf(strNodeName, 127, "%d:%s:%d", iServerId, tConf.m_strLocalIP.c_str(), iListenPort);
	snprintf(strNodeValue, 255, "%s:%d", tConf.m_strLocalIP.c_str(), iListenPort);
	return InitZooKeeper(tConf.m_strZKIP.c_str(), tConf.m_strZKRoot.c_str(), strParentNode, strNodeName, strNodeValue);
}

// [zk]
bool TZKClient::InitZooKeeper(const char *pcZKServerIP, const char *pcRoot, const char *pcParentNode, const char *pcNodeName, const char *pcNodeValue)
{
	// 初始化ZooKeeper

	int ret = 0;
	string strRoot = pcRoot;
	string strZKParent = pcParentNode;
	string strZKServerIP = pcZKServerIP;

	infoln("for={begin init zookeeper},rootNode=%s,parentNode=%s,ip=%s,nodeName=%s,nodeValue=%s\n", pcRoot, pcParentNode, pcZKServerIP, pcNodeName, pcNodeValue);

	// 连接zookeeper
	WatcherFunType funZKEvent = std::bind(&TZKClient::CallBackZKEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	ret = m_mgrZK.ConnectByConf(std::make_shared<WatcherFunType>(funZKEvent));
	if (ret != ZOK)
	{
		errorln("for={zk connect  error},err=%s\n", zerror(ret));
		return false;
	}

	char acNode[256] = { 0 };
	snprintf(acNode, 255, "%s/%s", strZKParent.c_str(), pcNodeName);
	m_strZKNode = strRoot + "/" + acNode;
	m_strZKValue = pcNodeValue;
	//StringCompletionFunType funCallBack = std::bind(&TZKClient::CallBackRegToZK, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	// 根节点是否存在
	if (m_mgrZK.Exists(strZKParent.c_str(), NULL) == ZOK)
	{
		infoln("for={zk parent node exist},parentNode=%s\n", strZKParent.c_str());
	}
	else
	{
		errorln("for={zk parent node no exist},parentNode=%s\n", strZKParent.c_str());
		// 创建根节点
		ret = m_mgrZK.Create(strZKParent.c_str(), "", strlen(""),  NULL);
		if (ret != ZOK)
		{
			errorln("for={zk create parent node error},parentNode=%s,err=%s\n", strZKParent.c_str(), zerror(ret));
			return false;
		}
		else
		{
			infoln("for={zk create parent node win},parentNode=%s\n", strZKParent.c_str());
		}
	}


	////////////////// [get, set]示例代码:///////////////////////////
	// 获取父节点的信息，并且设置父节点的值，主要用作get ,set示例.
	char acGet[256] = { 0 };
	int iGetLen = 255;
	Stat zkState;
	ret = m_mgrZK.Get(strZKParent.c_str(), acGet, &iGetLen, &zkState, std::make_shared<WatcherFunType>(funZKEvent));
	if (ret != ZOK)
	{
		errorln("for={zk get parent node error},parentNode=%s,err=%s\n", strZKParent.c_str(), zerror(ret));
		return false;
	}
	else
	{
		infoln("for={zk get parent node},parentNode=%s,str=%s\n", strZKParent.c_str(), acGet);
	}
	ret = m_mgrZK.Set(strZKParent.c_str(), "server ip list", strlen("server ip list"), zkState.version);
	if (ret != ZOK)
	{
		errorln("for={zk set node error},parentNode=%s,err=%s\n", strZKParent.c_str(), zerror(ret));
	}
	else
	{
		infoln("for={zk set node win},parentNode=%s,value=%s\n", strZKParent.c_str(), "server ip list");
	}
	///////////////////////////////////////////////////////////////////

	if (m_mgrZK.Exists(acNode, NULL) == ZOK)
	{
		infoln("for={zk create node exist},node=%s\n", acNode);
	}
	else
	{
		// 永久节点
		//if (m_mgrZK.Create(acNode, acAddr, strlen(acAddr), NULL) != ZOK)
		// 创建临时节点，上报自身的IP，Port信息
		errorln("for={zk create node no exist},node=%s\n", acNode);
		ret = m_mgrZK.Create(acNode, pcNodeValue, strlen(pcNodeValue), NULL, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, false);
		if (ret != ZOK)
		{
			errorln("for={zk create node error},node=%s,err=%s\n", acNode, zerror(ret));
			return false;
		}
		else
		{
			infoln("for={zk create node win},node=%s\n", acNode);
		}
	}

	
	memset(acGet, 0, sizeof(acGet));
	iGetLen = sizeof(acGet) - 1;
	// int32_t Get(const std::string &path, char *buffer, int* buflen, Stat *stat, std::shared_ptr<WatcherFunType> watcher_fun);
	// 从ZK获取上报节点的信息,并且设置变化监控器
	ret = m_mgrZK.Get(acNode, acGet, &iGetLen, &zkState, std::make_shared<WatcherFunType>(funZKEvent));
	if (ret != ZOK)
	{
		errorln("for={zk get node error},node=%s,err=%s\n", acNode, zerror(ret));
		return false;
	}
	else
	{
		infoln("for={zk get node},node=%s,str=%s\n", acNode, acGet);
	}


	// 得到所有的子节点，就是当前所有的FriendServer的IP列表,并且设置变化监控器
	ScopedStringVector strChild;
	//int32_t GetChildren(const std::string &path, ScopedStringVector &strings, std::shared_ptr<WatcherFunType> watcher_fun, Stat *stat = NULL);
	ret = m_mgrZK.GetChildren(strZKParent.c_str(), strChild, std::make_shared<WatcherFunType>(funZKEvent));
	if (ret != ZOK)
	{
		errorln("for={zk get child error},parent=%s,err=%s\n", strZKParent.c_str(), zerror(ret));
	}
	else
	{
		char *pcData = strChild.GetData(0);
		infoln("for={zk get child},parent=%s,sum=%d,str=%s\n", strZKParent.c_str(), strChild.GetSize(), pcData ? pcData : "");
	}

	//int32_t Set(const std::string &path, const char *buffer, int buflen, int version, Stat *stat = NULL);
	//int32_t Set(const std::string &path, const std::string &buffer, int version, Stat *stat = NULL);


	infoln("end zk");
	return true;
}

// [zk]
bool TZKClient::CallBackGetChildEvent(ZookeeperManager &zookeeper_manager, int type, int state, const char *path)
{
	const TZKConf &tConf = m_mgrZK.GetConf();

	infoln("for={zk getChildren is changed},type=%d,state=%d,path=%s,root=%s\n", type, state, path ? path : "", tConf.m_strZKRoot.c_str());
	// ZK连接会话事件
	if (path && state == ZOO_CONNECTED_STATE)
	{
		
		// 如果被删除掉
		if (type == ZOO_DELETED_EVENT)
		{
			infoln("for={zk getChildren node is deleted}, path=%s\n", path ? path : "");
		}

		TNodeData *pNode = &m_mapWatcher[path];
		TListChildData vecChild;
		GetChildren(pNode->m_strNodeName.c_str(), vecChild, pNode->m_pWatcher);

		if (pNode->m_pWatcher)
		{
			pNode->m_pWatcher->OnChildChange(pNode->m_strNodeName.c_str(), vecChild);
		}
	}

	return true;
}

// [zk]
bool TZKClient::CallBackGetEvent(ZookeeperManager &zookeeper_manager, int type, int state, const char *path)
{
	const TZKConf &tConf = m_mgrZK.GetConf();

	infoln("for={zk get is changed},type=%d,state=%d,path=%s,root=%s\n", type, state, path ? path : "", tConf.m_strZKRoot.c_str());
	// ZK连接会话事件
	if (path && state == ZOO_CONNECTED_STATE)
	{

		// 如果被删除掉
		if (type == ZOO_DELETED_EVENT)
		{
			infoln("for={zk get node is deleted}, path=%s\n", path ? path : "");
		}

		TNodeData *pNode = &m_mapWatcher[path];
		string strValue;
		Get(pNode->m_strNodeName.c_str(), strValue, pNode->m_pWatcher, pNode->m_iValueSize);

		if (pNode->m_pWatcher)
		{
			pNode->m_pWatcher->OnChange(pNode->m_strNodeName.c_str(), strValue);
		}
	}

	return true;
}

// [zk]
bool TZKClient::CallBackZKEvent(ZookeeperManager &zookeeper_manager, int type, int state, const char *path)
{
	const TZKConf &tConf = m_mgrZK.GetConf();
	infoln("for={zk event}, type=%d,state=%d,path=%s,mynode=%s,parent=%s,root=%s\n", type, state, path ? path : "",
		m_strZKNode.c_str(), tConf.m_strZKParent.c_str(), tConf.m_strZKRoot.c_str());

	// ZK连接会话事件
	if (type == ZOO_SESSION_EVENT)
	{
		// ZK连接成功
		if (state == ZOO_CONNECTED_STATE)
		{
			infoln("for={Connected to zookeeper service successfully}\n");
		}
		// ZK会话过期
		else if (state == ZOO_EXPIRED_SESSION_STATE)
		{
			infoln("for={Zookeeper session expired}\n");
		}
	}

	if (path && m_strZKNode == path && state == ZOO_CONNECTED_STATE)
	{
		// 如果创建的临时节点，被删除掉，需要重新创建
		if (type == ZOO_DELETED_EVENT)
		{
			infoln("for={zk node is deleted}, path=%s\n", m_strZKNode.c_str());
			// 节点被删除了，重新注册一下吧
			if (m_mgrZK.Create(m_strZKNode.c_str(), m_strZKValue.c_str(), m_strZKValue.size(), NULL, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, false) != ZOK)
			{
				errorln("for={zk create node error}\n");
			}
			else
			{
				infoln("for={zk create node win}\n");
			}

		}
		// 任何变化，都需要重新注册Watch
		// int32_t Get(const std::string &path, char *buffer, int* buflen, Stat *stat, std::shared_ptr<WatcherFunType> watcher_fun);
		// 从ZK获取上报节点的信息,并且设置变化监控器
		WatcherFunType funZKEvent = std::bind(&TZKClient::CallBackZKEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		char acGet[128] = { 0 };
		int iGetLen = 127;
		if (m_mgrZK.Get(m_strZKNode.c_str(), acGet, &iGetLen, NULL, std::make_shared<WatcherFunType>(funZKEvent)) != ZOK)
		{
			errorln("for={zk redo get node error}\n");
		}
		else
		{
			infoln("for={zk redo get node},str=%s\n", acGet);
		}

	}

	return true;
}

// [[zk]]
void TZKClient::CallBackRegToZK(ZookeeperManager &zookeeper_manager, int rc, const char *value)
{
	infoln("for={create win}, rc=%d,str=%s\n", rc, value);
}

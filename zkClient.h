#ifndef __ZKCLIENT__H__
#define __ZKCLIENT__H__
/*
功能: ZooKeeper客户端功能类,提供上报自身IP和端口功能,读取zk公共配置文件
作者: 王国庆
日期: 2017-08-30

struct TZKConf
{
TZKConf()
{
m_iRecvTimeout = 1000;
m_iConnectTimeout = 1000;

}
string  m_strZKIP;				// zookeeper Server IP列表
string  m_strZKRoot;			// zookeeper根节点名
int		m_iRecvTimeout;			// zookeeper接收包超时时间[毫秒]
int		m_iConnectTimeout;		// zookeeper连接超时时间[毫秒]
string  m_strZKParent;			// zookeeper 父节点名
string  m_strLocalIP;			// 本地IP
};

// 从INI文件读取配置

../conf/zookeeper.ini
[main]
zkIP=192.168.174.128:2181;192.168.174.129:2181
root=/Mahjong
recvTime=1000
conTime=1000
localIP=127.0.0.1



*/
#include <string>
#include <map>
//[zk]
#include "CppZookeeper.h"
using namespace zookeeper;

using std::string;
using std::map;
#include <vector>
using std::vector;

typedef vector<string> TListChildData;
// 数据监视器
struct IWatcher
{
	// 当前节点数据更新
	virtual bool OnChange(const char *pcNode, const string &rData) = 0;

	// 当前子节点列表更新
	virtual bool OnChildChange(const char *pcNode, const vector<string> &rData) = 0;
};

class TZKClient
{
public:
	TZKClient();
	virtual ~TZKClient();
	
	//[zk]///////////////////////////zookeeper/////////////////////////////////////////////

	// 初始化zookeeper从conf
	bool Init(const char *strParentNode, int iServerId, int iListenPort, const char *pcConfPath="../conf/zookeeper.ini");
	// 获取指定节点的子节点列表: 可以使用绝对路径[例如: /Mahjong/HallServer]和相对路径[例如:HallServer/1001]
	bool GetChildren(const char *pcNode, TListChildData &vecChild, IWatcher *pWatcher);
	// 获取指定节点值: 可以使用绝对路径[例如: /Mahjong/HallServer]和相对路径[例如:HallServer/1001]
	bool Get(const char *pcNode, string &strValue, IWatcher *pWatcher, int iMaxSize=1024);
	// 设置指定节点值(不存在创建): 可以使用绝对路径[例如: /Mahjong/HallServer]和相对路径[例如:HallServer/1001]
	bool Set(const char *pcNode, const char *strValue);

	// 拆分string
	static int Split(const string& str, vector<string>& ret_, string sep = ",");


	//////////////////////////////////////////////////////////////////////////////////////

private:
	struct TNodeData
	{
		TNodeData()
		{
			m_iValueSize = 1024;
			m_pWatcher = NULL;
		}
		string		m_strNodeName;		// 节点名称
		int			m_iValueSize;		// 返回值最大大小
		IWatcher *  m_pWatcher;			// watcher回调
	};

	// 节点IP上报 watch事件回调
	bool CallBackZKEvent(ZookeeperManager &zookeeper_manager, int type, int state, const char *path);
	// zookeeper 异步API操作完成事件回调，主要是A开头的API: ACreate,AGet,ASet,AGetChildren
	void CallBackRegToZK(ZookeeperManager &zookeeper_manager, int rc, const char *value);
	// 获取节点事件回调
	bool CallBackGetChildEvent(ZookeeperManager &zookeeper_manager, int type, int state, const char *path);
	// 获取节点事件回调
	bool CallBackGetEvent(ZookeeperManager &zookeeper_manager, int type, int state, const char *path);

	// 初始化zookeeper
	bool InitZooKeeper(const char *pcZKServerIP, const char *pcRoot, const char *pcParentNode, const char *pcNodeName, const char *pcNodeValue);
	
	
	// [zk] //////////////////zookeeper变量区//////////////////
	
	
	typedef map<string, TNodeData> TMapWatcher;

	// zookeeper管理器
	ZookeeperManager m_mgrZK;

	// 运行变量
	string			m_strZKNode;			// zookeeper注册的节点名称
	string			m_strZKValue;			// zookeeper注册的节点值

	TMapWatcher		m_mapWatcher;			// watcher词典

	/////////////////////////////////////////////////////////
};


#endif

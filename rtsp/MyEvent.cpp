#include <signal.h>
#include <WinSock2.h>
#include <string>
#include <memory>
#include <algorithm>
#include "MyEvent.h"
#include "event2/buffer.h"
#define RECV_BUF_SIZE 4096
MyEvent *MyEvent::instance = NULL;
MyEvent *MyEvent::getInstance()
{
	if (!instance)
	{
		instance = new MyEvent();
	}
	return instance;
}
MyEvent::MyEvent()
{
	printf("init\n");
	
#ifdef WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif
    base = event_base_new();
    if (!base) 
    {
        printf("Could not initialize libevent!\n");
    }
}

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = (struct event_base *)user_data;
    struct timeval delay = { 2, 0 };
    printf("get signal\n");
    event_base_loopexit(base, &delay);
}

void MyEvent::loop()
{
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

    if (!signal_event || event_add(signal_event, NULL)<0) 
    {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return;
    }

    event_base_dispatch(base);
}
MyEvent::~MyEvent()
{
	printf("~\n");
    event_free(signal_event);
    event_base_free(base);
}

static void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF) 
    {
        printf("Connection closed.\n");
    } 
    else if (events & BEV_EVENT_ERROR) 
    {
        printf("Got an error on the connection: %s\n",
	            strerror(errno));/*XXX win32*/
    }else if (events & BEV_EVENT_CONNECTED) {
        printf("Client has successfully cliented.\n");
		return;
    }
}

static void conn_readcb(bufferevent *bev, void *user_data)
{
	char buf[RECV_BUF_SIZE];
	int n;
	std::string tmpSend("");

	struct evbuffer *input = bufferevent_get_input(bev);
	while((n = evbuffer_remove(input, buf, sizeof(buf) - 1)) > 0)
	{
		buf[n] = 0;
		tmpSend += buf;
	}
	CallBack *funcPtr = (CallBack *)user_data;
	//funcPtr->func(tmpSend);
    (*funcPtr)(tmpSend);
}

struct closure
{
	std::function<void(std::string)> func;
};

bufferevent *MyEvent::connect(const char *host, unsigned int port, void *func)
{
    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
	server_addr.sin_addr.S_un.S_addr = inet_addr(host);

    bufferevent_socket_connect(bev, (sockaddr *)&server_addr, sizeof(server_addr));
    bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, func);
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
	return bev;
}

void MyEvent::sendTo(evutil_socket_t sock, const char *host, UINT32 port, std::string &data)
{
    sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr.S_un.S_addr = inet_addr(host);
    int clientAddrLen = sizeof(sockaddr);
    
    sendto(sock, data.c_str(), data.length(), 0, (sockaddr*)&client_addr, clientAddrLen);
}

void udp_read(evutil_socket_t fd, short what, void *arg)
{
    sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(sockaddr_in));
    int clientLen = sizeof(sockaddr);
    char buf[RECV_BUF_SIZE];
	while(1)
	{
		u_long mode;
		ioctlsocket(fd, FIONREAD, &mode);
		//printf("mode is :%d\n", mode);
		if (mode == 0)
		{
			break;
		}
		int len = recvfrom(fd, buf, RECV_BUF_SIZE - 1, 0, (sockaddr *)&clientAddr, &clientLen);
		//printf("len is :%d\n", len);
		if (len == -1 || len == 0)
		{
			return;
		}
		std::string data(buf, len);
		(*((CallBack *)arg))(data);
	}
}

void MyEvent::detachSock(evutil_socket_t sock)
{
    sock2FreeFunc[sock]();
}

void timeout_cb(evutil_socket_t fd, short what, void *arg)
{
    (*((CallBack *)arg))("");
}

void closeEvent(event *ev)
{
    event_del(ev);
    event_free(ev);
}

event *MyEvent::addTimer(UINT32 sec, void *func)
{
	event *ev;
	//ev = evsignal_new(base, -1, timeout_cb, NULL)
    ev = event_new(base, -1, EV_PERSIST, timeout_cb, func);
	timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = sec;
	tv.tv_usec = 0;
	event_add(ev, &tv);
	return ev;
}

evutil_socket_t MyEvent::bindUdp(UINT32 port, void *func)
{
    event *ev;
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == SOCKET_ERROR)
    {
        printf("create socket failed\n");
        return -1;
    }

    //绑定地址信息  
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    bind(sock, (sockaddr*)&serverAddr, sizeof(sockaddr));
    ev = event_new(base, sock, EV_READ | EV_PERSIST, udp_read, func);
    event_add(ev, NULL);
    sock2FreeFunc.insert(std::pair<evutil_socket_t, FREE_FUNC>(sock,
        [=]() {
        event_del(ev);
        event_free(ev);
        closesocket(sock);
    }));
    return sock;
}
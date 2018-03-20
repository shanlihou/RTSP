#pragma once
#include <functional>
#include <map>
#include "event2/event.h"
#include "event2/bufferevent.h"
class CallBack
{
public:
    std::function<void(std::string)> operator=(std::function<void(std::string)> func)
    {
        this->func = func;
        return func;
    };
    void operator()(std::string data)
    {
        func(data);
    }
	std::function<void(std::string)> func;
};
typedef std::function<void()> FREE_FUNC;

class EventDeleter
{
public:
	void operator() (event *ev)
	{
		event_del(ev);
		event_free(ev);
	};
};
class MyEvent
{
public:
	static MyEvent *getInstance();
	~MyEvent();
	void loop();
	bufferevent *connect(const char *host, unsigned int port, void *func);
    evutil_socket_t bindUdp(UINT32 port, void *func);
    void detachSock(evutil_socket_t sock);
    void sendTo(evutil_socket_t sock, const char *host, UINT32 port, std::string &data);
	event *addTimer(UINT32 sec, void *func);
private:
	MyEvent();
	static MyEvent *instance;
    struct event_base *base;
    struct event *signal_event;
    std::map<evutil_socket_t, FREE_FUNC> sock2FreeFunc;
};
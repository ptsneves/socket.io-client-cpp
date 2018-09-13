//
//  sio_test_sample.cpp
//
//  Created by Melo Yao on 3/24/15.
//

#include "../../src/sio_client.h"

#include <functional>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#ifdef WIN32
#define HIGHLIGHT(__O__) std::cout<<__O__<<std::endl
#define EM(__O__) std::cout<<__O__<<std::endl

#include <stdio.h>
#include <tchar.h>
#define MAIN_FUNC int _tmain(int argc, _TCHAR* argv[])
#else
#define HIGHLIGHT(__O__) std::cout<<"\e[1;31m"<<__O__<<"\e[0m"<<std::endl
#define EM(__O__) std::cout<<"\e[1;30;1m"<<__O__<<"\e[0m"<<std::endl

#define MAIN_FUNC int main(int argc ,const char* args[])
#endif

#ifndef SERVER_ADDRESS
#define SERVER_ADDRESS "127.0.0.1"
#endif

#ifndef NAMESPACE
#define NAMESPACE "/"
#endif

using namespace sio;
using namespace std;
std::mutex _lock;

std::condition_variable_any _cond;
bool connect_finish = false;

class connection_listener
{
    sio::client &handler;

public:
    
    connection_listener(sio::client& h):
    handler(h)
    {
    }
    

    void on_connected()
    {
        _lock.lock();
        _cond.notify_all();
        connect_finish = true;
        std::cout << "connected " << std::endl;
        _lock.unlock();
    }
    void on_close(client::close_reason const& reason)
    {
        std::cout<<"sio closed "<<std::endl;
        exit(0);
    }
    
    void on_fail()
    {
        std::cout<<"sio failed "<<std::endl;
        exit(0);
    }
};

int participants = -1;

socket::ptr current_socket;

void bind_events()
{
	current_socket->on("server_test", sio::socket::event_listener_aux([&](string const& name, message::ptr const& data, bool isAck,message::list &ack_resp)
                       {
                           _lock.lock();
                           EM("Received" << ":"<< data->get_map()["message"]->get_string());
                          _cond.notify_all();
                          _lock.unlock();
                       }));
}

MAIN_FUNC
{
    sio::client h;
    connection_listener l(h);
    h.set_open_listener(std::bind(&connection_listener::on_connected, &l));
    h.set_close_listener(std::bind(&connection_listener::on_close, &l,std::placeholders::_1));
    h.set_fail_listener(std::bind(&connection_listener::on_fail, &l));
    h.connect(SERVER_ADDRESS);
    _lock.lock();
    if(!connect_finish)
    {
        _cond.wait(_lock);
    }
  	current_socket = h.socket(NAMESPACE);
    bind_events();
    _lock.lock();
    current_socket->emit("server_test", std::make_shared<std::string>("Hello"));
    _lock.unlock();
    _lock.lock();
    _cond.wait(_lock);
    h.sync_close();
    h.clear_con_listeners();
	return 0;
}


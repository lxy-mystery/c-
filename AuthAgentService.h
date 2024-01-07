#ifndef __AUTHAGENT_SERVER_H__
#define __AUTHAGENT_SERVER_H__
#include "HardwareService.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "nlohmann/json.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef server::message_ptr message_ptr;
class AuthAgentService
{
public:
  void start();
  void stop();
  static void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg);
private:
  static HardwareService service;
  server  wsserver;
};

#endif

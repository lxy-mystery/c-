#include "pch.h"
#include "AuthAgentService.h"
HardwareService AuthAgentService::service;

void AuthAgentService::start() {
    try {
        // Set logging settings
        wsserver.set_access_channels(websocketpp::log::alevel::all);
        wsserver.clear_access_channels(websocketpp::log::alevel::frame_payload);
        std::cout << "start AuthAgent Service.." << std::endl;
        // Initialize Asio
        wsserver.init_asio();
        std::cout << "init asio ... " << std::endl;
        // Register our message handler
        wsserver.set_message_handler(bind(&on_message, &wsserver, ::_1, ::_2));

        // Listen on port 9002
        wsserver.listen(9002);
        std::cout << "listen in 9002 ..." << std::endl;
        // Start the server accept loop
        wsserver.start_accept();
        std::cout << "accept now ..." << std::endl;
        // Start the ASIO io_service run loop
        wsserver.run();
        std::cout << "auth agent stop ..." << std::endl;
    }
    catch (websocketpp::exception const& e) {
        std::cout << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "other exception" << std::endl;
    }
}

void AuthAgentService::stop()
{
  wsserver.stop();
}

void AuthAgentService::on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
  std::cout << "on_message called with hdl: " << hdl.lock().get()
    << " and message: " << msg->get_payload()
    << std::endl;

  // check for a special command to instruct the server to stop listening so
  // it can be cleanly exited.
  if (msg->get_payload() == "stop-listening") {
    s->stop_listening();
    return;
  }

  try {
    nlohmann::json hardware = {
      { "baseboard", service.getBaseboardSerialNumber() },
      { "bios", service.getBIOSSerialNumber() },
      { "cpu", service.getCPUSerialNumber() },
      { "disks", service.getDiskSerialNumber() },
      { "macs", service.getMACAddress() }
    };
    s->send(hdl, hardware.dump(), msg->get_opcode());
  }
  catch (websocketpp::exception const& e) {
    std::cout << "Echo failed because: "
      << "(" << e.what() << ")" << std::endl;
  }
}

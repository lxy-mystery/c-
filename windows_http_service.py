# 导入所需的模块
import win32serviceutil
import win32service
import win32event
import servicemanager
import socket
import sys
import os

import http.server
import socketserver
import logging
class LogHTTPHandler(http.server.SimpleHTTPRequestHandler):
    log_file = open('F:\deploy\LicodeReleaseService.log', 'a')
    def log_message(self, format, *args):
        self.log_file.write("%s - - [%s] %s\n" %
                            (self.client_address[0],
                             self.log_date_time_string(),
                             format%args))
        self.log_file.flush()

# 定义一个Windows服务类，继承自win32serviceutil.ServiceFramework
class LicodeReleaseService(win32serviceutil.ServiceFramework):
    # 服务名称
    _svc_name_ = 'LicodeReleaseService'
    # 服务显示名称
    _svc_display_name_ = 'LicodeReleaseService'

    # 初始化方法
    def __init__(self, args):
        win32serviceutil.ServiceFramework.__init__(self, args)
        self.hWaitStop = win32event.CreateEvent(None, 0, 0, None)
        self.httpd = socketserver.TCPServer(("", 9437), LogHTTPHandler)
        os.chdir("F:\deploy\licode")
        self.logger = logging.getLogger('LicodeReleaseService')
        self.logger.setLevel(logging.DEBUG)
        handler = logging.FileHandler('F:\deploy\LicodeReleaseService.log')
        handler.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        handler.setFormatter(formatter)
        self.logger.addHandler(handler)
        

    # 服务启动方法
    def SvcDoRun(self):
        servicemanager.LogMsg(servicemanager.EVENTLOG_INFORMATION_TYPE,
                              servicemanager.PYS_SERVICE_STARTED,
                              (self._svc_name_, ''))
        self.main()

    # 服务停止方法
    def SvcStop(self):
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        win32event.SetEvent(self.hWaitStop)
        self.httpd.shutdown()

    # 主要的功能代码
    def main(self):
        try:
            self.logger.info('LicodeReleaseService is starting...')
            self.httpd.serve_forever()
        except Exception as e:
            self.logger.error(e)

# 如果通过命令行直接运行该脚本，则执行以下代码来安装、启动或停止服务
if __name__ == '__main__':
    if len(sys.argv) == 1:
        servicemanager.Initialize()
        servicemanager.PrepareToHostSingle(LicodeReleaseService)
        servicemanager.StartServiceCtrlDispatcher()
    else:
        win32serviceutil.HandleCommandLine(LicodeReleaseService)

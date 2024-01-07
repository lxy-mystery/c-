#ifndef __HARDWARESERVICE_H__
#define __HARDWARESERVICE_H__
#include "WMIService.h"

class HardwareService
{
public:
  std::vector<std::string> getDiskSerialNumber();
  std::string getCPUSerialNumber();
  std::string getBIOSSerialNumber();
  std::string getBaseboardSerialNumber();
  std::vector<std::string> getMACAddress();
private:
  WMIService  service;
};

#endif
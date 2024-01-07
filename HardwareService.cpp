#include "pch.h"
#include "HardwareService.h"

std::vector<std::string> HardwareService::getDiskSerialNumber()
{
  return service.GetPropertyValues(L"Win32_DiskDrive", L"SerialNumber");
}

std::string HardwareService::getCPUSerialNumber()
{
  std::vector<std::string> cpuSerialNumbers = service.GetPropertyValues(L"Win32_Processor", L"ProcessorId");
  if (cpuSerialNumbers.size() == 0) {
    return "";
  }
  return cpuSerialNumbers[0];
}

std::string HardwareService::getBIOSSerialNumber()
{
  std::vector<std::string> biosSerialNumbers = service.GetPropertyValues(L"Win32_BIOS", L"SerialNumber");
  if (biosSerialNumbers.size() == 0) {
    return "";
  }
  return biosSerialNumbers[0];
}

std::string HardwareService::getBaseboardSerialNumber()
{
  std::vector<std::string> baseboardSerialNumbers = service.GetPropertyValues(L"Win32_BaseBoard", L"SerialNumber");
  if (baseboardSerialNumbers.size() == 0) {
    return "";
  }
  return baseboardSerialNumbers[0];
}

std::vector<std::string> HardwareService::getMACAddress()
{
    return service.GetPropertyValues(L"Win32_NetworkAdapter", L"MACAddress", L"PhysicalAdapter = TRUE");
}

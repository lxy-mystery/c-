#ifndef __WMI_SERVICE_H__
#define __WMI_SERVICE_H__
#pragma comment(lib, "wbemuuid.lib")
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <string>
class WMIService
{
public:
  WMIService();

  ~WMIService();

  std::vector<std::string> GetPropertyValues(const std::wstring& className, const std::wstring& propertyName, const std::wstring& conditaion = L"");
private:
  IWbemLocator* pLoc;
  IWbemServices* pSvc;
};
#endif
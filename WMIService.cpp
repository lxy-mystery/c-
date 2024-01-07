#include "pch.h"
#include "WMIService.h"
#include <locale>
#include <codecvt>

std::string wstringToString(const std::wstring& wstr) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(wstr);
}

WMIService::WMIService() : pSvc(nullptr)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLoc));
        if (SUCCEEDED(hr))
        {
            hr = pLoc->ConnectServer(bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
            if (SUCCEEDED(hr))
            {
                hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
                if (FAILED(hr))
                {
                    std::cerr << "Failed to set proxy blanket." << std::endl;
                }
            }
            else
            {
                std::cerr << "Failed to connect to WMI server." << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to create WMI locator." << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to initialize COM." << std::endl;
    }
}

WMIService::~WMIService()
{
  if (pSvc)
  {
    pSvc->Release();
  }

  if (pLoc)
  {
    pLoc->Release();
  }

  CoUninitialize();
}

std::vector<std::string> WMIService::GetPropertyValues(const std::wstring& className, const std::wstring& propertyName, const std::wstring& conditaion)
{
  std::vector<std::string> propertyValues;

  std::wstring query = L"SELECT " + propertyName + L" FROM " + className;
  if (conditaion.length() > 0) {
    query = query + L" WHERE " + conditaion;
  }

  IEnumWbemClassObject* pEnumerator = nullptr;
  HRESULT hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(query.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
  if (SUCCEEDED(hr))
  {
    while (true)
    {
      IWbemClassObject* pclsObj = nullptr;
      ULONG uReturn = 0;
      hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
      if (hr == WBEM_S_FALSE || uReturn == 0)
      {
        break;
      }

      VARIANT vtProp;
      hr = pclsObj->Get(propertyName.c_str(), 0, &vtProp, 0, 0);
      if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
      {
        propertyValues.push_back(wstringToString(vtProp.bstrVal));
      }

      VariantClear(&vtProp);

      pclsObj->Release();
    }

    pEnumerator->Release();
  }

  return propertyValues;
}

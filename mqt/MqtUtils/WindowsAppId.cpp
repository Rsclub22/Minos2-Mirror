#include <QWidget>

#ifdef Q_OS_WIN
#define _WIN32_WINNT      _WIN32_WINNT_WIN7

#include <SDKDDKVer.h>

#include <propsys.h>
#include <propkey.h>
#include <shlwapi.h>
#endif

#include "WindowsAppId.h"


#ifdef Q_OS_WIN
extern "C"
{
BOOL WndSetAppId(HWND hWnd, const WCHAR *pAppId)
{
    IPropertyStore *pps;
    PROPVARIANT pv;
    HRESULT hr;

    hr = SHGetPropertyStoreForWindow(hWnd, IID_IPropertyStore, reinterpret_cast<void **>(&pps));
    if(SUCCEEDED(hr))
    {
        if(pAppId)
        {
            pv.vt = VT_LPWSTR;
            hr = SHStrDup(pAppId, &pv.pwszVal);
        }
        else
            PropVariantInit(&pv);

        if(SUCCEEDED(hr))
        {
            hr = pps->SetValue(PKEY_AppUserModel_ID, pv);
            if(SUCCEEDED(hr))
                hr = pps->Commit();

            PropVariantClear(&pv);
        }

        pps->Release();
    }

    return SUCCEEDED(hr);
}
}
#endif
void setWinAppId(QWidget *w, QString id)
{
#ifdef Q_OS_WIN
    WId wid = w->winId();
    HWND hwnd = reinterpret_cast<HWND>(wid);

    const wchar_t *cwc = reinterpret_cast<const wchar_t *>(id.utf16());

    WndSetAppId(hwnd, cwc);
#endif
}
void clearWinAppId(QWidget *w)
{
#ifdef Q_OS_WIN
    WId wid = w->winId();
    HWND hwnd = reinterpret_cast<HWND>(wid);

    WndSetAppId(hwnd, nullptr);
#endif
}


#include <QSettings>

#include "regsettings.h"
#include "MTrace.h"
#include "rigcontrolmainwindow.h"

#include "powermateframe.h"
#include "ui_powermateframe.h"

#if defined (WIN32)

#include <shellapi.h>

extern "C"
{
#include "setupapi.h"
#include "hidsdi.h"
}
#include <tlhelp32.h>


enum HidAttributes			// Griffin PowerMate Knob ID
{
    VendorID		= 0x077d,
    ProductID		= 0x0410,
    VersionNumber	= 0x0311
};

enum RotationalDirection
{
    Left,
    Right
};

enum ButtonState
{
    Up,
    Down
};
#endif
PowerMateFrame::PowerMateFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PowerMateFrame)
{
    ui->setupUi(this);

#if not defined (WIN32)
    ui->enabledCb->setVisible(false);
#else
    RegSettings settings;

    bool cenabled = settings.getSettings().value("PowerMateFrame/Enabled", false).toBool();
    ui->enabledCb->setChecked(cenabled);
#endif
}

PowerMateFrame::~PowerMateFrame()
{
    delete ui;
}

void PowerMateFrame::on_enabledCb_clicked()
{
    RegSettings settings;
    settings.getSettings().setValue("PowerMateFrame/Enabled", ui->enabledCb->isChecked());
}
ControlPowerMate *ControlPowerMate::cp = nullptr;

ControlPowerMate::ControlPowerMate()
{
    connect(this, &ControlPowerMate::dataReceived, mainWindow, &RigControlMainWindow::tuneData);

}
ControlPowerMate::~ControlPowerMate()
{

}

void ControlPowerMate::start()
{
#if defined (WIN32)

    stop();
    RegSettings settings;
    bool enabled = settings.getSettings().value("PowerMateFrame/Enabled", false).toBool();

    if (enabled)
    {
        //==============PowerMate====================================================

        HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        if( Process32First(h, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"PowerMate"))
                {
                    HANDLE hp = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE,FALSE,pe.th32ProcessID);


                    HWND hWnd = FindWindow(NULL, L"PowerMateWnd");

                    if(NULL != hWnd)
                    {
                        trace("PowerMate is RUNNING");

                        PostMessage(hWnd, WM_CLOSE,0,0);

                        if( WaitForSingleObject(hp, 5000)!= WAIT_OBJECT_0)
                        {
                            trace("Failed to close PowerMate.exe");
                        }
                        else
                        {
                            trace("PowerMate.exe has been closed");
                        }
                    }
                }
            } while( Process32Next(h, &pe));
        }

        CloseHandle(h);

        // ke9ns INITIALIZE POWERMATE KNOB


        GUID hidClass;


        HidD_GetHidGuid(&hidClass);									// returns the device interface GUID for HIDClass devices.

        // retrieves a device information set that contains all the
        // devices of a specified class (hint: our HIDClass)
        HDEVINFO hDevInfoSet = SetupDiGetClassDevs(&hidClass,
                                                   NULL,					// retrieve dev info for all instances
                                                   NULL,				    // hWnd parent
                                                   DIGCF_PRESENT |			// only present devices
                                                       DIGCF_INTERFACEDEVICE);	// that expose hid interface



        if (INVALID_HANDLE_VALUE == hDevInfoSet)
        {
            trace("SetupDiGetClassDevs");
            return;
        }


        SP_INTERFACE_DEVICE_DATA interfaceData;												 // enumerate devices looking for hid interfaces
        interfaceData.cbSize = sizeof(interfaceData);

        // poll device manager until no matching devices left
        for(int i = 0; SetupDiEnumDeviceInterfaces(hDevInfoSet,NULL,&hidClass,i,&interfaceData); ++i)
        {

            DWORD bufferLength;                                                     // retrieve buffer size for interface detail data

            // retrieves detailed information about a specified device interface
            SetupDiGetDeviceInterfaceDetail(hDevInfoSet,&interfaceData,NULL,0,&bufferLength,NULL);

            PSP_INTERFACE_DEVICE_DETAIL_DATA interfaceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)new char[bufferLength];

            interfaceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

            // now get the interface detail data
            if( !SetupDiGetDeviceInterfaceDetail(hDevInfoSet,&interfaceData,interfaceDetail,bufferLength,NULL,NULL))
            {
                delete [] interfaceDetail;
                SetupDiDestroyDeviceInfoList(hDevInfoSet);
                break;
            }

            //==================================================================================================
            // - now that we have the device path, open device
            // - use FILE_FLAG_OVERLAPPED for simultaneous read/write
            // - the system does not maintain the file pointer,
            //   therefore you must pass the file position to the
            //   read and write functions in the OVERLAPPED structure

            HANDLE hDevice = CreateFile(interfaceDetail->DevicePath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED ,NULL);

            if( INVALID_HANDLE_VALUE == hDevice )
            {
                delete [] interfaceDetail;
                continue; // keep searching
            }
            else
            {
                HIDD_ATTRIBUTES hidAttr;
                BOOLEAN result = HidD_GetAttributes(hDevice,&hidAttr);

                if (result)
                {
                    // this is what we are looking for (PowerMate Knob)
                    //	VendorID		= 0x077d,
                    // ProductID		= 0x0410,
                    // VersionNumber	= 0x0311
                    // grab the first matching device we find and return



                    if( (int)HidAttributes::ProductID == hidAttr.ProductID && (int)HidAttributes::VendorID  == hidAttr.VendorID )
                    {
                        trace("FOUND GRIFFIN KNOB " );

                        handleToDevice = hDevice;

                        delete [] interfaceDetail;
                        SetupDiDestroyDeviceInfoList(hDevInfoSet);

                        pmThread = new PMThread(this, handleToDevice);
                        pmThread->start();

                    } // found knob

                } // result

            } // valid handle



        } // FOR LOOP

        SetupDiDestroyDeviceInfoList(hDevInfoSet);


    }
#endif
}

void ControlPowerMate::stop()
{
#if defined (WIN32)
    if (pmThread)
    {
        pmThread->terminated = true;
        pmThread->wait(1000);
        pmThread->deleteLater();
        pmThread = nullptr;
    }
#endif
}

#if defined (WIN32)


PMThread::PMThread(ControlPowerMate *p, HANDLE devH):QThread(p), devHandle(devH), controller(p)
{

}

PMThread::~PMThread()
{

}

void PMThread::run()
{
    trace("PMThread running");
    char reportBuffer[8] = {0};

    DWORD dwBytesRead = 0;									//

    HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);		//

    OVERLAPPED overLap;										//
    ZeroMemory( &overLap, sizeof(overLap));					//
    overLap.hEvent = hEvent;								//

    while(!terminated)
    {

        BOOL result = ReadFile(devHandle, reportBuffer, sizeof(reportBuffer), &dwBytesRead, &overLap);// async call

        DWORD dw = WaitForSingleObject(hEvent, TIMER_CAT);  // wait for 200=200mSec (was 2 seconds)

        switch(dw)
        {
        case WAIT_OBJECT_0:
        {
            if (reportBuffer[1] == 1)   // test for pushbutton
            {

                ButtonState bs = ButtonState::Down;
                //EventsHelper::Tire(buttonDelegate, bs, bandsel, qqq, aaa);  // send button state, and bandsel to form

                trace("DETECTED DN");

            }
            else if ( reportBuffer[2] == 0 && reportBuffer[1] == 0)  // test for knob UP
            {
                ButtonState bs = ButtonState::Up;
                //EventsHelper::Tire(buttonDelegate, bs, bandsel1, qqq, aaa);

                trace("DETECTED UP");
            }


            int rotvalue = reportBuffer[2];

            if (rotvalue != 0)                    // test for knob rotation
            {
                trace(QString("Reportbuffer %1").arg(rotvalue));

                if (rotvalue >= 1)
                {
                    emit controller->dataReceived(QString("U%1;").arg(rotvalue, 2, 10, QChar('0')).toLocal8Bit());
                }
                else if (rotvalue <= -1)
                {
                    emit controller->dataReceived(QString("D%1;").arg(-rotvalue, 2, 10, QChar('0')).toLocal8Bit());
                }

            } // knob rotation


        }
        break;  // case wait_object 0


            //==================================================================
        case WAIT_TIMEOUT:
        {

            if (terminated)
            {
                CloseHandle(hEvent);
                trace("hEvent closed");
            }
        }
        break;

        } // end switch

    } // end while
    trace("PMThread closed");
}

#endif

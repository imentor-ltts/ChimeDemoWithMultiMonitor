#pragma once
#include<Windows.h>
#include <assert.h>
#include <strsafe.h>
class CServiceBase
{
public:

    // Register the executable for a service with the Service Control Manager 
    // (SCM). After you call Run(ServiceBase), the SCM issues a Start command, 
    // which results in a call to the OnStart method in the service. This 
    // method blocks until the service has stopped.
    static BOOL Run(CServiceBase& service) {
        s_service = &service;

        SERVICE_TABLE_ENTRY serviceTable[] =
        {
            { service.m_name, ServiceMain },
            { NULL, NULL }
        };

        // Connects the main thread of a service process to the service control 
        // manager, which causes the thread to be the service control dispatcher 
        // thread for the calling process. This call returns when the service has 
        // stopped. The process should simply terminate when the call returns.
        return StartServiceCtrlDispatcher(serviceTable);
    }

    CServiceBase(PWSTR pszServiceName,
        BOOL fCanStop = TRUE,
        BOOL fCanShutdown = TRUE,
        BOOL fCanPauseContinue = FALSE) {

        // Service name must be a valid string and cannot be NULL.
        m_name = (PWSTR)((pszServiceName == NULL) ? L"" : pszServiceName);

        m_statusHandle = NULL;

        // The service runs in its own process.
        m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

        // The service is starting.
        m_status.dwCurrentState = SERVICE_START_PENDING;

        // The accepted commands of the service.
        DWORD dwControlsAccepted = 0;
        if (fCanStop)
            dwControlsAccepted |= SERVICE_ACCEPT_STOP;
        if (fCanShutdown)
            dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
        if (fCanPauseContinue)
            dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
        m_status.dwControlsAccepted = dwControlsAccepted;

        m_status.dwWin32ExitCode = NO_ERROR;
        m_status.dwServiceSpecificExitCode = 0;
        m_status.dwCheckPoint = 0;
        m_status.dwWaitHint = 0;

    }

    // Service object destructor. 
    virtual ~CServiceBase(void) {

    }

    // Stop the service.
    void Stop() {
        DWORD dwOriginalState = m_status.dwCurrentState;
        try
        {
            // Tell SCM that the service is stopping.
            SetServiceStatus(SERVICE_STOP_PENDING);

            // Perform service-specific stop operations.
            OnStop();

            // Tell SCM that the service is stopped.
            SetServiceStatus(SERVICE_STOPPED);
        }
        catch (DWORD dwError)
        {
            // Log the error.
            WriteErrorLogEntry((PWSTR)L"Service Stop", dwError);

            // Set the orginal service status.
            SetServiceStatus(dwOriginalState);
        }
        catch (...)
        {
            // Log the error.
            WriteEventLogEntry((PWSTR)L"Service failed to stop.", EVENTLOG_ERROR_TYPE);

            // Set the orginal service status.
            SetServiceStatus(dwOriginalState);
        }
    }

protected:

    // When implemented in a derived class, executes when a Start command is 
    // sent to the service by the SCM or when the operating system starts 
    // (for a service that starts automatically). Specifies actions to take 
    // when the service starts.
    virtual void OnStart(DWORD dwArgc, PWSTR* pszArgv) {

    }
    // When implemented in a derived class, executes when a Stop command is 
    // sent to the service by the SCM. Specifies actions to take when a 
    // service stops running.
    virtual void OnStop() {

    }

    // When implemented in a derived class, executes when a Pause command is 
    // sent to the service by the SCM. Specifies actions to take when a 
    // service pauses.
    virtual void OnPause() {}

    // When implemented in a derived class, OnContinue runs when a Continue 
    // command is sent to the service by the SCM. Specifies actions to take 
    // when a service resumes normal functioning after being paused.
    virtual void OnContinue() {}

    // When implemented in a derived class, executes when the system is 
    // shutting down. Specifies what should occur immediately prior to the 
    // system shutting down.
    virtual void OnShutdown() {}

    // Set the service status and report the status to the SCM.
    void SetServiceStatus(DWORD dwCurrentState,
        DWORD dwWin32ExitCode = NO_ERROR,
        DWORD dwWaitHint = 0){}

    // Log a message to the Application event log.
    void WriteEventLogEntry(PWSTR pszMessage, WORD wType){}

    // Log an error message to the Application event log.
    void WriteErrorLogEntry(PWSTR pszFunction,
        DWORD dwError = GetLastError()){}

private:

    // Entry point for the service. It registers the handler function for the 
    // service and starts the service.
    static void WINAPI ServiceMain(DWORD dwArgc, LPWSTR* lpszArgv) {
        assert(s_service != NULL);

        // Register the handler function for the service
        s_service->m_statusHandle = RegisterServiceCtrlHandler(
            s_service->m_name, ServiceCtrlHandler);
        if (s_service->m_statusHandle == NULL)
        {
            throw GetLastError();
        }

        // Start the service.
        s_service->Start(dwArgc, lpszArgv);
    }

    // The function is called by the SCM whenever a control code is sent to 
    // the service.
    static void WINAPI ServiceCtrlHandler(DWORD dwCtrl){
        switch (dwCtrl)
        {
        case SERVICE_CONTROL_STOP: s_service->Stop(); break;
        case SERVICE_CONTROL_PAUSE: s_service->Pause(); break;
        case SERVICE_CONTROL_CONTINUE: s_service->Continue(); break;
        case SERVICE_CONTROL_SHUTDOWN: s_service->Shutdown(); break;
        case SERVICE_CONTROL_INTERROGATE: break;
        default: break;
        }
    }

    // Start the service.
    void Start(DWORD dwArgc, PWSTR* pszArgv){
        try
        {
            // Tell SCM that the service is starting.
            SetServiceStatus(SERVICE_START_PENDING);

            // Perform service-specific initialization.
            OnStart(dwArgc, pszArgv);

            // Tell SCM that the service is started.
            SetServiceStatus(SERVICE_RUNNING);
        }
        catch (DWORD dwError)
        {
            // Log the error.
            WriteErrorLogEntry((PWSTR)L"Service Start", dwError);

            // Set the service status to be stopped.
            SetServiceStatus(SERVICE_STOPPED, dwError);
        }
        catch (...)
        {
            // Log the error.
            WriteEventLogEntry((PWSTR)"Service failed to start.", EVENTLOG_ERROR_TYPE);

            // Set the service status to be stopped.
            SetServiceStatus(SERVICE_STOPPED);
        }
    }

    // Pause the service.
    void Pause(){
        try
        {
            // Tell SCM that the service is pausing.
            SetServiceStatus(SERVICE_PAUSE_PENDING);

            // Perform service-specific pause operations.
            OnPause();

            // Tell SCM that the service is paused.
            SetServiceStatus(SERVICE_PAUSED);
        }
        catch (DWORD dwError)
        {
            // Log the error.
            WriteErrorLogEntry((PWSTR)L"Service Pause", dwError);

            // Tell SCM that the service is still running.
            SetServiceStatus(SERVICE_RUNNING);
        }
        catch (...)
        {
            // Log the error.
            WriteEventLogEntry((PWSTR)L"Service failed to pause.", EVENTLOG_ERROR_TYPE);

            // Tell SCM that the service is still running.
            SetServiceStatus(SERVICE_RUNNING);
        }
    }

    // Resume the service after being paused.
    void Continue(){
        try
        {
            // Tell SCM that the service is resuming.
            SetServiceStatus(SERVICE_CONTINUE_PENDING);

            // Perform service-specific continue operations.
            OnContinue();

            // Tell SCM that the service is running.
            SetServiceStatus(SERVICE_RUNNING);
        }
        catch (DWORD dwError)
        {
            // Log the error.
            WriteErrorLogEntry((PWSTR)L"Service Continue", dwError);

            // Tell SCM that the service is still paused.
            SetServiceStatus(SERVICE_PAUSED);
        }
        catch (...)
        {
            // Log the error.
            WriteEventLogEntry((PWSTR)L"Service failed to resume.", EVENTLOG_ERROR_TYPE);

            // Tell SCM that the service is still paused.
            SetServiceStatus(SERVICE_PAUSED);
        }

    // Execute when the system is shutting down.
    void Shutdown(){}

    // The singleton service instance.
    static CServiceBase* s_service;

    // The name of the service
    PWSTR m_name;

    // The status of the service
    SERVICE_STATUS m_status;

    // The service status handle
    SERVICE_STATUS_HANDLE m_statusHandle;
};

#pragma region Static members
// Initialize the singleton service instance.
CServiceBase* CServiceBase::s_service = NULL;


#ifndef _IOPBIOS_H_
#define _IOPBIOS_H_

#include <list>
#include "../MIPSAssembler.h"
#include "../MIPS.h"
#include "../ELF.h"
#include "../SIF.h"
#include "../ISO9660/ISO9660.h"
#include "Iop_Ioman.h"
#include "Iop_Stdio.h"
#include "Iop_Sysmem.h"
#include "Iop_Modload.h"
#ifdef _IOP_EMULATE_MODULES
#include "Iop_DbcMan.h"
#include "Iop_PadMan.h"
#endif

class CIopBios
{
public:
                            CIopBios(uint32, CMIPS&, uint8*, uint32, CSIF&, CISO9660*&);
    virtual                 ~CIopBios();

    void                    LoadAndStartModule(const char*, const char*, unsigned int);
    void                    LoadAndStartModule(uint32, const char*, unsigned int);
    void                    HandleException();
    void                    HandleInterrupt();

    void                    Reset();

    Iop::CIoman*            GetIoman();
#ifdef _IOP_EMULATE_MODULES
    Iop::CDbcMan*           GetDbcman();
    Iop::CPadMan*           GetPadman();
#endif
    void                    RegisterModule(Iop::CModule*);

    uint32                  CreateThread(uint32, uint32);
    void                    StartThread(uint32, uint32* = NULL);
    void                    DelayThread(uint32);
    uint32                  GetThreadId();
    void                    SleepThread();
    uint32                  WakeupThread(uint32);

    uint32                  CreateSemaphore(uint32, uint32);
    uint32                  SignalSemaphore(uint32, bool);
    uint32                  WaitSemaphore(uint32);

    bool                    RegisterIntrHandler(uint32, uint32, uint32, uint32);
    bool                    ReleaseIntrHandler(uint32);

private:
    enum DEFAULT_STACKSIZE
    {
        DEFAULT_STACKSIZE = 0x8000,
    };

    enum DEFAULT_PRIORITY
    {
        DEFAULT_PRIORITY = 7,
    };

    struct THREADCONTEXT
    {
        uint32      gpr[0x20];
        uint32      epc;
        uint32      delayJump;
    };

    struct THREAD
    {
        uint32          id;
        uint32          priority;
        THREADCONTEXT   context;
        uint32          status;
        uint32          waitSemaphore;
        uint32          wakeupCount;
        uint64          nextActivateTime;
    };

    struct SEMAPHORE
    {
        uint32          id;
        uint32          count;
        uint32          maxCount;
        uint32          waitCount;
    };

    struct INTRHANDLER
    {
        uint32          line;
        uint32          mode;
        uint32          handler;
        uint32          arg;
    };

    struct LOADEDMODULE
    {
        std::string     name;
        uint32          begin;
        uint32          end;
    };

    enum THREAD_STATUS
    {
        THREAD_STATUS_CREATED = 1,
        THREAD_STATUS_RUNNING = 2,
        THREAD_STATUS_SLEEPING = 3,
        THREAD_STATUS_ZOMBIE = 4,
        THREAD_STATUS_WAITING = 5,
    };

    typedef std::multimap<uint32, THREAD, std::greater<uint32> > ThreadMapType;
    typedef std::map<std::string, Iop::CModule*> IopModuleMapType;
    typedef std::map<uint32, SEMAPHORE> SemaphoreMapType;
    typedef std::map<uint32, INTRHANDLER> IntrHandlerMapType;
    typedef std::list<LOADEDMODULE> LoadedModuleListType;
    typedef std::pair<uint32, uint32> ExecutableRange;

    THREAD&                 GetThread(uint32);
    ThreadMapType::iterator GetThreadPosition(uint32);
    void                    ExitCurrentThread();
    void                    LoadThreadContext(uint32);
    void                    SaveThreadContext(uint32);
    void                    Reschedule();
    uint32                  GetNextReadyThread(bool);
    uint64                  GetCurrentTime();
	void					ReturnFromException();

    SEMAPHORE&              GetSemaphore(uint32);

    void                    LoadAndStartModule(CELF&, const char*, const char*, unsigned int);
    uint32                  LoadExecutable(CELF&, ExecutableRange&);
    unsigned int            GetElfProgramToLoad(CELF&);
    void                    RelocateElf(CELF&, uint32);
    std::string             ReadModuleName(uint32);
    std::string             GetModuleNameFromPath(const std::string&);
    const LOADEDMODULE&     GetModuleAtAddress(uint32);
    void                    LoadModuleTags(const LOADEDMODULE&, CMIPSTags&, const char*);
    void                    SaveAllModulesTags(CMIPSTags&, const char*);
    void                    DeleteModules();
    uint32                  Push(uint32&, const uint8*, uint32);

    uint32                  AssembleThreadFinish(CMIPSAssembler&);
	uint32					AssembleReturnFromException(CMIPSAssembler&);
	uint32					AssembleIdleFunction(CMIPSAssembler&);

    CMIPS&                  m_cpu;
    CSIF&                   m_sif;
    CISO9660*&              m_iso;
    uint8*                  m_ram;
    uint32                  m_ramSize;
    uint32                  m_baseAddress;
    uint32                  m_threadFinishAddress;
	uint32					m_returnFromExceptionAddress;
	uint32					m_idleFunctionAddress;
    uint32                  m_nextThreadId;
    uint32                  m_nextSemaphoreId;
    uint32                  m_currentThreadId;
    bool                    m_rescheduleNeeded;
    ThreadMapType           m_threads;
    SemaphoreMapType        m_semaphores;
    IntrHandlerMapType      m_intrHandlers;
    IopModuleMapType        m_modules;
    LoadedModuleListType    m_loadedModules;
    Iop::CSifMan*           m_sifMan;
    Iop::CStdio*            m_stdio;
    Iop::CIoman*            m_ioman;
    Iop::CSysmem*           m_sysmem;
    Iop::CModload*          m_modload;
#ifdef _IOP_EMULATE_MODULES
    Iop::CDbcMan*           m_dbcman;
    Iop::CPadMan*           m_padman;
#endif
};

#endif

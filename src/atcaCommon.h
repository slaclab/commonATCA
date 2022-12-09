#ifndef _ATCA_COMMON_FW_H
#define _ATCA_COMMON_FW_H

#include <cpsw_api_user.h>
#include <cpsw_api_builder.h>

typedef enum {
   twogb = 0,
   fourgb,
   eightgb,
   autogb
} dram_region_size_t;

class IATCACommonFw;
typedef shared_ptr<IATCACommonFw> ATCACommonFw;

class IATCACommonFw : public virtual IEntry {
public:
    static ATCACommonFw create(Path p);
    // debug streams
    virtual void createStreams(ConstPath p, const char *prefix)     = 0;
    virtual int64_t readStream(uint32_t index, uint8_t *buf, uint64_t size, CTimeout timeout) = 0;
    //

    virtual void getUpTimeCnt(uint32_t *cnt)             = 0;
    virtual void getBuildStamp(uint8_t *str)             = 0;
    virtual void getFpgaVersion(uint32_t *ver)           = 0;
    virtual void getFpgaTemperature(uint32_t *val)       = 0;
    virtual void getEthUpTimeCnt(uint32_t *cnt)          = 0;
    virtual void getGitHash(uint8_t *str)                = 0;
    virtual void getJesdCnt(uint32_t *cnt, int i, int j) = 0;
    virtual void getAmcClkFreq(uint32_t *freq, int i)    = 0;
    
    // DaqMux Commands
    virtual void triggerDaq(int index)                   = 0;
    virtual void armHwTrigger(int index)                 = 0;
    virtual void freezeBuffer(int index)                 = 0;
    virtual void clearTriggerStatus(int index)           = 0;
    // DaqMux Control
    virtual void cascadedTrigger(uint32_t cmd, int index)    = 0;
    virtual void hardwareAutoRearm(uint32_t cmd, int index)  = 0;
    virtual void daqMode(uint32_t cmd, int index)            = 0;
    virtual void enablePacketHeader(uint32_t cmd, int index) = 0;
    virtual void enableHardwareFreeze(uint32_t cmd, int index) = 0;
    virtual void decimationRateDivisor(uint32_t div, int index) = 0;
    virtual void dataBufferSize(uint32_t size, int index)       = 0;
    virtual void getTimestamp(uint32_t *sec, uint32_t *nsec, int index) = 0;
    virtual void getTriggerCount(uint32_t *val, int index) = 0;
    virtual void dbgInputValid(uint32_t *val, int index)   = 0;
    virtual void dbgLinkReady(uint32_t *val, int index)    = 0;
    virtual void inputMuxSelect(uint32_t val, int index, int chn) = 0;
    virtual void getStreamPause(uint32_t *val, int index, int chn) = 0;
    virtual void getStreamPause(uint32_t *vals, int index)         = 0;
    virtual void getStreamReady(uint32_t *val, int index, int chn) = 0;
    virtual void getStreamReady(uint32_t *vals, int index)         = 0;
    virtual void getStreamOverflow(uint32_t *val, int index, int chn) = 0;
    virtual void getStreamOverflow(uint32_t *val, int index)          = 0;
    virtual void getStreamError(uint32_t *val, int index, int chn)    = 0;
    virtual void getStreamError(uint32_t *val, int index)             = 0;
    virtual void getInputDataValid(uint32_t *val, int index, int chn) = 0;
    virtual void getInputDataValid(uint32_t *val, int index)          = 0;
    virtual void getStreamEnabled(uint32_t *val, int index, int chn)  = 0;
    virtual void getStreamEnabled(uint32_t *vals, int index)          = 0;
    virtual void getFrameCount(uint32_t *val, int index, int chn)     = 0;
    virtual void getFrameCount(uint32_t *val, int index)              = 0;
    virtual void formatSignWidth(uint32_t val, int index, int chn)    = 0;
    virtual void formatDataWidth(uint32_t val, int index, int chn)    = 0;
    virtual void enableFormatSign(uint32_t val, int index, int chn)   = 0;
    virtual void enableDecimation(uint32_t val, int index, int chn)   = 0;

    virtual void getWfEngineStartAddr(uint64_t *val, int index, int chn) = 0;
    virtual void getWfEngineEndAddr(uint64_t *val, int index, int chn) = 0;
    virtual void getWfEngineWrAddr(uint64_t *val, int index, int chn) = 0;
    virtual void getWfEngineStatus(uint32_t *val, int index, int chn) = 0;

    virtual void setWfEngineStartAddr(uint64_t val, int index, int chn) = 0;
    virtual void setWfEngineEndAddr(uint64_t val, int index, int chn) = 0;
    virtual void enableWfEngine(uint32_t val, int index, int chn) = 0;
    virtual void setWfEngineMode(uint32_t val, int index, int chn) = 0;
    virtual void setWfEngineMsgDest(uint32_t val, int index, int chn) = 0;
    virtual void setWfEngineFramesAfterTrigger(uint32_t val, int index, int chn) = 0;

    virtual void initWfEngine(int index) = 0;
    virtual int  setupWaveformEngine(unsigned waveformEngineIndex, uint64_t sizeInBytes, dram_region_size_t ramAllocatedSize) = 0;
    virtual dram_region_size_t getAllocableSize(unsigned sizeInBytes) = 0;
    virtual void setupDaqMux(unsigned daqMuxIndex) = 0;
};

#endif /* _ATCA_COMMON_FW_H */

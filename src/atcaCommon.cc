#include <cpsw_yaml.h>
#include <yaml-cpp/yaml.h>
#include <cpsw_sval.h>
#include <cpsw_entry_adapt.h>
#include <cpsw_hub.h>
#include <fstream>
#include <sstream>

#include <string.h>
#include <math.h>

#include "atcaCommon.h"

#define MAX_DEBUG_STREAM   8
#define MAX_DAQMUX_CNT     2
#define MAX_WAVEFORMENGINE_CNT 2

#define MAX_JESD_CNT       6
#define NUM_JESD           2
#define JESD_CNT_STR       "JesdRx/StatusValidCnt[%d]"

#define CPSW_TRY_CATCH(X)       try {   \
        (X);                            \
    } catch (CPSWError &e) {            \
        fprintf(stderr,                 \
                "CPSW Error: %s at %s, line %d\n",     \
                e.getInfo().c_str(),    \
                __FILE__, __LINE__);    \
        throw e;                        \
    }


extern "C" {
int32_t Gen2UpConvYaml = 0;    // will be expose to epics ioc shell variable  0: old upconverter, 1: gen2 upconverter
}


class CATCACommonFwAdapt;
typedef shared_ptr<CATCACommonFwAdapt> ATCACommonFwAdapt;

class CATCACommonFwAdapt : public IATCACommonFw, public IEntryAdapt {
    protected:
        Path         _p_axiVersion;
        Path         _p_axiSysMonUltraScale;
        Path         _p_bsi;
        Path         _p_jesd0;
        Path         _p_jesd1;

        Path         _p_waveformEngine[MAX_WAVEFORMENGINE_CNT];


        Path         _p_daqMuxV2[MAX_DAQMUX_CNT];

// debug stream
        Stream      _stream[MAX_DEBUG_STREAM];

// Common
        ScalVal_RO   _upTimeCnt;
        ScalVal_RO   _buildStamp;
        ScalVal_RO   _fpgaVersion;
        ScalVal_RO   _EthUpTimeCnt;
        ScalVal_RO   _GitHash;
        ScalVal_RO   _fpgaTemp;
// JESD Counter
        ScalVal_RO   _jesd0ValidCnt[MAX_JESD_CNT];
        ScalVal_RO   _jesd1ValidCnt[MAX_JESD_CNT];
// DaqMuxV2
        struct {
        ScalVal      _triggerCasc;   // enable/disable cascaded trigger
        ScalVal      _autoRearm;     // auto re-arm for hardware trigger
        ScalVal      _daqMode;       // daq mode 0: trigger, 1: continuous
        ScalVal      _packetHeader;  // packet header 0: disable, 1: enable
        ScalVal      _freezeHwMask;    // mask for enabling/disabling hardware feeze buffer request
        ScalVal      _decimationRateDiv;  // decimateRate Dvisor
        ScalVal      _bufferSize;         // data buffer size
        ScalVal_RO   _timestamp[2];          // timestamp
        ScalVal_RO   _triggerCnt;          // trigger counter
        ScalVal_RO   _dbgInputValid;      // Input Valid bus for debugging
        ScalVal_RO   _dbgLinkReady;        // Input Link Ready
        ScalVal      _inputMuxSel[4];        // Input Mux Selector
        ScalVal_RO   _streamPause[4];
        ScalVal_RO   _streamReady[4];        // raw diagnostic stream control ready
        ScalVal_RO   _streamOverflow[4];     // raw diagnostic stream control overflow
        ScalVal_RO   _streamError[4];        // error during last acquisition
        ScalVal_RO   _inputDataValid[4];     // the incoming data is valid
        ScalVal_RO   _streamEnabled[4];      // Output stream enabled
        ScalVal_RO   _frameCnt[4];           // frame counter
        ScalVal      _formatSignWidth[4];    // indicating sign extension point
        ScalVal      _formatDataWidth[4];    // data width 32bit (0) or 16bit (1)
        ScalVal      _formatSign[4];         // unsigned (0) or signed (1)
        ScalVal      _decimation[4];
        Command      _triggerDaq;
        Command      _armHwTrigger;
        Command      _freezeBuffers;
        Command      _clearTrigStatus;
        } _daqMux[MAX_DAQMUX_CNT];

// Waveform Engines

        struct {
        ScalVal     _startAddr[4];
        ScalVal     _endAddr[4];
        ScalVal_RO  _wrAddr[4];
        ScalVal     _enabled[4];
        ScalVal     _mode[4];
        ScalVal     _msgDest[4];
        ScalVal     _framesAfterTrigger[4];
        ScalVal_RO  _status[4];
        Command     _initialize;
        } _waveformEngine[MAX_WAVEFORMENGINE_CNT];


    public:
        CATCACommonFwAdapt(Key &k, ConstPath p, shared_ptr<const CEntryImpl> ie);
        virtual void createStreams(ConstPath p, const char *prefix);
        virtual int64_t readStream(uint32_t index, uint8_t *buff, uint64_t size, CTimeout timeout);
        virtual void getUpTimeCnt(uint32_t *cnt);
        virtual void getBuildStamp(uint8_t *str);
        virtual void getFpgaVersion(uint32_t *ver);
        virtual void getFpgaTemperature(uint32_t *val);
        virtual void getEthUpTimeCnt(uint32_t *cnt);
        virtual void getGitHash(uint8_t *str);
        virtual void getJesdCnt(uint32_t *cnt, int i, int j);

        // DaqMux Commands
        virtual void triggerDaq(int index);
        virtual void armHwTrigger(int index);
        virtual void freezeBuffer(int index);
        virtual void clearTriggerStatus(int index);

        virtual void cascadedTrigger(uint32_t cmd, int index);
        virtual void hardwareAutoRearm(uint32_t cmd, int index);
        virtual void daqMode(uint32_t cmd, int index);
        virtual void enablePacketHeader(uint32_t cmd, int index);
        virtual void enableHardwareFreeze(uint32_t cmd, int index);
        virtual void decimationRateDivisor(uint32_t div, int index);
        virtual void dataBufferSize(uint32_t size, int index);
        virtual void getTimestamp(uint32_t *sec, uint32_t *nsec, int index);
        virtual void getTriggerCount(uint32_t *count, int index);
        virtual void dbgInputValid(uint32_t *val, int index);
        virtual void dbgLinkReady(uint32_t *val, int index);
        virtual void inputMuxSelect(uint32_t val, int index, int chn);
        virtual void getStreamPause(uint32_t *val, int index, int chn);
        virtual void getStreamPause(uint32_t *vals, int index);
        virtual void getStreamReady(uint32_t *val, int index, int chn);
        virtual void getStreamReady(uint32_t *vals, int index);
        virtual void getStreamOverflow(uint32_t *val, int index, int chn);
        virtual void getStreamOverflow(uint32_t *vals, int index);
        virtual void getStreamError(uint32_t *val, int index, int chn);
        virtual void getStreamError(uint32_t *vals, int index);
        virtual void getInputDataValid(uint32_t *val, int index, int chn);
        virtual void getInputDataValid(uint32_t *vals, int index);
        virtual void getStreamEnabled(uint32_t *val, int index, int chn);
        virtual void getStreamEnabled(uint32_t *vals, int index);
        virtual void getFrameCount(uint32_t *val, int index, int chn);
        virtual void getFrameCount(uint32_t *val, int index);
        virtual void formatSignWidth(uint32_t val, int index, int chn);
        virtual void formatDataWidth(uint32_t val, int index, int chn);
        virtual void enableFormatSign(uint32_t val, int index, int chn);
        virtual void enableDecimation(uint32_t val, int index, int chn);

        virtual void getWfEngineStartAddr(uint64_t *val, int index, int chn);
        virtual void getWfEngineEndAddr(uint64_t *val, int index, int chn);
        virtual void getWfEngineWrAddr(uint64_t *val, int index, int chn);
        virtual void getWfEngineStatus(uint32_t *val, int index, int chn);

        virtual void setWfEngineStartAddr(uint64_t val, int index, int chn);
        virtual void setWfEngineEndAddr(uint64_t val, int index, int chn);
        virtual void enableWfEngine(uint32_t val, int index, int chn);
        virtual void setWfEngineMode(uint32_t val, int index, int chn);
        virtual void setWfEngineMsgDest(uint32_t val, int index, int chn);
        virtual void setWfEngineFramesAfterTrigger(uint32_t val, int index, int chn);

        virtual void initWfEngine(int index);


};

ATCACommonFw IATCACommonFw::create(Path p)
{
    return IEntryAdapt::check_interface<ATCACommonFwAdapt, DevImpl>(p);
}


CATCACommonFwAdapt::CATCACommonFwAdapt(Key &k, ConstPath p, shared_ptr<const CEntryImpl> ie) :
    IEntryAdapt(k, p, ie),
    _p_axiVersion( p->findByName("AmcCarrierCore/AxiVersion")),
    _p_axiSysMonUltraScale( p->findByName("AmcCarrierCore/AxiSysMonUltraScale")),
    _p_bsi( p->findByName("AmcCarrierCore/AmcCarrierBsi")) 
{
    if(Gen2UpConvYaml) {   /* JESD path setup for Gen2UpConverter */
        _p_jesd0 = p->findByName("AppTop/AppTopJesd0");
        _p_jesd1 = p->findByName("AppTop/AppTopJesd1");

    } else {   /* JESD path setup for old UpConverter system */
        _p_jesd0 = p->findByName("AppTop/AppTopJesd[0]");
        _p_jesd1 = p->findByName("AppTop/AppTopJesd[1]");
    }


    _p_daqMuxV2[0] = p->findByName("AppTop/DaqMuxV2[0]");
    _p_daqMuxV2[1] = p->findByName("AppTop/DaqMuxV2[1]");

    _p_waveformEngine[0] = p->findByName("AmcCarrierCore/AmcCarrierBsa/BsaWaveformEngine[0]/WaveformEngineBuffers");
    _p_waveformEngine[1] = p->findByName("AmcCarrierCore/AmcCarrierBsa/BsaWaveformEngine[1]/WaveformEngineBuffers");

    _upTimeCnt    = IScalVal_RO::create(_p_axiVersion->findByName("UpTimeCnt"));
    _buildStamp   = IScalVal_RO::create(_p_axiVersion->findByName("BuildStamp"));
    _fpgaVersion  = IScalVal_RO::create(_p_axiVersion->findByName("FpgaVersion"));
    _fpgaTemp     = IScalVal_RO::create(_p_axiSysMonUltraScale->findByName("Temperature"));
    _EthUpTimeCnt = IScalVal_RO::create(_p_bsi->findByName("EthUpTime"));
    _GitHash      = IScalVal_RO::create(_p_axiVersion->findByName("GitHash"));


    for(int i = 0; i<MAX_JESD_CNT; i++) {
        char name[80];
        sprintf(name, JESD_CNT_STR, i);
        _jesd0ValidCnt[i] = IScalVal_RO::create(_p_jesd0->findByName(name));
        _jesd1ValidCnt[i] = IScalVal_RO::create(_p_jesd1->findByName(name));
    }

    for(int i = 0; i<MAX_DAQMUX_CNT; i++) {
        (_daqMux+i)->_triggerCasc       = IScalVal::create(_p_daqMuxV2[i]->findByName("TriggerCascMask"));
        (_daqMux+i)->_autoRearm         = IScalVal::create(_p_daqMuxV2[i]->findByName("TriggerHwAutoRearm"));
        (_daqMux+i)->_daqMode           = IScalVal::create(_p_daqMuxV2[i]->findByName("DaqMode"));
        (_daqMux+i)->_packetHeader      = IScalVal::create(_p_daqMuxV2[i]->findByName("PacketHeaderEn"));
        (_daqMux+i)->_freezeHwMask      = IScalVal::create(_p_daqMuxV2[i]->findByName("FreezeHwMask"));
        (_daqMux+i)->_decimationRateDiv = IScalVal::create(_p_daqMuxV2[i]->findByName("DecimationRateDiv"));
        (_daqMux+i)->_bufferSize        = IScalVal::create(_p_daqMuxV2[i]->findByName("DataBufferSize"));
        (_daqMux+i)->_triggerCnt        = IScalVal_RO::create(_p_daqMuxV2[i]->findByName("TrigCount"));
        (_daqMux+i)->_dbgInputValid     = IScalVal_RO::create(_p_daqMuxV2[i]->findByName("DbgInputValid"));
        (_daqMux+i)->_dbgLinkReady      = IScalVal_RO::create(_p_daqMuxV2[i]->findByName("DbgLinkReady"));

        
        for(int j = 0; j< 4; j++) {
            char name[80];
            sprintf(name, "InputMuxSel[%d]",        j); (_daqMux+i)->_inputMuxSel[j]     = IScalVal::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "StreamPause[%d]",        j); (_daqMux+i)->_streamPause[j]     = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "StreamReady[%d]",        j); (_daqMux+i)->_streamReady[j]     = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "StreamOverflow[%d]",     j); (_daqMux+i)->_streamOverflow[j]  = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "StreamError[%d]",        j); (_daqMux+i)->_streamError[j]     = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "InputDataValid[%d]",     j); (_daqMux+i)->_inputDataValid[j]  = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "StreamEnabled[%d]",      j); (_daqMux+i)->_streamEnabled[j]   = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "FrameCnt[%d]",           j); (_daqMux+i)->_frameCnt[j]        = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "FormatSignWidth[%d]",    j); (_daqMux+i)->_formatSignWidth[j] = IScalVal::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "FormatDataWidth[%d]",    j); (_daqMux+i)->_formatDataWidth[j] = IScalVal::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "FormatSign[%d]",         j); (_daqMux+i)->_formatSign[j]      = IScalVal::create(_p_daqMuxV2[i]->findByName(name));
            sprintf(name, "DecimationAveraging[%d]",j); (_daqMux+i)->_decimation[j]      = IScalVal::create(_p_daqMuxV2[i]->findByName(name));
            

        }

        for(int j = 0; j < 2; j++) {
            char name[80];
            sprintf(name, "Timestamp[%d]", j); (_daqMux+i)->_timestamp[j] = IScalVal_RO::create(_p_daqMuxV2[i]->findByName(name));
        }

        (_daqMux+i)->_triggerDaq      = ICommand::create(_p_daqMuxV2[i]->findByName("TriggerDaq"));
        (_daqMux+i)->_armHwTrigger    = ICommand::create(_p_daqMuxV2[i]->findByName("ArmHwTrigger"));
        (_daqMux+i)->_freezeBuffers   = ICommand::create(_p_daqMuxV2[i]->findByName("FreezeBuffers"));
        (_daqMux+i)->_clearTrigStatus = ICommand::create(_p_daqMuxV2[i]->findByName("ClearTrigStatus"));
    }

    for(int i = 0; i < MAX_WAVEFORMENGINE_CNT; i++) {
        (_waveformEngine+i)->_initialize = ICommand::create(_p_waveformEngine[i]->findByName("Initialize"));
        for(int j = 0; j < 4; j ++) {
            char name[80];
            sprintf(name, "StartAddr[%d]", j); (_waveformEngine+i)->_startAddr[j] = IScalVal::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "EndAddr[%d]",   j); (_waveformEngine+i)->_endAddr[j]   = IScalVal::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "WrAddr[%d]",    j); (_waveformEngine+i)->_wrAddr[j]    = IScalVal_RO::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "Enabled[%d]",   j); (_waveformEngine+i)->_enabled[j]   = IScalVal::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "Mode[%d]",      j); (_waveformEngine+i)->_mode[j]      = IScalVal::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "MsgDest[%d]",   j); (_waveformEngine+i)->_msgDest[j]   = IScalVal::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "FramesAfterTrigger[%d]", j);
                                               (_waveformEngine+i)->_framesAfterTrigger[j] = IScalVal::create(_p_waveformEngine[i]->findByName(name));
            sprintf(name, "Status[%d]",    j); (_waveformEngine+i)->_status[j]    = IScalVal_RO::create(_p_waveformEngine[i]->findByName(name));
        }
    }

}

void CATCACommonFwAdapt::createStreams(ConstPath p, const char *prefix = NULL)
{
    const char *str_stream = (!prefix)?"Stream%d":prefix;
    char path_name[80];

    for(int i = 0; i < MAX_DEBUG_STREAM; i++) {
        sprintf(path_name, str_stream, i); _stream[i] =  IStream::create(p->findByName(path_name));
    } 

}

int64_t CATCACommonFwAdapt::readStream(uint32_t index, uint8_t *buff, uint64_t size, CTimeout timeout)
{
    return _stream[index]->read(buff, size, timeout);
}


void CATCACommonFwAdapt::getUpTimeCnt(uint32_t *cnt)
{
    CPSW_TRY_CATCH(_upTimeCnt->getVal(cnt));
}

void CATCACommonFwAdapt::getBuildStamp(uint8_t *str)
{
    CPSW_TRY_CATCH(_buildStamp->getVal(str, 256));
}

void CATCACommonFwAdapt::getFpgaVersion(uint32_t *ver)
{
    CPSW_TRY_CATCH(_fpgaVersion->getVal(ver));
}

void CATCACommonFwAdapt::getFpgaTemperature(uint32_t *val)
{
    CPSW_TRY_CATCH(_fpgaTemp->getVal(val));
}

void CATCACommonFwAdapt::getEthUpTimeCnt(uint32_t *cnt)
{
    CPSW_TRY_CATCH(_EthUpTimeCnt->getVal(cnt));
}

void CATCACommonFwAdapt::getJesdCnt(uint32_t *cnt, int i, int j)
{
    if(j >= MAX_JESD_CNT) {
        *cnt = 0;
        return;
    }

    switch(i) {
        case 0:
            CPSW_TRY_CATCH(_jesd0ValidCnt[j]->getVal(cnt));
            break;
        case 1:
            CPSW_TRY_CATCH(_jesd1ValidCnt[j]->getVal(cnt));
            break;
    }
}

void CATCACommonFwAdapt::getGitHash(uint8_t *str)
{
    uint8_t githash[32];


    CPSW_TRY_CATCH(_GitHash->getVal(githash, 20));

    for(int i = 0; i < 20; i++) {
        sprintf((char*) (str+(2*i)), "%2.2x", githash[20-1-i]);
    }
}

void CATCACommonFwAdapt::triggerDaq(int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_triggerDaq->execute());
}

void CATCACommonFwAdapt::armHwTrigger(int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_armHwTrigger->execute());
}

void CATCACommonFwAdapt::freezeBuffer(int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_freezeBuffers->execute());
}

void CATCACommonFwAdapt::clearTriggerStatus(int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_clearTrigStatus->execute());
}

void CATCACommonFwAdapt::cascadedTrigger(uint32_t cmd, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_triggerCasc->setVal(cmd?1:0));
}

void CATCACommonFwAdapt::hardwareAutoRearm(uint32_t cmd, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_autoRearm->setVal(cmd?1:0));
}

void CATCACommonFwAdapt::daqMode(uint32_t cmd, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_daqMode->setVal(cmd?1:0));
}

void CATCACommonFwAdapt::enablePacketHeader(uint32_t cmd, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_packetHeader->setVal(cmd?1:0));
}

void CATCACommonFwAdapt::enableHardwareFreeze(uint32_t cmd, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_freezeHwMask->setVal(cmd?1:0));
}

void CATCACommonFwAdapt::decimationRateDivisor(uint32_t div, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_decimationRateDiv->setVal(div));
}

void CATCACommonFwAdapt::dataBufferSize(uint32_t size, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_bufferSize->setVal(size));
}

void CATCACommonFwAdapt::getTimestamp(uint32_t *sec, uint32_t *nsec, int index)
{
    try {
        (_daqMux+index)->_timestamp[0]->getVal(sec);
        (_daqMux+index)->_timestamp[1]->getVal(nsec);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
    
}


void CATCACommonFwAdapt::getTriggerCount(uint32_t *count, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_triggerCnt->getVal(count));
}


void CATCACommonFwAdapt::dbgInputValid(uint32_t *val, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_dbgInputValid->getVal(val));
}

void CATCACommonFwAdapt::dbgLinkReady(uint32_t *val, int index)
{
    CPSW_TRY_CATCH((_daqMux+index)->_dbgLinkReady->getVal(val));
}


void CATCACommonFwAdapt::inputMuxSelect(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_inputMuxSel[chn]->setVal(val));
}

void CATCACommonFwAdapt::getStreamPause(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_streamPause[chn]->getVal(val));
}

void CATCACommonFwAdapt::getStreamPause(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_streamPause[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }   
}

void CATCACommonFwAdapt::getStreamReady(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_streamReady[chn]->getVal(val));
}

void CATCACommonFwAdapt::getStreamReady(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_streamReady[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
}


void CATCACommonFwAdapt::getStreamOverflow(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_streamOverflow[chn]->getVal(val));
}

void CATCACommonFwAdapt::getStreamOverflow(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_streamOverflow[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
}

void CATCACommonFwAdapt::getStreamError(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_streamError[chn]->getVal(val));
}

void CATCACommonFwAdapt::getStreamError(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_streamError[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
}

void CATCACommonFwAdapt::getInputDataValid(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_inputDataValid[chn]->getVal(val));
}

void CATCACommonFwAdapt::getInputDataValid(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_inputDataValid[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
}

void CATCACommonFwAdapt::getStreamEnabled(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_streamEnabled[chn]->getVal(val));
}

void CATCACommonFwAdapt::getStreamEnabled(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_streamEnabled[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
}

void CATCACommonFwAdapt::getFrameCount(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_frameCnt[chn]->getVal(val));
}

void CATCACommonFwAdapt::getFrameCount(uint32_t *vals, int index)
{
    try {
        for(int i = 0; i < 4; i++) (_daqMux+index)->_frameCnt[i]->getVal(vals + i);
    } catch (CPSWError &e) {
        fprintf(stderr,"CPSW Error: %s at %s, line %d\n",
                        e.getInfo().c_str(),
                        __FILE__, __LINE__);
        throw e;
    }
}

void CATCACommonFwAdapt::formatSignWidth(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_formatSignWidth[chn]->setVal(val));
}

void CATCACommonFwAdapt::formatDataWidth(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_formatDataWidth[chn]->setVal(val));
}

void CATCACommonFwAdapt::enableFormatSign(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_formatSign[chn]->setVal(val));
}

void CATCACommonFwAdapt::enableDecimation(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_daqMux+index)->_decimation[chn]->setVal(val));
}

void CATCACommonFwAdapt::getWfEngineStartAddr(uint64_t *val, int index, int chn)
{
   CPSW_TRY_CATCH((_waveformEngine+index)->_startAddr[chn]->getVal(val));
}

void CATCACommonFwAdapt::getWfEngineEndAddr(uint64_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_endAddr[chn]->getVal(val));
}

void CATCACommonFwAdapt::getWfEngineWrAddr(uint64_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_wrAddr[chn]->getVal(val));
}

void CATCACommonFwAdapt::getWfEngineStatus(uint32_t *val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_status[chn]->getVal(val));
}

void CATCACommonFwAdapt::setWfEngineStartAddr(uint64_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_startAddr[chn]->setVal(val));
}

void CATCACommonFwAdapt::setWfEngineEndAddr(uint64_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_endAddr[chn]->setVal(val));
}

void CATCACommonFwAdapt::enableWfEngine(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_enabled[chn]->setVal(val?1:0));
}

void CATCACommonFwAdapt::setWfEngineMode(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_mode[chn]->setVal(val));
}

void CATCACommonFwAdapt::setWfEngineMsgDest(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_msgDest[chn]->setVal(val));
}

void CATCACommonFwAdapt::setWfEngineFramesAfterTrigger(uint32_t val, int index, int chn)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_framesAfterTrigger[chn]->setVal(val));
}


void CATCACommonFwAdapt::initWfEngine(int index)
{
    CPSW_TRY_CATCH((_waveformEngine+index)->_initialize->execute());
}

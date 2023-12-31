//////////////////////////////////////////////////////////////////////////////
// This file is part of 'commonATCA'.
// It is subject to the license terms in the LICENSE.txt file found in the 
// top-level directory of this distribution and at: 
//    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
// No part of 'commonATCA', including this file, 
// may be copied, modified, propagated, or distributed except according to 
// the terms contained in the LICENSE.txt file.
//////////////////////////////////////////////////////////////////////////////
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
#define MAX_AMC_CNT        2
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

        Path         _p_amcClkFreq[MAX_AMC_CNT];
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
        ScalVal_RO   _amcClkFreq[MAX_AMC_CNT];
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

        enum WFEMsgDstEnums{
            WFEMsgDstSoftware = 0,
            WFEMsgDstAutoReadOut = 1
        };

        enum WFEModeEnums{
            WFEModeWrap = 0,
            WFEModeDoneWhenFull = 1
        };

        enum WFEEnableEnums{
            WFEDisable = 0,
            WFEEnable = 1
        };

        enum DMDaqModeEnums{
            DMTriggerMode = 0,
            DMContinuousMode = 1
        };

        enum DMHWFreezeEnableEnums{
            DMHWFreezeDisable = 0,
            DMHWFreezeEnable = 1
        };

        enum DMDecimationEnableEnums{
            DMDecimationDisable = 0,
            DMDecimationEnable = 1
        };

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
        virtual void getAmcClkFreq(uint32_t *freq, int i);

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
        virtual void enableDecimationAvg(uint32_t val, int index, int chn);

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
        virtual int  setupWaveformEngine(unsigned waveFormEngineIndex, uint64_t sizeInBytes, dram_region_size_t ramAllocatedSize);
        virtual dram_region_size_t getAllocableSize(unsigned sizeInBytes);
        virtual void setupDaqMux(unsigned daqMuxIndex);

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

    /* All paths to AmcClkFreq register
       GMD: AppTop/AppCore/AmcBay1/AmcBpmCore/AmcGenericAdcDacCtrl/AmcClkFreq
       BPMC:AppTop/AppCore/AmcBay0/AmcBpmCore/AmcGenericAdcDacCtrl/AmcClkFreq 
            AppTop/AppCore/AmcBay1/AmcBpmCore/AmcGenericAdcDacCtrl/AmcClkFreq
       BPMCS: AppTop/AppCore/AmcBay0/AmcBpmCore/AmcBpmCtrl/AmcClkFreq
              AppTop/AppCore/AmcBay1/AmcBpmCore/AmcGenericAdcDacCtrl/AmcClkFreq      
       BPMSS: AppTop/AppCore/AmcBay0/AmcBpmCore/AmcBpmCtrl/AmcClkFreq
              AppTop/AppCore/AmcBay1/AmcBpmCore/AmcBpmCtrl/AmcClkFreq
       MPS:   AppTop/AppCore/AmcGenericAdcDacCore[0]/AmcGenericAdcDacCtrl/AmcClkFreq
              AppTop/AppCore/AmcGenericAdcDacCore[1]/AmcGenericAdcDacCtrl/AmcClkFreq
       BPM:   AppTop/AppCore/AmcGenericAdcDacCore[0]/AmcGenericAdcDacCtrl/AmcClkFreq
              AppTop/AppCore/AmcGenericAdcDacCore[1]/AmcGenericAdcDacCtrl/AmcClkFreq
       BCM:   AppTop/AppCore/AmcGenericAdcDacCore[0]/AmcGenericAdcDacCtrl/AmcClkFreq
              AppTop/AppCore/AmcGenericAdcDacCore[1]/AmcGenericAdcDacCtrl/AmcClkFreq
       BLEN:  AppTop/AppCore/AmcGenericAdcDacCore[0]/AmcGenericAdcDacCtrl/AmcClkFreq
              AppTop/AppCore/AmcGenericAdcDacCore[1]/AmcGenericAdcDacCtrl/AmcClkFreq
       LLRF:  AMC1: AppTop/AppCore/AmcMrLlrfGen2UpConvert/AmcClkFreq
              AMC0: AppTop/AppCore/AmcMrLlrfDownConvert/AmcClkFreq
              AMC1: AppTop/AppCore/AmcMrLlrfUpConvert/AmcClkFreq
       LL:    AMC1: AppTop/AppCore/AmcMrLlrfGen2UpConvert/AmcClkFreq
              AMC0: AppTop/AppCore/AmcMrLlrfDownConvert/AmcClkFreq
              AMC1: AppTop/AppCore/AmcMrLlrfUpConvert/AmcClkFreq

       */
    #define LLRF_UPG1 0 /* AMC1=DAQMUX1 */
    #define LLRF_UPG2 1 /* AMC1=DAQMUX1 */
    #define LLRF_DOWN 2 /* AMC0=DAQMUX0 */


    #define DAQMUX0 0
    #define DAQMUX1 1

    for(int daqMuxIndex = 0; daqMuxIndex< MAX_AMC_CNT; daqMuxIndex++) {
        char path[3][100];
        sprintf(path[0], "AppTop/AppCore/AmcBay%d/AmcBpmCore/AmcGenericAdcDacCtrl/AmcClkFreq", daqMuxIndex); 
        sprintf(path[1], "AppTop/AppCore/AmcBay%d/AmcBpmCore/AmcBpmCtrl/AmcClkFreq", daqMuxIndex);
        sprintf(path[2], "AppTop/AppCore/AmcGenericAdcDacCore[%d]/AmcGenericAdcDacCtrl/AmcClkFreq", daqMuxIndex);
        for (unsigned int AmcClkFreqRegIndex = 0; AmcClkFreqRegIndex < 3; AmcClkFreqRegIndex++)
        {
            try{
                _p_amcClkFreq[daqMuxIndex] = p->findByName(path[AmcClkFreqRegIndex]);
                break;
            } catch (...){
                /* Did not find the AMC frequency register. Try another. */
            }
        }
    }

    char path[3][100];
    sprintf(path[LLRF_UPG2], "AppTop/AppCore/AmcMrLlrfGen2UpConvert/AmcClkFreq");
    sprintf(path[LLRF_DOWN], "AppTop/AppCore/AmcMrLlrfDownConvert/AmcClkFreq");
    sprintf(path[LLRF_UPG1], "AppTop/AppCore/AmcMrLlrfUpConvert/AmcClkFreq");    
    /* LLRF_DOWN */
    if (_p_amcClkFreq[DAQMUX0] == NULL)
    {
        try{
            _p_amcClkFreq[DAQMUX0] = p->findByName(path[LLRF_DOWN]);
        } catch (...){
            /* Did not find the AMC frequency register. Try another. */
        }
    }
    /* LLRF_UPG1 & LLRF_UPG2 */
    if (_p_amcClkFreq[DAQMUX1] == NULL)
    {
        try{
            _p_amcClkFreq[DAQMUX1] = p->findByName(path[LLRF_UPG1]);
        } catch (...){
            try{
                _p_amcClkFreq[DAQMUX1] = p->findByName(path[LLRF_UPG2]);
            } catch (...){
                /* Did not find the AMC frequency register. Try another. */
            }
        }
    }    

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

    for(int i = 0; i<MAX_AMC_CNT; i++) {
        if (_p_amcClkFreq[i] != NULL)
            _amcClkFreq[i] = IScalVal_RO::create(_p_amcClkFreq[i]);
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


void CATCACommonFwAdapt::getAmcClkFreq(uint32_t *freq, int i)
{
    if (_p_amcClkFreq[i] != NULL)
    {
        _amcClkFreq[i]->getVal(freq);
        *freq *= 2;
    } else {
        *freq = 0;
    }
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

void CATCACommonFwAdapt::enableDecimationAvg(uint32_t val, int index, int chn)
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

dram_region_size_t CATCACommonFwAdapt::getAllocableSize(unsigned sizeInBytes)
{
     if (sizeInBytes <= 0x10000000)
     {
        return twogb;
     } else if (sizeInBytes > 0x10000000 && sizeInBytes <= 0x20000000) 
     {
        return fourgb;
     } else
     {
        return eightgb;
     }
}

int CATCACommonFwAdapt::setupWaveformEngine(unsigned waveformEngineIndex, uint64_t sizeInBytes, dram_region_size_t ramAllocatedSize)
{
    uint32_t framesAfterTriggerVal = 0;
    uint64_t start;
    uint64_t step;
    uint64_t memoryPerWaveformEngine, waveFormEngineBase, totalMemoryAllocated;
    /**
     * sizeInBytes <= 256 MB (268435456) use address space 0x0x0000000100000000 - 0x0000000180000000 (2GB)
     * sizeInBytes > 256 MB &&  < 512 MB use address space 0x0x0000 0001 0000 0000 - 0x0000 0002 0000 0000 (4GB)
     * sizeInBytes > 512 MB (536870912) use address space 0x0x0000 0000 0000 0000 - 0x0000 0002 0000 0000 (8GB)
     **/

     if (sizeInBytes <= 0x10000000 && (ramAllocatedSize == autogb || ramAllocatedSize == twogb)) // Allocate 2 GB
     {
        totalMemoryAllocated = 0x80000000;
        waveFormEngineBase = 0x0000000100000000;
     } else if ( (sizeInBytes > 0x10000000 && sizeInBytes <= 0x20000000) || 
                 (sizeInBytes <= 0x20000000 && ramAllocatedSize == fourgb )
                 ) // Allocate 4GB
     {
        totalMemoryAllocated = 0x100000000;
        waveFormEngineBase = 0x0000000100000000;
     } else if (( sizeInBytes > 0x20000000  && ramAllocatedSize == autogb) ||
                  ramAllocatedSize == eightgb)  // sizeInBytes > 0x20000000 -- Allocate 8GB
     {
        totalMemoryAllocated = 0x200000000;
        waveFormEngineBase = 0x0000000000000000;
     } else {
        return -1;
     }

    memoryPerWaveformEngine = totalMemoryAllocated >> 1;
    start = waveFormEngineBase + waveformEngineIndex * memoryPerWaveformEngine;
    step  = totalMemoryAllocated >> 3; 

    if (waveformEngineIndex != 0 && waveformEngineIndex != 1)
        return -1;

    for(int j = 0; j < 4; j++) {
        CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_startAddr[j]->setVal(start));
        CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_endAddr[j]->setVal(start + sizeInBytes));
        CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_framesAfterTrigger[j]->setVal(framesAfterTriggerVal));
        CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_enabled[j]->setVal(WFEEnable));
        CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_mode[j]->setVal(WFEModeDoneWhenFull)); 
        CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_msgDest[j]->setVal(WFEMsgDstAutoReadOut));

        start += step;
    }
    CPSW_TRY_CATCH((_waveformEngine+waveformEngineIndex)->_initialize->execute());
    return 0;
}

void CATCACommonFwAdapt::setupDaqMux(unsigned daqMuxIndex)
{

    if (daqMuxIndex != 0 && daqMuxIndex != 1)
        return;

    CPSW_TRY_CATCH((_daqMux+daqMuxIndex)->_clearTrigStatus->execute());
    CPSW_TRY_CATCH((_daqMux+daqMuxIndex)->_daqMode->setVal(DMTriggerMode));
    CPSW_TRY_CATCH((_daqMux+daqMuxIndex)->_freezeHwMask->setVal(DMHWFreezeDisable));
    CPSW_TRY_CATCH((_waveformEngine+daqMuxIndex)->_initialize->execute());
    CPSW_TRY_CATCH((_daqMux+daqMuxIndex)->_packetHeader->setVal(true?1:0));

}
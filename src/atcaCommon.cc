#include "atcaCommon.h"

#include <cpsw_yaml.h>
#include <yaml-cpp/yaml.h>
#include <cpsw_sval.h>
#include <cpsw_entry_adapt.h>
#include <cpsw_hub.h>
#include <fstream>
#include <sstream>

#include <string.h>
#include <math.h>

#define MAX_JESD_CNT       8
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


class CATCACommonFwAdapt;
typedef shared_ptr<CATCACommonFwAdapt> ATCACommonFwAdapt;

class CATCACommonFwAdapt : public IATCACommonFw, public IEntryAdapt {
    protected:
        Path         _p_axiVersion;
        Path         _p_bsi;
        Path         _p_jesd0;
        Path         _p_jesd1;
        ScalVal_RO   _upTimeCnt;
        ScalVal_RO   _buildStamp;
        ScalVal_RO   _fpgaVersion;
        ScalVal_RO   _EthUpTimeCnt;
        ScalVal_RO   _jesd0ValidCnt[MAX_JESD_CNT];
        ScalVal_RO   _jesd1ValidCnt[MAX_JESD_CNT];

    public:
        CATCACommonFwAdapt(Key &k, ConstPath p, shared_ptr<const CEntryImpl> ie);
        virtual void getUpTimeCnt(uint32_t *cnt);
        virtual void getBuildStamp(uint8_t *str);
        virtual void getFpgaVersion(uint32_t *ver);
        virtual void getEthUpTimeCnt(uint32_t *cnt);
        virtual void getJesdCnt(uint32_t *cnt, int i, int j);
};

ATCACommonFw IATCACommonFw::create(Path p)
{
    return IEntryAdapt::check_interface<ATCACommonFwAdapt, DevImpl>(p);
}


CATCACommonFwAdapt::CATCACommonFwAdapt(Key &k, ConstPath p, shared_ptr<const CEntryImpl> ie) :
    IEntryAdapt(k, p, ie),
    _p_axiVersion( p->findByName("AmcCarrierCore/AxiVersion")),
    _p_bsi( p->findByName("AmcCarrierCore/AmcCarrierBsi")),
    _p_jesd0( p->findByName("AppTop/AppTopJesd[0]")),
    _p_jesd1( p->findByName("AppTop/AppTopJesd[1]"))
{
    _upTimeCnt    = IScalVal_RO::create(_p_axiVersion->findByName("UpTimeCnt"));
    _buildStamp   = IScalVal_RO::create(_p_axiVersion->findByName("BuildStamp"));
    _fpgaVersion  = IScalVal_RO::create(_p_axiVersion->findByName("FpgaVersion"));
    _EthUpTimeCnt = IScalVal_RO::create(_p_bsi->findByName("EthUpTime"));
    

    for(int i = 0; i<MAX_JESD_CNT; i++) {
        char name[80];
        sprintf(name, JESD_CNT_STR, i);
        _jesd0ValidCnt[i] = IScalVal_RO::create(_p_jesd0->findByName(name));
        _jesd1ValidCnt[i] = IScalVal_RO::create(_p_jesd1->findByName(name));
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


void CATCACommonFwAdapt::getEthUpTimeCnt(uint32_t *cnt)
{
    CPSW_TRY_CATCH(_EthUpTimeCnt->getVal(cnt));
}

void CATCACommonFwAdapt::getJesdCnt(uint32_t *cnt, int i, int j)
{
    switch(i) {
        case 0:
            CPSW_TRY_CATCH(_jesd0ValidCnt[j]->getVal(cnt));
            break;
        case 1:
            CPSW_TRY_CATCH(_jesd1ValidCnt[j]->getVal(cnt));
            break;
    }
}

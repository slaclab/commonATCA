#ifndef _ATCA_COMMON_FW_H
#define _ATCA_COMMON_FW_H

#include <cpsw_api_user.h>
#include <cpsw_api_builder.h>

class IATCACommonFw;
typedef shared_ptr<IATCACommonFw> ATCACommonFw;

class IATCACommonFw : public virtual IEntry {
public:
    static ATCACommonFw create(Path p);
    virtual void getUpTimeCnt(uint32_t *cnt)             = 0;
    virtual void getBuildStamp(uint8_t *str)             = 0;
    virtual void getFpgaVersion(uint32_t *ver)           = 0;
    virtual void getEthUpTimeCnt(uint32_t *cnt)          = 0;
    virtual void getJesdCnt(uint32_t *cnt, int i, int j) = 0;

};

#endif /* _ATCA_COMMON_FW_H */

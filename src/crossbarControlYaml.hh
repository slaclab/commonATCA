//////////////////////////////////////////////////////////////////////////////
// This file is part of 'commonATCA'.
// It is subject to the license terms in the LICENSE.txt file found in the 
// top-level directory of this distribution and at: 
//    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
// No part of 'commonATCA', including this file, 
// may be copied, modified, propagated, or distributed except according to 
// the terms contained in the LICENSE.txt file.
//////////////////////////////////////////////////////////////////////////////
#ifndef CROSSBAR_CONTROL_YAML_H
#define CROSSBAR_CONTROL_YAML_H

#include <cpsw_api_builder.h>

#include <stdint.h>
#include <vector>


namespace CrossbarControl {

    class CrossbarControlYaml {
        public:
            CrossbarControlYaml(Path core);
            uint32_t GetOutputConfig0(void);
            uint32_t GetOutputConfig1(void);
            uint32_t GetOutputConfig2(void);
            uint32_t GetOutputConfig3(void);
            
            void     SetOutputConfig0(uint32_t output);
            void     SetOutputConfig1(uint32_t output);
            void     SetOutputConfig2(uint32_t output);
            void     SetOutputConfig3(uint32_t output);
            
            
        protected:
            Path     _path;
            ScalVal  _outputConfig0;
            ScalVal  _outputConfig1;
            ScalVal  _outputConfig2;
            ScalVal  _outputConfig3;
    };

};  /* namespace CrossbarControl */

#endif /* CROSSBAR_CONTROL_YAML_H */
#pragma once

namespace Emulator {
    class Cpu {
    private:
        Registers regs();
        Bus bus();
    
    public:
        enum InstrType {
            R,  //Register type
            I,  //Immediate type
            S,  //Store type
            B,  //Break type
            U,  //Register type
            J   //Jump type
        };

        template<InstrType IT>
        void handler(uint32_t instr);


        void (*handlers)(uint32_t) = {
            extern handler<R>, extern handler<I>,
            extern handler<S>, extern handler<B>,
            extern handler<U>, extern handler<J>
        };

        inline void operator()(uint64_t instr)
        {

        }
    };
};
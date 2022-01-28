class MemoryStage: public Stage
{
   private: 
   uint64_t valM;

void setWInput(W * wreg, uint64_t stat, uint64_t icode, 
                            uint64_t valE, uint64_t valM, 
                           uint64_t dstE, uint64_t dstM
                            );
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t addr(PipeReg ** pregs, uint64_t icode, uint64_t valE, uint64_t valA);
      bool memRead(uint64_t icode);
      bool memWrite(uint64_t icode);
      uint64_t getm_valM();
};



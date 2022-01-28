class DecodeStage: public Stage
{
   private: 


      void setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                     uint64_t ifun, uint64_t valC, uint64_t valA,
                     uint64_t valB, uint64_t dstE, uint64_t dstM,
                     uint64_t srcA, uint64_t srcB);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t srcAComponent(D * dreg, uint64_t d_icode);
      uint64_t srcBComponent(D * dreg, uint64_t d_icode);
      uint64_t dstEComponent(D * dreg, uint64_t d_icode);
      uint64_t dstMComponent(D * dreg, uint64_t d_icode);
      uint64_t selFwdA(PipeReg ** pregs, uint64_t srcA, uint64_t icode, Stage ** stages);
      uint64_t fwdB(PipeReg ** pregs, uint64_t srcB, uint64_t icode, Stage ** stages, uint64_t dstE);
};


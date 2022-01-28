class ExecuteStage: public Stage
{
   private: 
      void setMInput(M * mreg, uint64_t stat, uint64_t icode, 
                           uint64_t cnd, uint64_t valE, uint64_t valA,
                           uint64_t dstE, uint64_t dstM);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t gete_dstE();
      uint64_t gete_valE();
      uint64_t aluA (PipeReg ** pregs, uint64_t e_icode);
      uint64_t aluB (PipeReg ** pregs, uint64_t e_icode);
      uint64_t aluFun(PipeReg ** pregs, uint64_t e_icode);
      bool setCc(PipeReg ** pregs, uint64_t e_icode);      
      uint64_t e_dstE(PipeReg ** pregs, uint64_t e_icode, uint64_t e_cnd);
      void ccCircuit(bool cc,  bool needSF, bool needZF, bool needOF);
      uint64_t aluCircuit(uint64_t aluF, uint64_t aComp, uint64_t bComp);
      uint64_t cond(uint64_t icode, uint64_t ifun);
};


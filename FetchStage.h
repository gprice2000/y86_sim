//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
   public:
      uint64_t selectPC(PipeReg ** pregs);
      
      bool needRegIds(uint64_t f_icode);
      bool needValC(uint64_t f_icode);
      uint64_t predictPC(uint64_t f_valP, uint64_t f_icode, uint64_t f_valC );
      uint64_t pcIncrement(uint64_t f_pc, bool needReg, bool needC);
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      void getRegIds(bool needReg, PipeReg** pregs, uint64_t & rA, uint64_t & rB);
      uint64_t buildValC(uint32_t f_pc, bool needC, PipeReg** pregs, uint64_t valC);
};

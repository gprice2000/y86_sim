#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "DecodeStage.h"
#include "Instructions.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"

RegisterFile * regInstance = RegisterFile::getInstance();

/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{

 D * dreg = (D *) pregs[DREG];
 E * ereg = (E *) pregs[EREG];

 uint64_t stat =  dreg->getstat()->getOutput(), icode = dreg->geticode()->getOutput(), ifun = dreg->getifun()->getOutput(), valC = dreg->getvalC()->getOutput();
 uint64_t dstE = dstEComponent(dreg, icode), dstM = dstMComponent(dreg, icode), srcA = srcAComponent(dreg, icode), srcB = srcBComponent(dreg, icode); 
 uint64_t valB = fwdB(pregs, srcB, icode, stages, dstE), valA = selFwdA(pregs, srcA, icode, stages);

 setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
 return true;  
}
/*
* doClockHigh
 * applies the appropriate control signal to the D
 * and E register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
   E * ereg = (E *) pregs[EREG];

   ereg->getstat()->normal();
   ereg->geticode()->normal();
   ereg->getifun()->normal();
   ereg->getvalC()->normal();
   ereg->getvalA()->normal();
   ereg->getvalB()->normal();
   ereg->getdstE()->normal();
   ereg->getdstM()->normal();
   ereg->getsrcA()->normal();
   ereg->getsrcB()->normal();
}
/* setEInput
 * provides the input to potentially be stored in the E register
 * during doClockHigh
 *
 * @param: dreg - pointer to the E register instance
 * @param: stat - value to be stored in the stat pipeline register within E
 * @param: icode - value to be stored in the icode pipeline register within E
 * @param: ifun - value to be stored in the ifun pipeline register within E
 * @param: valC - value to be stored in the valC pipeline register within E
 * @param: valA - value to be stored in the valA pipeline register within E
 * @param: valB - value to be stored in the valB pipeline register within E
 * @param: dstE- value to be stored in the dstE pipeline register within E
 * @param: dstM- value to be stored in the dstM pipeline register within E
 * @param: srcA- value to be stored in the srcA pipeline register within E
 * @param: srcB- value to be stored in the srcB pipeline register within E
*/

//Need to change vals from uint64_t to PipeRegField* or convert PipeRegField* to uint64_t
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM,
                            uint64_t srcA, uint64_t srcB)
{
   ereg->getstat()->setInput(stat);
   ereg->geticode()->setInput(icode);
   ereg->getifun()->setInput(ifun);
   ereg->getvalC()->setInput(valC);
   ereg->getvalA()->setInput(valA);
   ereg->getvalB()->setInput(valB);
   ereg->getdstE()->setInput(dstE);
   ereg->getdstM()->setInput(dstM);
   ereg->getsrcA()->setInput(srcA);
   ereg->getsrcB()->setInput(srcB);
   
}
/*
*  srcAComponent 
 * checks icode and returns source register rA, if instruction requires it, RSP if required, or RNONE
 * 
 * @param: dreg - pointer to D pipeline register
 * @param d_icode is the icode for instruction
 * 
 * @return rA if icode is IIRMOVQ, IRMMOVQ, IOPQ, or IPUSHQ
 * @return RSP if icode is IPOPQ or IRET
 * @return RNONE otherwise 
 */
uint64_t DecodeStage::srcAComponent(D * dreg, uint64_t d_icode){

   if(d_icode == IRRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ || d_icode == IPUSHQ){
      return dreg->getrA()->getOutput();
   }
   else if(d_icode == IPOPQ || d_icode == IRET){
      return RSP;
   }
   else{
      return RNONE;
   }
}
/*
*  srcBComponent 
 * checks icode and returns source register rB, if instruction requires it, RSP if required, or RNONE
 * 
 * @param: dreg - pointer to D pipeline register
 * @param d_icode is the icode for instruction
 * 
 * @return rB if icode is IMRMOVQ,IRMMOVQ, or IOPQ, or IPUSHQ
 * @return RSP if icode is IPOPQ, IPUSHQ or ICALL
 * @return RNONE otherwise 
 */
uint64_t DecodeStage::srcBComponent(D * dreg, uint64_t d_icode){
    if (d_icode == IOPQ|| d_icode == IRMMOVQ || d_icode == IMRMOVQ){
      return dreg->getrB()->getOutput();
   }
   else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET){
      return RSP;
   }
   else{
      return RNONE;
   }

}
/*
 * dstEComponent
 * checks icode and returns destination register rB, if instruction requires it, RSP if required, or RNONE
 * 
 * @param: dreg - pointer to D pipeline register
 * @param d_icode is the icode for instruction
 * 
 * @return rB if icode is IRRMOVQ, IIRMOVQ, or IOPQ 
 * @return RSP if icode is IPOPQ, IPUSHQ or ICALL
 * @return RNONE otherwise 
 */
uint64_t DecodeStage::dstEComponent(D * dreg, uint64_t d_icode){
   if (d_icode == IRRMOVQ || d_icode == IIRMOVQ || d_icode == IOPQ){

      return dreg->getrB()->getOutput();
   }
   else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET){

      return RSP;
   }
   else{

      return RNONE;
   }
}
/*
 *  dstMComponent 
 * checks icode and returns destination source register rA, if instruction requires it, RSP if required, or RNONE
 * 
 * @param: dreg - pointer to D pipeline register
 * @param d_icode is the icode for instruction
 * 
 * @return rA if icode is IMRMOVQ or IPOPQ 
 * @return RNONE otherwise 
 */
uint64_t DecodeStage::dstMComponent(D * dreg, uint64_t d_icode){
   if (d_icode == IMRMOVQ || d_icode == IPOPQ){
      return dreg->getrA()->getOutput();
   }
   else{
      return RNONE;
   }
}
/*
 * selFwdA
 * checks srcA and returns it's corresponding register in registerfile's value 
 * 
 * @param srcA is the srcA component 
 * @param pregs is the array of pipeline registers
 * @return srcA's corresponding registerfile value 
 */
uint64_t DecodeStage::selFwdA(PipeReg ** pregs,uint64_t srcA, uint64_t icode, Stage ** stages){

   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   D * dreg = (D *) pregs[DREG];

   ExecuteStage * exec = (ExecuteStage*) stages[ESTAGE];
   MemoryStage * memStage = (MemoryStage*) stages[MSTAGE];

   bool error = false;

   if(icode == ICALL || icode == IJXX) return dreg->getvalP()->getOutput();
   if(srcA == RNONE) return 0;
   if(srcA == exec->gete_dstE()) return exec->gete_valE();
   if(srcA == mreg->getdstM()->getOutput()) return memStage->getm_valM();
   if(srcA == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
   if(srcA == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();
   if(srcA == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();
   return regInstance->readRegister(srcA, error);
}
/*
 * fwdB 
 * checks srcB and returns it's corresponding register in registerfile's value 
 * 
 * @param srcB is the srcB component 
 * 
 * @return srcB's corresponding registerfile value
 */
uint64_t DecodeStage::fwdB(PipeReg ** pregs, uint64_t srcB, uint64_t icode, Stage ** stages, uint64_t dstE){


   bool error = false;

   ExecuteStage * exec = (ExecuteStage*) stages[ESTAGE];
   MemoryStage * memStage = (MemoryStage*) stages[MSTAGE];
    
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];

   if (srcB == RNONE) return 0;
   if(srcB == exec->gete_dstE()) return exec->gete_valE();
   if(srcB == mreg->getdstM()->getOutput() ) return memStage->getm_valM();
   if(srcB == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
   if(srcB == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();
   if(srcB == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();
  
   return regInstance->readRegister(srcB, error);

}
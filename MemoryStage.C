#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "MemoryStage.h"
#include "Instructions.h"
#include "Memory.h"
#include "Tools.h"

/*
 * doClockLow:
 * Performs the Memory stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */

Memory * mem_instance2 = Memory::getInstance();
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
 bool error = false;
 M * mreg = (M *) pregs[MREG];
 W * wreg = (W *) pregs[WREG];

 uint64_t stat =  mreg->getstat()->getOutput(), icode = mreg->geticode()->getOutput(),
                      valE= mreg->getvalE()->getOutput(), dstE= mreg->getdstE()->getOutput(), dstM = mreg->getdstM()->getOutput();
 uint64_t address = addr(pregs, icode, valE, mreg->getvalA()->getOutput());

 //reset valM
 valM = 0;

 if(memRead(icode)) valM = mem_instance2->getLong(address, error);
 if(memWrite(icode)) mem_instance2->putLong(mreg->getvalA()->getOutput(), address, error);
 setWInput(wreg, stat, icode, valE, valM,  dstE, dstM);
 return true;  
  
}
/*
* doClockHigh
 * applies the appropriate control signal to the M
 * and W register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{

   W * wreg = (W *) pregs[WREG];

   wreg->getstat()->normal();
   wreg->geticode()->normal();
   wreg->getvalE()->normal();
   wreg->getvalM()->normal();
   wreg->getdstE()->normal();
   wreg->getdstM()->normal();
}
/* setWInput
 * provides the input to potentially be stored in the W register
 * during doClockHigh
 *
 * @param: wreg - pointer to the W register instance
 * @param: stat - value to be stored in the stat pipeline register within W
 * @param: icode - value to be stored in the icode pipeline register within W
 * @param: valE - value to be stored in the valE pipeline register within W
 * @param: valM - value to be stored in the valM pipeline register within W
 * @param: dstE- value to be stored in the dstE pipeline register within W
 * @param: dstM- value to be stored in the dstM pipeline register within W
*/

void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, 
                            uint64_t valE, uint64_t valM, 
                           uint64_t dstE, uint64_t dstM
                            )
{
   wreg->getstat()->setInput(stat);
   wreg->geticode()->setInput(icode);
   wreg->getvalE()->setInput(valE);
   wreg->getvalM()->setInput(valM);
   wreg->getdstE()->setInput(dstE);
   wreg->getdstM()->setInput(dstM);
}

/* addr
* code for the addr component in pipeline
* 
* @param pregs is the array of pipeline registerss
* @param icode is the current instruction code in memory stage
*
* @return valE if icode is IRMMOVQ, IPUSHQ, ICALL or IMRMOVQ
* @return valA if icode is IPOPQ or IRET
* @return 0 otherwise
*/
uint64_t MemoryStage::addr(PipeReg ** pregs, uint64_t icode, uint64_t valE, uint64_t valA){

   switch(icode){

      case(IRMMOVQ):
      case(IPUSHQ):
      case(ICALL):
      case(IMRMOVQ):
       return valE;
       break;

      case (IPOPQ):
      case (IRET): 
       return valA;
       break;

      default:
       return 0;
       break;
   }

}
/* memRead
* takes icode as input and returns true if instruction needs to read from memory
* 
* @param icode is the instruction code of current instruction in memory stage
*
* @return true if icode is IMRMOVQ, IPOPQ or IRET. False otherwise
*/
bool MemoryStage::memRead(uint64_t icode){
  return (icode == IMRMOVQ || icode == IPOPQ || icode == IRET);
}
/* memWrite 
* takes icode as input and returns true if instruction needs to write to memory 
* 
* @param icode is the instruction code of current instruction in memory stage
*
* @return true if icode is IRMMOVQ, IPUSHQ, or ICALL 
*/
bool MemoryStage::memWrite(uint64_t icode){
  return (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL);
}
/* getm_valM
* returns valM  
* @return valM which is valM in memory stage register
*/
uint64_t MemoryStage::getm_valM(){
   return valM;
}
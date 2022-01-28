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
#include "Memory.h"
#include "Tools.h"
#include "Instructions.h"

   Memory * mem_instance = Memory::getInstance();
/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   uint64_t   icode = 0, ifun = 0, valC = 0 ;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;


   bool error = false; 

   int32_t f_pc = selectPC(pregs);

   //Obtain byte to split into icode and ifun into memory
   //build a long with just memByte to be able to use getBits method
   uint8_t memByte = mem_instance->getByte(f_pc, error);
   uint8_t byteArr [] = {memByte, 0, 0,0,0,0,0,0};
   uint64_t memLong = Tools::buildLong(byteArr);

   icode = (Tools::getBits(memLong,4, 7));
   ifun= (Tools::getBits(memLong, 0, 3));


   //call needRegIds() and needValC() to pass input to PCincrement
   //PC increment takes f_pc, result of needRegIds() and needValC() and calcs addr of next instruction
   //store result of PCIncrement in valP

   bool needReg = needRegIds(icode);
   bool needC =  needValC(icode);

   uint64_t valP = pcIncrement(f_pc,needReg ,needC);
   
   //Pass valP to predictPC along with icode and valC (currently 0)
   //pass output of predictPC to input to F_predPC register
   //The value passed to setInput below will need to be changed

   freg->getpredPC()->setInput(predictPC(valP, icode, valC));

   getRegIds(needReg, pregs, rA, rB);
   valC=buildValC(f_pc, needC, pregs, valC);

   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}
/* selectPC
* obtains value of f_pc
*
* @param: pregs - array of pipeline register (F, D, E, M, W instances)
*/
uint64_t FetchStage::selectPC(PipeReg ** pregs){

        F * freg = (F * ) pregs[FREG];
       M * mreg = (M *) pregs[MREG];
       W * wreg = (W * ) pregs[WREG];
//M_icode is equal to !M_Cnd && IJXX icode
   uint64_t m_icode = mreg->geticode()->getOutput();
       if (m_icode ==  IJXX && m_icode ==  !(mreg->getCnd()->getOutput()) ){
                return mreg->getvalA()->getOutput();
       }
//W_icode equal to icode of ret instruction
       else if(wreg->geticode()->getOutput() == IRET ){
               return wreg->getvalM()->getOutput(); 
      }
      else{
       return freg->getpredPC()->getOutput();
       }

}
/* needRegIds
* returns true if icode includes a register specifier byte
* 
* 
* @param f_icode is the icode of f register
* @return true if f_icode belongss to an instruction that contains a register specifier byte 
*/
bool FetchStage::needRegIds(uint64_t f_icode){
      //Check if f_icode matches any icodes of instructions that contain a register specifier byte 
       return (f_icode == IRRMOVQ || f_icode == IOPQ ||  f_icode == IPUSHQ || f_icode == IPOPQ || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ); 
}
/* needValC
* determines if instruction includes a constant word
*
* @param f_icode is the icode in the f register
*
* @return true if f_code belongs to an instruction that includes constant word 
*/
bool FetchStage::needValC(uint64_t f_icode){
 return(f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IJXX || f_icode == ICALL);
}

/* predictPC
* predicts PC for next instruction
*
* returns f_valC if f_icode belongs to JXX or CALL instructions, returns f_valP otherwise
*
* @param f_valP is the valP value of the f register
* @param f_icode is the icode of the f register
* @param f_valC is the valC value of the f register
* 
* @return f_valC if f_icode belongs to JXX or CALL instructions, returns f_valP otherwise
*/
uint64_t FetchStage::predictPC(uint64_t f_valP, uint64_t f_icode, uint64_t f_valC ){

      if(f_icode == IJXX || f_icode == ICALL ) return f_valC;

      return f_valP; 
}
/*pcIncrement
* increments program counter
* 
* @param f_pc is the current pc
* @param needReg is boolean for if current instruction contains a register specifier byte
* @param needC is boolean for if current instruction contains 64 bit constant word
*
* return updated program counter
*/
uint64_t FetchStage::pcIncrement(uint64_t f_pc, bool needReg, bool needC){
/*The PC incrementer hardware unit generates the signal valP, based on the current value 
of the PC, and the two signals need_regids and need_valC. For PC value p, need_regids value r, 
and need_valC value i, the incrementer generates the value p + 1 + r + 8i.
*/

         return (f_pc + 1 + needReg + (8 * needC));
}


/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];

   freg->getpredPC()->normal();
   dreg->getstat()->normal();
   dreg->geticode()->normal();
   dreg->getifun()->normal();
   dreg->getrA()->normal();
   dreg->getrB()->normal();
   dreg->getvalC()->normal();
   dreg->getvalP()->normal();
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}
/* getRegIds
* if instruction contains register specifier byte, set rA to first part of byte and rB to second
* @param needReg is a boolean signifying if the instruction contains a register specifier byte
* @param pregs is the array of PipeRegs
* @param rA is the source register in instruction
* @param rB is the destination register in instruction
*/
void FetchStage::getRegIds(bool needReg,PipeReg** pregs,uint64_t & rA, uint64_t & rB){
   bool error = false;
   F * freg = (F *) pregs[FREG];
   if(needReg){
      uint8_t regByte= mem_instance->getByte(freg->getpredPC()->getOutput() + 1, error);
      uint8_t byteArr [] = {regByte, 0, 0,0,0,0,0,0};
      uint64_t regLong= Tools::buildLong(byteArr);

      rA = (Tools::getBits(regLong,4, 7));
      rB= (Tools::getBits(regLong, 0, 3));     
   }
}

/*buildValC
* if instruction contains a value to be stored, build a long and stores it in valC
* @param needC is a boolean signifying if instruction contains an immediate value
* @ param pregs is the array of PipeRegs
*
* @return newValC is the value to be stored in valC
*/

uint64_t FetchStage:: buildValC(uint32_t f_pc, bool needC, PipeReg ** pregs, uint64_t valC){
   bool error = false;

   if(needC){

   uint8_t byteArr[8];

   for(int i = 9; i>=2;i--){
      byteArr[i-2] = mem_instance->getByte(f_pc + i,  error);
   } 
      uint64_t newValC =  Tools::buildLong(byteArr); 

      return newValC;

   }
   else{
   return valC;
   }

}
     

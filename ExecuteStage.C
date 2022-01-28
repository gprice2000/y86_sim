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
#include "ExecuteStage.h"
#include "Instructions.h"
#include "ConditionCodes.h"
#include "Tools.h"

ConditionCodes * cc_instance = ConditionCodes::getInstance();
RegisterFile * reggie  = RegisterFile::getInstance();

//Used as fields to enable forwarding
static uint64_t valE;
static uint64_t dstE;

/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    E * ereg = (E *) pregs[EREG]; 

    uint64_t stat = ereg->getstat()->getOutput(), icode = ereg->geticode()->getOutput() ;
    uint64_t valA = ereg->getvalA()->getOutput(), dstM = ereg->getdstM()->getOutput();


    uint64_t aluAComp = aluA(pregs, icode);
    uint64_t aluBComp = aluB(pregs, icode);
    uint64_t functionCode = aluFun(pregs,icode); 
    valE = aluCircuit(functionCode,aluAComp ,aluBComp);

    bool setCondition = setCc(pregs, icode);
    bool needSF = Tools::sign(valE);
    bool needZF = (valE == 0); 
    bool needOF = 0;

    if(functionCode ==  0) needOF = Tools::addOverflow(aluAComp, aluBComp);
    if(functionCode ==  1) needOF = Tools::subOverflow(aluAComp, aluBComp);

    ccCircuit(setCondition, needSF, needZF, needOF);

    uint64_t cnd = cond(icode, ereg->getifun()->getOutput());
    dstE = e_dstE(pregs, icode, cnd) ;  

    setMInput(mreg, stat, icode, cnd, valE, valA, dstE, dstM);

    return true;
}

/*
* doClockHigh
 * applies the appropriate control signal to the M
 * and E register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */

void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
    
    M * Mreg = (M *) pregs[MREG];
    
    Mreg->getstat()->normal();
    Mreg->geticode()->normal();
    Mreg->getCnd()->normal();
    Mreg->getvalE()->normal();
    Mreg->getvalA()->normal();
    Mreg->getdstE()->normal();
    Mreg->getdstM()->normal();
}
/* gete_dstE
*  returns dstE, allowing forwarding
*
* @param pregs is the array of pipeline registers
* @param icode is the current icode
* @param cnd is the current cnd
* 
* @return dstE
*/
uint64_t ExecuteStage::gete_dstE(){
 return dstE;
}
/* gete_valE
*  returns valE, allowing forwarding
* @param pregs is the array of pipeline registers
* @param icode is the current icode
* 
* @return valE 
*/
uint64_t ExecuteStage::gete_valE(){
    return valE;
}

/* setMInput
 * provides the input to potentially be stored in the M register
 * during doClockHigh
 * 
 * @param Mreg - pointer to the M register instance
 * @param stat - value to be stored in the stat pipeline register within M
 * @param icode-value to be stored in the icode pipeline register within M
 * @param Cnd  - value to be stored in the Cnd pipeline register within M
 * @param valE -  value to be stored in the valE pipeline register within M
 * @param valA - value to be stored in the valA pipeline register within M
 * @param dstE - value to be stored in the dstE pipeline register within M
 * @param dstM - value to be stored in the dstM pipeline register within M
 * 
 */

void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, 
                           uint64_t cnd, uint64_t valE, uint64_t valA,
                           uint64_t dstE, uint64_t dstM)
    {
        mreg->getstat()->setInput(stat);
        mreg->geticode()->setInput(icode);
        mreg->getCnd()->setInput(cnd);
        mreg->getvalE()->setInput(valE);
        mreg->getvalA()->setInput(valA);
        mreg->getdstE()->setInput(dstE);
        mreg->getdstM()->setInput(dstM);
    }

/* aluA 
 * computes aluA value based on current instruction
 *
 * @param pregs is the array of pileline registers
 * @param e_icode is the icode of current instruction in e register
 * 
 * @return valA if instruction is IRRMOVQ or IOPQ
 * @return valC if insturction is IRRMOVQ, IRMMOVQ, or IMRMOVQ
 * @return -8 if instruction is ICALL or IPUSHQ
 * @return 8 if instruction is IRET or IPOPQ
 * @return 0 otherwise
 */
uint64_t ExecuteStage::aluA (PipeReg ** pregs, uint64_t e_icode){

  E * ereg = (E *) pregs[EREG]; 
         if(e_icode == IRRMOVQ || e_icode == IOPQ){
              return ereg->getvalA()->getOutput();
          }
          else if(e_icode == IIRMOVQ|| e_icode == IRMMOVQ || e_icode == IMRMOVQ){
              return ereg->getvalC()->getOutput();
          }
          else if(e_icode == ICALL || e_icode == IPUSHQ){
              return -8;
          }
          else if(e_icode == IRET || e_icode == IPOPQ){
              return 8;
           }
           else{
               return 0;
           }
}
 /* aluB 
 * computes aluB value based on current instruction
 *
 * @param pregs is the array of pileline registers
 * @param e_icode is the icode of current instruction in e register
 * 
 * return valB if instruction is IRRMOVQ, IMRMOVQ, IOPQ, ICALL, IPUSHQ, IRET, or IPOPQ
 * @return 0 otherwise
 */     
uint64_t ExecuteStage::aluB (PipeReg ** pregs, uint64_t e_icode){

    E * ereg = (E *) pregs[EREG]; 
        
    if(e_icode == IRMMOVQ || e_icode == IMRMOVQ || e_icode == IOPQ || e_icode == ICALL 
       || e_icode == IPUSHQ || e_icode == IRET || e_icode == IPOPQ){
          return ereg->getvalB()->getOutput();
      }
      else{
         return 0;
      }
}
 /* aluFun 
 * checks the function code of the instruction to determine which operation to perform in ALU
 *
 * @param pregs is the array of pileline registers
 * @param e_icode is the icode of current instruction in e register
 * 
 * @return ifun if icode is IOPQ
 * @return ADDQ otherwise
 */
uint64_t ExecuteStage::aluFun(PipeReg ** pregs, uint64_t e_icode){

   E * ereg = (E *) pregs[EREG]; 
   if(e_icode == IOPQ){
         return ereg->getifun()->getOutput();
    }
    else{
        return ADDQ;
     }
} 
/* setCc 
 * determines if condition codes need to be set for current instruction 
 *
 * @param pregs is the array of pileline registers
 * @param e_icode is the icode of current instruction in e register
 * 
 * @return true if icode is IOPQ, false otherwise 
 */
bool ExecuteStage::setCc(PipeReg ** pregs, uint64_t e_icode){

    E * ereg = (E *) pregs[EREG]; 
    return (ereg->geticode()->getOutput() == IOPQ);
}

/* e_dstE
 * generates value of dstE
 *
 * @param pregs is the array of pipeline registers
 * @param e_icode is the icode in the executestage register 
 * @param e_cnd is the cnd in the execute register 
 *
 * @return RNONE if icode is IRRMOVQ or not cnd, dstE in ereg otherwise
*/
uint64_t ExecuteStage::e_dstE(PipeReg ** pregs, uint64_t e_icode, uint64_t e_cnd){

    E * ereg = (E *) pregs[EREG]; 
    if(e_icode == IRRMOVQ && !e_cnd ){
        return RNONE;
    }
    else{
        return ereg->getdstE()->getOutput();
    }
}
/* ccCurcuit 
 * sets the condition codes based on the performed operation in alu 
 *
 * @param cc is true if condition code need to be set 
 * @param needSF is true if SF flag needs to be set 
 * @param needZF is true if SF flag needs to be set 
 * @param needOF is true if SF flag needs to be set 
 */
void ExecuteStage::ccCircuit(bool cc, bool needSF, bool needZF, bool needOF){
    bool error = false;
    if (cc){
        cc_instance->setConditionCode(needSF, SF,error);
        cc_instance->setConditionCode(needZF, ZF,error);
        cc_instance->setConditionCode(needOF, OF,error);
    }
    return;
}
/* aluCurcuit 
 * performs operation specified in IOPQ  
 *
 * @param aluF specifies which operation to perform 
 * @param aComp is operand 2 in the computation 
 * @param bComp is operand 1 in the computation 
 *
 * @return result is the result of the operation
 */
uint64_t ExecuteStage::aluCircuit(uint64_t aluF, uint64_t aComp, uint64_t bComp){
    uint64_t result;
    switch(aluF){
       //addq
        case ADDQ:
            result = bComp + aComp;
            break;
       //subq
        case SUBQ:
            result = bComp - aComp;
            break;
       //andq
       case ANDQ: 
            result = bComp & aComp;
             break;
        //xorq
        case XORQ:
            result = bComp ^ aComp; 
            break;
          }
        return result;     
}
/* cond
* calculates cnd based on icode and ifun
*
* @param icode is the icode for current instruction in execute stage
* @param ifun is the function code for current instruction in execute stage
*
* @return 0 if icode is not IJXX or ICMOVXX 
* @return seveleral diff values based on ifun and condition codes otherwise
*/
uint64_t ExecuteStage::cond(uint64_t icode, uint64_t ifun){

    if(icode != IJXX && icode != ICMOVXX) return 0;

    bool error = false;

    uint64_t sf = cc_instance->getConditionCode(SF, error);
    uint64_t zf = cc_instance->getConditionCode(ZF, error);
    uint64_t of = cc_instance->getConditionCode(OF, error);

    switch (ifun){
        
        //jmp or rrmovq
        case (0):
         return 1;
         break;

        //jle or cmovle
        case (1):
         return (sf ^ of) | zf;
         break;

        //jl or cmove
        case (2):
         return (sf ^ of);
         break;

        //je or cmove
        case (3):
         return zf;
         break;

        //jne or cmovne
        case (4):
         return !zf;
         break;

        //jg or cmovq
        case (6):
         return !(sf ^ of) & !zf;
         break;

        //jge or cmovge
        case (5):
         return !(sf ^ of);
         break;
    }

}

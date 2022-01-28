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
#include "WritebackStage.h"
#include "Instructions.h"

    RegisterFile * reg= RegisterFile::getInstance();

/*
 * doClockLow:
 * Performs the Writeback stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    W * wreg = (W *) pregs[WREG]; 
    if(wreg->geticode()->getOutput()== IHALT) return true;

    return false; 
}
/*
* doClockHigh
* does nothing since writeback is final stage
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg ** pregs)
{
    
W * wreg = (W *) pregs[WREG]; 
bool error = false;
reg->writeRegister(wreg->getvalE()->getOutput(), wreg->getdstE()->getOutput(), error);
reg->writeRegister(wreg->getvalM()->getOutput(), wreg->getdstM()->getOutput(), error);

}
CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Loader.o Memory.o Tools.o RegisterFile.o \
ConditionCodes.o D.o DecodeStage.o E.o ExecuteStage.o \
F.o FetchStage.o M.o MemoryStage.o PipeReg.o PipeRegField.o\
Simulate.o W.o WritebackStage.o 

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h RegisterFile.h ConditionCodes.h Loader.h PipeReg.h Stage.h Simulate.h  

Loader.o: Loader.h Memory.h 

Memory.o:  Memory.h Tools.h

Tools.o:  Tools.h

RegisterFile.o:  RegisterFile.h Tools.h

ConditionCodes.o:  ConditionCodes.h Tools.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h FetchStage.h Status.h Debug.h DecodeStage.h Instructions.h 

E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h E.h Status.h 

ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h FetchStage.h Status.h Debug.h ExecuteStage.h Instructions.h

F.o:  PipeRegField.h PipeReg.h F.h 

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h FetchStage.h Status.h Debug.h Memory.h Instructions.h

M.o:  RegisterFile.h Instructions.h PipeRegField.h PipeReg.h M.h Status.h

MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h FetchStage.h Status.h Debug.h MemoryStage.h

PipeReg.o: PipeReg.h

PipeRegField.o: PipeRegField.h

WritebackStage.o:  RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h FetchStage.h Status.h Debug.h WritebackStage.h Instructions.h DecodeStage.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h

W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h W.h Status.h


clean:
	rm $(OBJ) yess

run:
	make yess
	./run.sh


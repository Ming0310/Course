#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "pin.H"

using namespace std;

ofstream OutFile;

#define truncate(val, bits) ((val) & ((1 << (bits)) - 1))

static UINT64 takenCorrect = 0;
static UINT64 takenIncorrect = 0;
static UINT64 notTakenCorrect = 0;
static UINT64 notTakenIncorrect = 0;

template <size_t N, UINT64 init = (1 << N)/2 - 1>   // N < 64
class SaturatingCnt
{
    UINT64 val;
    public:
        SaturatingCnt() { reset(); }

        void increase() { if (val < (1 << N) - 1) val++; }
        void decrease() { if (val > 0) val--; }

        void reset() { val = init; }
        UINT64 getVal() { return val; }

        BOOL isTaken() { return (val > (1 << N)/2 - 1); }
};

template<size_t N>      // N < 64
class ShiftReg
{
    UINT64 val;
    public:
        ShiftReg() { val = 0; }

        bool shiftIn(bool b)
        {
            bool ret = !!(val&(1<<(N-1)));
            val <<= 1;
            val |= b;
            val &= (1<<N)-1;
            return ret;
        }

        UINT64 getVal() { return val; }
};

class BranchPredictor
{
    public:
        BranchPredictor() { }
        virtual BOOL predict(ADDRINT addr) { return FALSE; };
        virtual void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr) {};
};

BranchPredictor* BP;



/* ===================================================================== */
/* 实现下列3种动态预测方法                                                 */
/* ===================================================================== */
// 1. BHT-based branch predictor
template<size_t L>
class BHTPredictor: public BranchPredictor
{
    SaturatingCnt<2> counter[1 << L];
    
    public:
        BHTPredictor() { }

        BOOL predict(ADDRINT addr)
        {
            // TODO:
            return counter[addr & ( (1 << L) - 1)].isTaken();
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            if(counter[addr & ( (1 << L) - 1)].getVal() == 1 && takenActually){
                counter[addr & ( (1 << L) - 1)].increase();
            }
            if(counter[addr & ( (1 << L) - 1)].getVal() == 2 && !takenActually)
                counter[addr & ( (1 << L) - 1)].decrease();
            if(takenActually)
                counter[addr & ( (1 << L) - 1)].increase();
            else
                counter[addr & ( (1 << L) - 1)].decrease();
        }
};

// 2. Global-history-based branch predictor
template<size_t L, size_t H, UINT64 BITS = 2>
class GlobalHistoryPredictor: public BranchPredictor
{
    SaturatingCnt<BITS> bhist[1 << L];  // PHT中的分支历史字段
    ShiftReg<H> GHR;
    public:
        GlobalHistoryPredictor() { }
    // TODO:
    BOOL predict(ADDRINT addr)
        {
            // TODO:
            int x = (addr ^ GHR.getVal()) & ((1 << L) - 1);
            return bhist[x].isTaken();
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            int x = (addr ^ GHR.getVal()) & ((1 << L) - 1);
            if(takenActually){
                if(bhist[x].getVal() == 1)
                    bhist[x].increase();
                bhist[x].increase();
            }
            else{
                if(bhist[x].getVal() == 2)
                    bhist[x].decrease();
                bhist[x].decrease();
            }
            GHR.shiftIn(takenActually);
        }
};

// 3. Local-history-based branch predictor
template<size_t L, size_t H, size_t HL = 6, UINT64 BITS = 2>
class LocalHistoryPredictor: public BranchPredictor
{
    SaturatingCnt<BITS> bhist[1 << L];  // PHT中的分支历史字段
    ShiftReg<H> LHT[1 << HL];
    public:
    LocalHistoryPredictor() { }
    // TODO:
    BOOL predict(ADDRINT addr)
        {
            // TODO:
            int x = (LHT[(addr >> L) & ((1 << HL) - 1)].getVal() ^ addr) & ((1 << L) - 1);
            return bhist[x].isTaken();
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            int x = (LHT[(addr >> L) & ((1 << HL) - 1)].getVal() ^ addr) & ((1 << L) - 1);
            switch(bhist[x].getVal()){
                case 0:{
                    if(takenActually)
                        bhist[x].increase();
                    break;
                }
                case 1:{
                    if(takenActually)
                        {
                            bhist[x].increase();
                            bhist[x].increase();
                        }
                    else
                        bhist[x].decrease();
                    break;
                }
                case 2:{
                    if(takenActually)
                        bhist[x].increase();
                    else{
                        bhist[x].decrease();
                        bhist[x].decrease();
                    }
                    break;
                }
                case 3:{
                    if(!takenActually)
                        bhist[x].decrease();
                }
            }
            LHT[(addr >> L) & ((1 << HL) - 1)].shiftIn(takenActually);
        }
};
/* ===================================================================== */
/* 锦标赛预测器的选择机制可用全局法或局部法实现，二选一即可                   */
/* ===================================================================== */
// 2. Tournament predictor: Select output by local selection history
template<size_t L, UINT64 BITS = 2>
class TournamentPredictor_LSH: public BranchPredictor
{
    SaturatingCnt<BITS> LSHT[1 << L];
    BranchPredictor* BPs[2];

    public:
        TournamentPredictor_LSH(BranchPredictor* BP0, BranchPredictor* BP1)
        {
            BPs[0] = BP0;
            BPs[1] = BP1;
        }

        // TODO:
        BOOL predict(ADDRINT addr)
        {
            // TODO:
            if(LSHT[addr & ((1 << L ) - 1)].isTaken())
                return (BPs[1]->predict(addr));
            else
                return (BPs[0]->predict(addr));
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            int index1 = addr & ((1 << L ) - 1);
            int x = LSHT[index1].getVal();
            int index2 = (x >> 1) & 1;
            BPs[index2]->update(takenActually,takenPredicted,addr);
            bool taken2 = BPs[1 - index2]->predict(addr);
            BPs[1 - index2]->update(takenActually,taken2,addr);
            switch(x){
                case 0:{
                    if(takenActually != takenPredicted && taken2 == takenActually)// B
                        LSHT[index1].increase();
                    break;
                }
                case 1:{
                    if(takenActually != takenPredicted && taken2 == takenActually)// B
                    {
                        LSHT[index1].increase();
                        LSHT[index1].increase();
                    }
                    else if(takenActually == takenPredicted && taken2 != takenActually)
                        LSHT[index1].decrease();
                    break;
                }
                case 2:{
                    if(takenActually != takenPredicted && taken2 == takenActually)
                        {
                            LSHT[index1].decrease();
                            LSHT[index1].decrease();
                        }
                    else if(takenActually == takenPredicted && taken2 != takenActually)
                        LSHT[index1].increase();
                    break;
                }
                case 3:{
                    if(takenActually != takenPredicted && taken2 == takenActually)
                        LSHT[index1].decrease();
                }
            }   
        }
};


// This function is called every time a control-flow instruction is encountered
void predictBranch(ADDRINT pc, BOOL direction)
{
    BOOL prediction = BP->predict(pc);
    BP->update(direction, prediction, pc);
    if (prediction)
    {
        if (direction)
            takenCorrect++;
        else
            takenIncorrect++;
    }
    else
    {
        if (direction)
            notTakenIncorrect++;
        else
            notTakenCorrect++;
    }
}

// Pin calls this function every time a new instruction is encountered
void Instruction(INS ins, void * v)
{
    if (INS_IsControlFlow(ins) && INS_HasFallThrough(ins))
    {
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)predictBranch,
                        IARG_INST_PTR, IARG_BOOL, TRUE, IARG_END);

        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)predictBranch,
                        IARG_INST_PTR, IARG_BOOL, FALSE, IARG_END);
    }
}

// This knob sets the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "brchPredict.txt", "specify the output file name");

// This function is called when the application exits
VOID Fini(int, VOID * v)
{
	double precision = 100 * double(takenCorrect + notTakenCorrect) / (takenCorrect + notTakenCorrect + takenIncorrect + notTakenIncorrect);
    
    cout << "takenCorrect: " << takenCorrect << endl
    	<< "takenIncorrect: " << takenIncorrect << endl
    	<< "notTakenCorrect: " << notTakenCorrect << endl
    	<< "nnotTakenIncorrect: " << notTakenIncorrect << endl
    	<< "Precision: " << precision << endl;
    
    OutFile.setf(ios::showbase);
    OutFile << "takenCorrect: " << takenCorrect << endl
    	<< "takenIncorrect: " << takenIncorrect << endl
    	<< "notTakenCorrect: " << notTakenCorrect << endl
    	<< "nnotTakenIncorrect: " << notTakenIncorrect << endl
    	<< "Precision: " << precision << endl;
    
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // TODO: New your Predictor below.
    // BP = new GlobalHistoryPredictor<27,27>();
    // BP = new BHTPredictor<27>();
    // BP = new LocalHistoryPredictor<27,27>();
    BranchPredictor* BP1,*BP2;
    BP1 = new LocalHistoryPredictor<27,27>();
    BP2 = new BHTPredictor<27>();
    BP = new TournamentPredictor_LSH<12>(BP1,BP2);
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    
    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

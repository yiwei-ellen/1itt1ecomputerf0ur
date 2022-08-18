/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>
#define INSN_OP(I) ((I) >> 12) //15:12
#define INSN_11_9(I) (((I) >> 9) & 0x7) //11:9
#define INSN_8_7(I) (((I) >> 7) & 0x3) //8:7
#define INSN_8_6(I) (((I) >> 6) & 0x7) //8:6
#define INSN_5_3(I) (((I) >> 3) & 0x7) //5:3
#define INSN_5_4(I) (((I) >> 4) & 0x3) //5:4
#define INSN_2_0(I) ((I) & 0x7)   //2:0

#define INSN_3_0(I) ((I) & 0xF) // IMM4
#define INSN_4_0(I) ((I) & 0x1F) //IMM5
#define INSN_5_0(I) ((I) & 0x3F) //IMM6
#define INSN_6_0(I) ((I) & 0x7F) //IMM7
#define INSN_7_0(I) ((I) & 0xFF) //IMM8
#define INSN_8_0(I) ((I) & 0x1FF) //IMM9
#define INSN_10_0(I) ((I) & 0x7FF) //IMM11

#define INSN_15(I) ((I) >> 15) // MSB
/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{
    int i;
    //this is for part one testing
    /*
    for (i = 0;i<65536;i++){
        if(CPU->memory[i]!=0 && i<=0x9FFF){
            printf("address: %d content: 0x%x \n",i,CPU->memory[i]);
        }
        ifCPU->memory[i] = 0;
    }*/ 
    
    //reset registers
    for (i = 0 ; i < 8; i++){
        CPU->R[i] = 0;
    }
    CPU->PC = 0x8200;
    CPU->PSR =0x8002; //
    ClearSignals(CPU);
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 0;;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    CPU->regInputVal = 0;
    CPU->NZPVal = 2; //'0'
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    char ins_str [17];
    int pc,instruction, regWE, Reg, Reg_V, nzpWE, nzp, dWE,dADD,dVAL;
    int i;
    int opcode;

    pc = CPU->PC;
    //first get instruction 
    instruction = CPU->memory[CPU->PC];
    opcode = INSN_OP(instruction);
    //populate instruction to string
    for(i=0;i<16;i++){
        if (instruction %2 ==0){
            ins_str[15-i] = '0';
        } else{
            ins_str[15-i]='1';
        }
        instruction/=2;
    }
    ins_str[16] = '\0';

    regWE = CPU->regFile_WE;

    nzpWE = CPU->NZP_WE;    
    
    dWE = CPU->DATA_WE;

    if (regWE ==1 && opcode !=15 && opcode != 4){
        Reg = INSN_11_9(CPU->memory[CPU->PC]);
        Reg_V = CPU->R[Reg];
    } else if(regWE == 1 && (opcode ==15 || opcode==4) ){
        Reg = 7;
        Reg_V = CPU->R[Reg];
    } else {
        Reg = 0;
        Reg_V = 0;
    }
    if(nzpWE==0){
        nzp =0; //mute display of nzp if NZP we is 0
    } else {
        nzp = CPU->NZPVal;
    }
    
    if (dWE ==1 ){
        dADD = CPU->dmemAddr;
        dVAL = CPU->dmemValue;
    } else if (opcode == 6){
        dADD = CPU->dmemAddr;
        dVAL = CPU->memory[dADD];
    } else {
        dADD = 0;
        dVAL = 0;
    }
    
    fprintf(output,"%04X %s %X %X %04X %X %X %X %04X %04X\n",pc,ins_str,regWE,Reg, Reg_V, nzpWE,nzp,dWE,dADD,dVAL);
    return;

}


/*
 * This function should execute one LC4 datapath cycle.
 * logic: take the opcode and do switch statement for each
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    
    int opcode;
    int instruction, regWE, Reg, Reg_V, nzpWE, nzp, dWE,dADD,dVAL;
    int i;
    int rd,rs,rt;
    int rdv,rsv,rtv;
    int imm;
    int temp;
    int priv;

    int pc = CPU->PC;
    int psr = INSN_15(CPU->PSR);
    if (psr == 0 && (pc>=0x2000)){
        CPU->PC = 0x80FF;
        printf("error: not proper code location, pc points to 0x%x \n",pc);
        return (-1);
    }

    //first get instruction 
    instruction = CPU->memory[CPU->PC];
    opcode = INSN_OP(instruction);
    //printf("address is %x\n",CPU->PC);
    //printf("opcode is %d\n",opcode);
    switch (opcode){
        case 0:
            BranchOp(CPU,output);
            break;
        case 1:
            ArithmeticOp(CPU,output);
            break;
        case 2:
            ComparativeOp(CPU,output);
            break;
        case 4:
            JSROp(CPU,output);
            break;
        case 5:
            LogicalOp(CPU,output);
            break;
        case 10:
            ShiftModOp(CPU,output);
            break;
        case 12:
            JumpOp(CPU,output);
            break;
        case 6:
            //LDR
            CPU->DATA_WE = 0;
            CPU ->regFile_WE = 1;
            CPU -> rsMux_CTL = 0;
            CPU -> rdMux_CTL = 0;
            rd = INSN_11_9(instruction);
            rs = INSN_8_6(instruction);
            imm = INSN_5_0(instruction);
            if(imm/32==1){
                imm = (imm^32)-32;
            }
            rsv = CPU->R[rs];
            temp = rsv+imm;
            priv = INSN_15(CPU->PSR);
            if ((temp<0x2000 ||temp>=0x8000)&&  priv==0){
                CPU->PC = 0x80FF;
                printf("error: access OS section without privilege");
                return(-1);
            } else {
                CPU->dmemAddr = temp;
                CPU->R[rd]=CPU->memory[temp];
                SetNZP(CPU,CPU->R[rd]);
                WriteOut(CPU,output);
                CPU->PC++;
            }
            break;
        case 7://STR
            CPU->DATA_WE = 1;
            CPU ->regFile_WE = 0;
            CPU -> rsMux_CTL = 0;
            CPU -> rtMux_CTL = 1;
            CPU->NZP_WE=0;
            rt = INSN_11_9(instruction);
            rs = INSN_8_6(instruction);
            imm = INSN_5_0(instruction);
            if(imm/32==1){
                imm = (imm^32)-32;
            }
            rsv = CPU->R[rs];
            temp=rsv+imm;
            priv = INSN_15(CPU->PSR);
            if ((temp<0x2000 ||temp>=0x8000)&&  priv==0){
                CPU->PC = 0x80FF;
                printf("error: write OS section without privilege");
                return(-1);
            } else {
                CPU->dmemValue = CPU->R[rt];
                CPU->dmemAddr = temp;
                CPU->memory[CPU->dmemAddr]=CPU->dmemValue;
                WriteOut(CPU,output);
                CPU->PC++;
                CPU->DATA_WE = 0;
            }
            break;
        case 8:
            //RTI
            CPU->DATA_WE = 0;
            CPU -> rsMux_CTL = 1;
            CPU -> regFile_WE = 0;
            CPU->NZP_WE = 0;//question 
            CPU->PSR = CPU->PSR & 0x7FFF;
            WriteOut(CPU,output);
            CPU->PC=CPU->R[7];
            break;
        case 9:
            //const
            CPU->rdMux_CTL=0;
            CPU->DATA_WE = 0;
            CPU -> regFile_WE = 1;
            rd = INSN_11_9(instruction);
            imm = INSN_8_0(instruction);
            if(imm/256==1){
                imm = (imm^256)-256;
            }
            CPU->R[rd]= imm;
            SetNZP(CPU,imm);
            //printf("COnST: R%d is value: %x\n",rd,CPU->R[rd]);
            WriteOut(CPU,output);
            CPU->PC++;
            break;
        case 13:
            //hiconst
            CPU->rdMux_CTL=0;
            CPU->regFile_WE=1;
            CPU->DATA_WE = 0;
            rd = INSN_11_9(instruction);
            imm = INSN_7_0(instruction);
            rdv = CPU->R[rd];

            temp = (rdv & 0xFF)|(imm<<8);
            CPU->R[rd]=temp;
            SetNZP(CPU,temp);
            WriteOut(CPU,output);
            CPU->PC++;
            break;
        case 15:
            //trap
            CPU->rdMux_CTL = 1;
            CPU->DATA_WE=0;
            CPU->regFile_WE=1;
            CPU->R[7]=CPU->PC+1;
            imm = INSN_7_0(instruction);
            CPU->PSR = CPU->PSR | 0x8000;
            SetNZP(CPU,CPU->R[7]);
            WriteOut(CPU,output);
            CPU->PC = (0x8000|imm);
            break;
    }
    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int encoding = INSN_11_9(instruction);
    int imm = INSN_8_0(instruction);
    int nzp = CPU->NZPVal;

    CPU ->regFile_WE = 0;
    CPU ->DATA_WE = 0;
    CPU ->NZP_WE = 0;


    if (imm/256 == 1){
        imm = (imm^256)-256;
    }
    //printf("current nzp: %d\n",nzp);
    switch(encoding){
        case 0: //NOP
            WriteOut(CPU,output);
            CPU->PC++;
            return;
        case 1: //BRp
            WriteOut(CPU,output);
            if (nzp == 1){ 
                CPU->PC = CPU->PC+1+imm;
            } else {
                CPU->PC++;
            }
            break;
        case 2://BRz
            WriteOut(CPU,output);
            if (nzp == 2){
                CPU->PC = CPU->PC+1+imm;
            } else {
                CPU->PC++;
            }
            break;
        case 3: //BRzp
            WriteOut(CPU,output);
            if (nzp == 1 || nzp ==2){
                CPU->PC = CPU->PC+1+imm;
            } else {
                CPU->PC++;
            }
            break;
        case 4: //BRn
            WriteOut(CPU,output);
            if (nzp == 4){
                CPU->PC = CPU->PC+1+imm;
            } else {
                CPU->PC++;
            }
            break;
        case 5: //BRnp
            WriteOut(CPU,output);
            if (nzp == 1 || nzp == 4){
                CPU->PC = CPU->PC+1+imm;
            } else {
                CPU->PC++;
            }
            break;
        case 6: //BRnz
            WriteOut(CPU,output);
            if (nzp == 2 || nzp == 4){
                CPU->PC = CPU->PC+1+imm;
            } else {
                CPU->PC++;
            }
            break;
        case 7: //BRnzp
            WriteOut(CPU,output);
            CPU->PC = CPU->PC+1+imm;
            break;
    }
    return;
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int rd = INSN_11_9(instruction);
    int rs = INSN_8_6(instruction);
    int type = INSN_5_3(instruction);
    int rt = INSN_2_0(instruction);
    int imm = INSN_4_0(instruction);
    int rdv = CPU->R[rd];
    int rsv = CPU->R[rs];
    int rtv = CPU->R[rt];
    CPU->rsMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    CPU ->regFile_WE = 1;
    CPU ->DATA_WE = 0;

    switch (type){
        case 0: //Add
            CPU->rtMux_CTL =0;
            CPU->R[rd] = rsv+rtv;
            break;
        case 1://MUl
            CPU->rtMux_CTL =0;
            CPU->R[rd] = rsv*rtv;
            break;
        case 2://Sub
            CPU->rtMux_CTL =0;
            CPU->R[rd] = rsv-rtv;
            break;
        case 3://DIV
            CPU->rtMux_CTL =0;
            CPU->R[rd] = rsv/rtv;
            break;
        default://ADD Imm
            if (imm/16==1){
                imm = (imm^16)-16;
            }
            CPU->R[rd] = rsv+imm;
            break;
    }
    SetNZP(CPU,CPU->R[rd]);
    WriteOut(CPU,output);
    CPU->PC++;
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int rs = INSN_11_9(instruction);
    int type = INSN_8_7(instruction);
    int rt = INSN_2_0(instruction);
    int imm = INSN_6_0(instruction);
    int rsv = CPU->R[rs];
    int rtv = CPU->R[rt];
    int result;
    CPU->rsMux_CTL = 2;
    CPU ->regFile_WE = 0;
    CPU ->DATA_WE = 0;
    switch(type){
        case 0://CMP
            CPU -> rtMux_CTL = 0;
            if(INSN_15(rsv)==1){
                rsv = (rsv^32768)-32768; //fix: unsigned number to signed
            }
            if(INSN_15(rtv)==1){
                rtv = (rtv^32768)-32768;//fix: unsigned number to signed
            }
            result = rsv-rtv;
            break;
        case 1://CMPU
            CPU -> rtMux_CTL = 0;
            result = rsv - rtv;
            break;
        case 2://CMPI
            if(INSN_15(rsv)==1){
                rsv = (rsv^32768)-32768;
            }
            if(imm/64==1){
                imm = (imm^64)-64;
            }
            result = rsv - imm;
            break;
        case 3: //CMPIU
            result = rsv - imm;
            break;
    }
    SetNZP(CPU,result);
    WriteOut(CPU, output);
    CPU->PC++;
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int rd = INSN_11_9(instruction);
    int rs = INSN_8_6(instruction);
    int type = INSN_5_3(instruction);
    int rt = INSN_2_0(instruction);
    int imm = INSN_4_0(instruction);
    int rdv = CPU->R[rd];
    int rsv = CPU->R[rs];
    int rtv = CPU->R[rt];
    CPU->rsMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    CPU ->regFile_WE = 1;
    CPU ->DATA_WE = 0;

    switch(type){
        case 0: //AND
            CPU->rtMux_CTL = 0;
            rdv = rsv & rtv;
            CPU->R[rd]=rdv;
            break;
        case 1://NOt
            rdv = ~rsv;
            CPU->R[rd]=rdv;
            break;
        case 2: //or
            CPU->rtMux_CTL = 0;
            rdv = rsv | rtv;
            CPU->R[rd]=rdv;
            break;
        case 3://XOR
            CPU->rtMux_CTL = 0;
            rdv = rsv ^ rtv;
            CPU->R[rd]=rdv;
            break;
        default: //AND imm
            if (imm/16==1){
                imm = (imm^16)-16;
            }
            CPU->R[rd] = rsv & imm;
            break;
    }
    SetNZP(CPU,CPU->R[rd]);
    WriteOut(CPU,output);
    CPU->PC++;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int type = (instruction >> 11) & 0x1;
    int rs, rsv,imm;
    CPU->DATA_WE = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    if(type == 0){//JMPR
        CPU->rsMux_CTL = 0;
        rs = INSN_8_6(instruction);
        rsv = CPU->R[rs];
        WriteOut(CPU, output);
        CPU->PC = rsv;
    } else { //JMP
        imm = INSN_10_0(instruction);
        if(imm/1024==1){
            imm = (imm^1024)-1024;
        }
        WriteOut(CPU, output);
        CPU->PC = CPU->PC+1+imm;
    }
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int type = (instruction >> 11) & 0x1;
    int rs, rsv,imm,priv,result;
    CPU->DATA_WE = 0;
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->rdMux_CTL = 1;
    priv = INSN_15(CPU->PSR);
    if(type == 0){//JSRR
        CPU->rsMux_CTL = 0;
        rs = INSN_8_6(instruction);
        rsv = CPU->R[rs];
        if (rsv>=0x8000 && priv==0){
            CPU->PC = 0x80FF;
            printf("error: access OS section without privilege");
            return;
        }
        CPU->R[7] = CPU->PC+1;
        WriteOut(CPU, output);
        CPU->PC = rsv;
    } else { //JSR
        imm = INSN_10_0(instruction);
        if(imm/1024==1){
            imm = (imm^1024)-1024;
        }
        CPU->R[7] = CPU->PC+1;
        result = ((CPU->PC & 0x8000)|(imm<<4));
        if (result>=0x8000 && priv==0){
            CPU->PC = 0x80FF;
            printf("error: access OS section without privilege");
            return;
        }
        WriteOut(CPU, output);
        CPU->PC = result;
    }
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    int instruction = CPU->memory[CPU->PC];
    int rd = INSN_11_9(instruction);
    int rs = INSN_8_6(instruction);
    int type = INSN_5_4(instruction);
    int rt;
    int imm = INSN_3_0(instruction);
    int rdv = CPU->R[rd];
    int rsv = CPU->R[rs];
    int rtv;
    int i;
    CPU->rsMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    CPU ->regFile_WE = 1;
    CPU ->DATA_WE = 0;

    
    switch(type){
        case 0://SLL
            rdv = rsv <<imm;
            CPU->R[rd]=rdv;
            break;
        case 1://SRA
            //fix: had diff understanding of what SRA was
            if (rsv >>15 ==1){
                while(rsv >>15 ==1 && imm>0){
                    rdv = (rsv >>1) + 0x8000;
                    rsv = (rsv>>1)+ 0x8000;
                    imm--;
                }
                //printf("Memory is %X, SRA rs=%X\n",CPU->PC, rdv);
            } else {
                rdv = rsv >> imm;
                //printf("else Memory is %X, SRA rs=%X\n",CPU->PC, rdv);
            }
            CPU->R[rd]=rdv;
            break;
        case 2://SRL
            rdv = rsv>>imm;
            CPU->R[rd]=rdv;
            break;
        case 3: //MOD
            CPU->rtMux_CTL = 0;
            rt= INSN_2_0(instruction);
            rtv = CPU->R[rt];
            rdv = rsv % rtv;
            CPU->R[rd]=rdv;
            break;
    }
    SetNZP(CPU,CPU->R[rd]);
    WriteOut(CPU,output); 
    CPU->PC++;
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    CPU->NZP_WE = 1;
    if (result == 0){
        CPU->PSR = CPU->PSR & 0xFFF8+2;
        CPU->NZPVal = 2;
    } else if (result > 0){
        CPU->PSR = CPU->PSR & 0xFFF8 + 1;
        CPU->NZPVal = 1;
    } else {
        CPU->PSR = CPU->PSR & 0xFFF8 + 4;
        CPU->NZPVal = 4;
    }
    return;
    
}

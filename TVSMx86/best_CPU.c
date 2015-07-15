
/*
AUTHORS
-Thejass Krishnan (010002330)
-Shweta Bohare    (008931377)
-Vivek Goel       (010126753)
-Mridula Mishra   (009294090)
*/


//#include <iostream.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
//using namespace std;
#define MAX 256
int top,MemB;
unsigned cache[50];
unsigned regInputBuffer;
//General Purpose Registers
unsigned regA,regB,regC,regD,regE;

//Instruction Register - holds the instruction
unsigned regIR;

//Flag register
//Flag register has 32 bits
unsigned regF;

//The bits in Memory Address Register is directly linked to the address of the Main Memory.
unsigned regMAR;

//Memory Data Register stores the data to be written into the Memory address in regMDR or taken from the memory address in regMDR.
unsigned regMDR;

//Pointers
unsigned regBP,regSP,regES,regPC,regDS,regSS,regIP;

//Code Segment registers
//regSAP stores the starting address of the program
int stack[MAX];
int labelTable[20][2],labelIndex=0;
unsigned mainMemory[1024];
unsigned instructionQ;


void printMemory()
{
    
    printf("\nRegister Values");
    printf("\nRegister A = %d",regA);
    printf("\nRegister B = %d",regB);
    printf("\nRegister C = %d",regC);
    printf("\nRegister D = %d",regD);
    printf("\nRegistrMDR = %d",regMDR);
    printf("\nRegistrMAR = %d",regMAR);
    printf("\nRegistrFLG = %d",regF);
    printf("\nRegistrPC  = %d",regPC);
    
    printf("\nMain Memory\n");
    
    for(int i=0;i<1024;i++)
    {
        printf("%d",mainMemory[i]);
        if((i+1)%32==0)
            printf("   EOMB(32bit)\n");
    }
    
    
}

unsigned convertStringToHex(char regInputBufferString[])
{
    unsigned tempReg;
    for(int i=0;i<8;i++)
        switch(regInputBufferString[9-i])
    {
        case '0':tempReg+=0*pow(16,i);break;
        case '1':tempReg+=1*pow(16,i);break;
        case '2':tempReg+=2*pow(16,i);break;
        case '3':tempReg+=3*pow(16,i);break;
        case '4':tempReg+=4*pow(16,i);break;
        case '5':tempReg+=5*pow(16,i);break;
        case '6':tempReg+=6*pow(16,i);break;
        case '7':tempReg+=7*pow(16,i);break;
        case '8':tempReg+=8*pow(16,i);break;
        case '9':tempReg+=9*pow(16,i);break;
        case 'a':tempReg+=10*pow(16,i);break;
        case 'b':tempReg+=11*pow(16,i);break;
        case 'c':tempReg+=12*pow(16,i);break;
        case 'd':tempReg+=13*pow(16,i);break;
        case 'e':tempReg+=14*pow(16,i);break;
        case 'f':tempReg+=15*pow(16,i);break;
    }
    //cout<<tempReg;
    return tempReg;
}

void inputInterrupt()
{
    printf("\nInput Interrupt!");
}
void writeToMemoryInterrupt()
{
    //printf("\nWrite to memory interrupt!");
}
void readFromMemoryInterrup()
{
    printf("\nRead from memory interrup!");
}

unsigned twoscomplement(unsigned reg)
{
    return (~reg+1);
}

unsigned addsubALU(unsigned reg1, unsigned reg2)
{
    regF=regF-(regF&0x00000008);
    regF=regF-(regF&0x00000004);
    unsigned carryIN=0,carryOUT=0;
    //Carry Flag
    if(reg1<reg2 && (reg2&0x80000000))
        //regF=regF-(regF&0x00000001)+1;
        //printf("<%d,%d>",reg1,reg2);
    carryOUT = ( ( (reg1) %2) & ( (reg2) %2) );
    reg1=reg1-((reg1)%2)+(((reg1)%2)^((reg2)%2));
    //printf("\nOUT->%d",carryOUT);
    //printf("\nIN->%d",carryIN);
    //printf("\nreg1->%d",reg1);

    for(int i=1;i<32;i++)
    {
        carryIN=carryOUT;
        carryOUT = ((((unsigned)(reg1/pow(2,i)))%2) & (((unsigned)(reg2/pow(2,i)))%2)) || ((((unsigned)(reg1/pow(2,i)))%2)&carryIN) || ((((unsigned)(reg2/pow(2,i)))%2)&carryIN)  ;
        reg1=reg1-((unsigned)(reg1/pow(2,i))%2)*pow(2,i)+((((unsigned)(reg1/pow(2,i))%2)^(((unsigned)(reg2/pow(2,i))%2)))^carryIN)*pow(2,i);
        //printf("\nOUT->%d",carryOUT);
        //printf("\nIN->%d",carryIN);
        //printf("\nreg1->%d",reg1);
    }
    
    //Carry Flag
    if (carryOUT == 1 && !(reg2&0x80000000))
    {
        //regF=regF-(regF&0x00000001)+1;
    }
    if (carryIN ==1 && (reg2&0x80000000 || reg1&0x80000000))
    {
        //regF=regF-(regF&0x00000001)+1;
    }
    //Sign Flag
    if (reg1<0)
    {
        //printf("Neg");
        regF=regF-(regF&0x00000002)+2;
        printMemory();
    }
        //Overflow Flag
        if((carryIN==1 && carryOUT==0) || (carryIN==0 && carryOUT==1))
            regF=regF-(regF&0x00000004)*4+4;

    //Setting Zero FLAG
    if(reg1 == 0 )
    {
        regF = regF - (regF&0x00000008)+8;
        //printf("FLAG 8");
        printMemory();
    }
    return reg1;
}

unsigned mulALU(unsigned reg1, unsigned reg2)
{
    int k=0;
    unsigned carryIN[32]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned carryOUT[32]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned sumIN[64]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned sumOUT[64]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned mul=0;
    
    if(((reg1&0x80000000)^(reg2&0x80000000)))
    {
        regF=regF-(regF&0x00000002)+2;
        //cout<<">"<<regF;
    }
    if(regF&0x00000004)
        regF=regF-4;
    if(reg1&0x80000000)
        reg1=twoscomplement(reg1);
    if(reg2&0x80000000)
        reg2=twoscomplement(reg2);
    
    for(int i=0;i<32;i++)
    {
        for(int j=0;j<32;j++)
        {
            sumOUT[j+i]=(((((unsigned)(reg1/pow(2,j))%2)&((unsigned)(reg2/pow(2,i))%2))^carryIN[j])^sumIN[j+i]);
            carryOUT[j]=(((((unsigned)(reg1/pow(2,j))%2)&((unsigned)(reg2/pow(2,i))%2)) & carryIN[j])||(carryIN[j] & sumIN[j+i]) || ((((unsigned)(reg1/pow(2,j))%2)&((unsigned)(reg2/pow(2,i))%2))&sumIN[j+i]));
            
            for(k=0;k<64;k++)
            {
                sumIN[k]=sumOUT[k];
                if(sumIN[k]==1 && k>31)
                    regF=regF-(regF&0x00000004)+4;
            }
            
            for(k=0;k<32;k++)
            {
                carryIN[k]=carryOUT[k];
            }
            
            if(carryIN[31]==1)
                regF=regF-(regF&0x00000004)+4;
            
            if((regF&0x00000004)==4)
                break;
            
            
        }
        if((regF&0x00000004)==4)
        {
            printf("\n\nOverflow!!\n");
            break;
        }
    }
    
    for(k=0;k<32;k++)
        mul=mul+sumOUT[k]*pow(2,k);
    
    if((regF&0x00000002)==2)
        mul=twoscomplement(mul);
    
    if(carryOUT[31]==1)
    {
        regF=regF-(regF&0x00000001)+1;
        regF=regF-(regF&0x00000004)+4;
    }
    //Setting Zero FLAG
    if(reg1 ==0 || mul==0)
        regF = regF - (regF&0x00000008)+8;
    
    if((regF&0x00000004)==4)
        return reg1;
    else
        return mul;
    
}

unsigned modALU(unsigned reg1, unsigned reg2)
{
    //setting sign bit
    unsigned reg=reg2;
    if(reg1&0x80000000)
    {
        reg1=twoscomplement(reg1);
        regF = regF-(regF&0x00000002)+2;
    }
    if(!(reg2&0x80000000))
    {
        reg2=twoscomplement(reg2);
    }
    while(reg1>=reg)
    {
        reg1=addsubALU(reg1,reg2);
    }
    
    regF = regF - (regF&0x00000008);
    //Setting Zero FLAG
    if(reg1 == 0)
        regF = regF - (regF&0x00000008)+8;
    return reg1;
}

unsigned divALU(unsigned reg1, unsigned reg2)
{
    unsigned quo=0,reg=reg2;
    regF = regF-(regF&0x00000002)+2*((reg1&0x80000000)^(reg2&0x80000000));
    if(reg1&0x80000000)
    {
        reg1 = twoscomplement(reg1);
    }
    if(reg2&0x80000000)
    {
        reg2 = twoscomplement(reg2);
    }
    
    reg2=twoscomplement(reg2);
    
    while(reg1>=reg)
    {
        quo++;
        reg1=addsubALU(reg1,reg2);
    }
    
    //cout<<"  "<<quo;
    if((regF&0x00000002)==1)
        quo=twoscomplement(quo);

    //printf(" %d ",quo);
    //Setting Zero FLAG
    regF = regF - (regF&0x00000008);
    if(quo == 0)
        regF = regF - (regF&0x00000008)+8;
    return quo;
}


void push (int stack[], int data)
{   printf("\n Push::%d",data);
    if (top == (MAX-1))
     {

         printf("\n Stack OverFlow while Inserting");
     }

    else
    {
    ++top;
    stack [top] = data;
    }
}

/*POP FUNCTION*/
int pop (int stack[])
{
    int ret;
    if (top == -1)
    {   ret = 0;

    printf("\n Stack OverFlow while Popping");
    }
    else
    {
    ret = stack [top];
    --top;
    }
return ret;
}

/*FUNCTION TO DISPLAY STACK*/
void display (int stack[])
{   int i;
    printf ("\nThe Stack is: ");
    if (top == -1)
    printf ("empty");
    else
    {   for (i=top; i>=0; --i)
       printf ("\n--------\n|%3d   |\n--------",stack[i]);
    }
    printf ("\n");
}


unsigned lea(unsigned reg1,unsigned reg2,unsigned s, unsigned d)    //Mem[Reg[Rb]+s*Reg[Ri]+d]  Reg[Rb]=reg2  Reg[Ri]=reg1
{   unsigned regri;
    printf("%d,%d,%d,%d",reg1,reg2,s,d);
    regri=mulALU(reg1,s);          //S * reg1
    printf("%d",regri);
    regri=addsubALU(regri,d);       //S*regri+d
    printf("%d,%d",reg2,regri);
    regri=addsubALU(3,9);   //reg2+s*regri+d
    printf("%d",regri);
    return(regri);        //Mem[Reg[Rb]+S*Reg[Ri]+D]
}


void decode()
{
    
    
    //Bitwise AND operation is done on the IR register to get the specific Destination Register.
    switch(regIR&0xFF000000)
    {
            //MOVE OPERATION
        case 0x00000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000:
                switch(regIR&0x0000FF00)
												{
                                                    case 0x00000000:regA = regA; break;
                                                    case 0x00000100:regA = regB; break;
                                                    case 0x00000200:regA = regC; break;
                                                    case 0x00000300:regA = regD; break;
                                                    default: printf("\nMove Instruction Error - Wrong Source Register!");
                                                } break;
            case 0x00010000:
                switch(regIR&0x0000FF00)
												{
                                                    case 0x00000000:regB = regA; break;
                                                    case 0x00000100:regB = regB; break;
                                                    case 0x00000200:regB = regC; break;
                                                    case 0x00000300:regB = regD; break;
                                                    default: printf("\nMove Instruction Error - Wrong Source Register!");
                                                } break;
            case 0x00020000:
                switch(regIR&0x0000FF00)
												{
                                                    case 0x00000000:regC = regA; break;
                                                    case 0x00000100:regC = regB; break;
                                                    case 0x00000200:regC = regC; break;
                                                    case 0x00000300:regC = regD; break;
                                                    default: printf("\nMove Instruction Error - Wrong Source Register!");
                                                } break;
            case 0x00030000:
                switch(regIR&0x0000FF00)
												{
                                                    case 0x00000000:regD = regA; break;
                                                    case 0x00000100:regD = regB; break;
                                                    case 0x00000200:regD = regC; break;
                                                    case 0x00000300:regD = regD; break;
                                                    default: printf("\nMove Instruction Error - Wrong Source Register!");
                                                } break;
            default: printf("\nMove Instruction Error - Wrong Destination Register!");
        }; break;
            
            //Immediate Addressing mode
        case 0x01000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: regA = regIR&0x0000FFFF; if(regA&0x00008000)regA=regA+0xFFFF0000;break;
            case 0x00010000: regB = regIR&0x0000FFFF; if(regB&0x00008000)regB=regB+0xFFFF0000;break;
            case 0x00020000: regC = regIR&0x0000FFFF; if(regC&0x00008000)regC=regC+0xFFFF0000;break;
            case 0x00030000: regD = regIR&0x0000FFFF; if(regD&0x00008000)regD=regD+0xFFFF0000;break;
            case 0x00040000: regE = regIR&0x0000FFFF; if(regE&0x00008000)regE=regE+0xFFFF0000;break;
            default: printf("\nMove Instruction Error - Wrong Destination Register!");break;
        }break;
            
            //LOAD OPERATION
        case 0x02000000:
            //The Address is first taken from the register and stored in MAR .. Data in MAR goes to MDR to Accumulator
            switch (regIR&0x0000FF00)
        {
            case 0x00000000: regMAR=regA; break;
            case 0x00000100: regMAR=regB; break;
            case 0x00000200: regMAR=regC; break;
            case 0x00000300: regMAR=regD; break;
            default: printf("\nLoad Instruction Error - Wrong Source Register!");break;
        }
            switch(regIR&0x00FF0000)
        {
            case 0x00000000:regA = mainMemory[regMAR]; break;
            case 0x00010000:regB = mainMemory[regMAR]; break;
            case 0x00020000:regC = mainMemory[regMAR]; break;
            case 0x00030000:regD = mainMemory[regMAR]; break;
            default: printf("\nLoad Instruction Error - Wrong Destination Register!");break;
        }
            break;
        case 0x03000000:
            //The Address is first taken from the Instruction Register and stored in MAR .. Data in MDR goes to the Destination Register
            if((regIR&0x0000FFF0)/16<256)
                printf("\nMemory Access Denied");
            else
            {
                regMAR = (regIR&0x0000FFF0)/16;
                switch (regIR&0x00FF0000)
                {
		                		case 0x00000000: regA = mainMemory[regMAR]; break;
		                		case 0x00010000: regB = mainMemory[regMAR]; break;
		                		case 0x00020000: regC = mainMemory[regMAR]; break;
		                		case 0x00030000: regD = mainMemory[regMAR]; break;
		                		default: printf("\nLoad Instruction Error - Wrong Destination Register!");break;
                }
            } break;
            
            //STORE OPERATIONS - 0x04------ and 0x05------
            // 0x04DDSS-- : STORE RegisterContainingDestinationMemoryAddress, RegisterContainingValue; DD - Destination Register, SS - Source Register containing Memory Address
        case 0x04000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: regMAR=regA; break;
            case 0x00010000: regMAR=regB; break;
            case 0x00020000: regMAR=regC; break;
            case 0x00030000: regMAR=regD; break;
            default: printf("\nStore Instruction Error!");break;
        }
            switch(regIR&0x0000FF00)
        {
            case 0x00000000: regMDR=regA; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
            case 0x00000100: regMDR=regB; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
            case 0x00000200: regMDR=regC; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
            case 0x00000300: regMDR=regD; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
        }
            break;
            
            //0x05------ : Direct Write to Memory
            //0x05DDDSS0 : STORE MemoryLocation, RegisterContainingValue; DDD - Destination Memory Location, SS- Register Containing Value
        case 0x05000000:
            if((regIR&0x00FFF000)/4096<256)
                printf("\nMemory Access Denied");
            else{
                switch(regIR&0x00000FF0)
                {
                    case 0x00000000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regA; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
                    case 0x00000010: regMAR=(regIR&0x00FFF000)/4096; regMDR = regB; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
                    case 0x00000020: regMAR=(regIR&0x00FFF000)/4096; regMDR = regC; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
                    case 0x00000030: regMAR=(regIR&0x00FFF000)/4096; regMDR = regD; writeToMemoryInterrupt(); mainMemory[regMAR] = regMDR; break;
                    default: printf("\nStore Instruction Error!");break;
                };}
            break;
        case 0x06000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regA=addsubALU(regA,regA);break;
                                                 case 0x00000100: regA=addsubALU(regA,regB);break;
                                                 case 0x00000200: regA=addsubALU(regA,regC);break;
                                                 case 0x00000300: regA=addsubALU(regA,regD);break;
                                             }break;
            case 0x00010000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regB=addsubALU(regB,regA);break;
                                                 case 0x00000100: regB=addsubALU(regB,regB);break;
                                                 case 0x00000200: regB=addsubALU(regB,regC);break;
                                                 case 0x00000300: regB=addsubALU(regB,regD);break;
                                             }break;
            case 0x00020000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regC=addsubALU(regC,regA);break;
                                                 case 0x00000100: regC=addsubALU(regC,regB);break;
                                                 case 0x00000200: regC=addsubALU(regC,regC);break;
                                                 case 0x00000300: regC=addsubALU(regC,regD);break;
                                             }break;
            case 0x00030000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regD=addsubALU(regD,regA);break;
                                                 case 0x00000100: regD=addsubALU(regD,regB);break;
                                                 case 0x00000200: regD=addsubALU(regD,regC);break;
                                                 case 0x00000300: regD=addsubALU(regD,regD);break;
                                             }break;
        }
            break;
        case 0x07000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: if(regIR&0x00008000)
                regA=addsubALU(regA,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regA=addsubALU(regA,regIR&0x0000FFFF);
                break;
            case 0x00010000: if(regIR&0x00008000)
                regB=addsubALU(regB,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regB=addsubALU(regB,regIR&0x0000FFFF);
                break;
            case 0x00020000: if(regIR&0x00008000)
                regC=addsubALU(regC,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regC=addsubALU(regC,regIR&0x0000FFFF);
                break;
            case 0x00030000: if(regIR&0x00008000)
                regD=addsubALU(regD,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regD=addsubALU(regD,regIR&0x0000FFFF);
                break;
        }
            break;
        case 0x08000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regA=addsubALU(regA,twoscomplement(regA));break;
                                                 case 0x00000100: regA=addsubALU(regA,twoscomplement(regB));break;
                                                 case 0x00000200: regA=addsubALU(regA,twoscomplement(regC));break;
                                                 case 0x00000300: regA=addsubALU(regA,twoscomplement(regD));break;
                                             }break;
            case 0x00010000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regB=addsubALU(regB,twoscomplement(regA));break;
                                                 case 0x00000100: regB=addsubALU(regB,twoscomplement(regB));break;
                                                 case 0x00000200: regB=addsubALU(regB,twoscomplement(regC));break;
                                                 case 0x00000300: regB=addsubALU(regB,twoscomplement(regD));break;
                                             }break;
            case 0x00020000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regC=addsubALU(regC,twoscomplement(regA));break;
                                                 case 0x00000100: regC=addsubALU(regC,twoscomplement(regB));break;
                                                 case 0x00000200: regC=addsubALU(regC,twoscomplement(regC));break;
                                                 case 0x00000300: regC=addsubALU(regC,twoscomplement(regD));break;
                                             }break;
            case 0x00030000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regD=addsubALU(regD,twoscomplement(regA));break;
                                                 case 0x00000100: regD=addsubALU(regD,twoscomplement(regB));break;
                                                 case 0x00000200: regD=addsubALU(regD,twoscomplement(regC));break;
                                                 case 0x00000300: regD=addsubALU(regD,twoscomplement(regD));break;
                                             }break;
        }
            break;
        case 0x09000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: if(regIR&0x00008000)
                regA=addsubALU(regA,twoscomplement((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regA=addsubALU(regA,twoscomplement(regIR&0x0000FFFF));
                break;
            case 0x00010000: if(regIR&0x00008000)
                regB=addsubALU(regB,twoscomplement((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regB=addsubALU(regB,twoscomplement(regIR&0x0000FFFF));
                break;
            case 0x00020000: if(regIR&0x00008000)
                regC=addsubALU(regC,twoscomplement((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regC=addsubALU(regC,twoscomplement(regIR&0x0000FFFF));
                break;
            case 0x00030000: if(regIR&0x00008000)
                regD=addsubALU(regD,twoscomplement((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regD=addsubALU(regD,twoscomplement(regIR&0x0000FFFF));
                break;
        }
            break;
        case 0x0a000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regA=mulALU(regA,regA);break;
                                                 case 0x00000100: regA=mulALU(regA,regB);break;
                                                 case 0x00000200: regA=mulALU(regA,regC);break;
                                                 case 0x00000300: regA=mulALU(regA,regD);break;
                                             }break;
            case 0x00010000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regB=mulALU(regB,regA);break;
                                                 case 0x00000100: regB=mulALU(regB,regB);break;
                                                 case 0x00000200: regB=mulALU(regB,regC);break;
                                                 case 0x00000300: regB=mulALU(regB,regD);break;
                                             }break;
            case 0x00020000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regC=mulALU(regC,regA);break;
                                                 case 0x00000100: regC=mulALU(regC,regB);break;
                                                 case 0x00000200: regC=mulALU(regC,regC);break;
                                                 case 0x00000300: regC=mulALU(regC,regD);break;
                                             }break;
            case 0x00030000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regD=mulALU(regD,regA);break;
                                                 case 0x00000100: regD=mulALU(regD,regB);break;
                                                 case 0x00000200: regD=mulALU(regD,regC);break;
                                                 case 0x00000300: regD=mulALU(regD,regD);break;
                                             }break;
        }
            break;
        case 0x0b000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: if(regIR&0x00008000)
                regA=mulALU(regA,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regA=mulALU(regA,(regIR&0x0000FFFF));
                break;
            case 0x00010000: if(regIR&0x00008000)
                regB=mulALU(regB,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regB=mulALU(regB,(regIR&0x0000FFFF));
                break;
            case 0x00020000: if(regIR&0x00008000)
                regC=mulALU(regC,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regC=mulALU(regC,(regIR&0x0000FFFF));
                break;
            case 0x00030000: if(regIR&0x00008000)
                regD=mulALU(regD,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regD=mulALU(regD,(regIR&0x0000FFFF));
                break;
        }
            break;
        case 0x0c000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regA=divALU(regA,regA);break;
                                                 case 0x00000100: regA=divALU(regA,regB);break;
                                                 case 0x00000200: regA=divALU(regA,regC);break;
                                                 case 0x00000300: regA=divALU(regA,regD);break;
                                             }break;
            case 0x00010000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regB=divALU(regB,regA);break;
                                                 case 0x00000100: regB=divALU(regB,regB);break;
                                                 case 0x00000200: regB=divALU(regB,regC);break;
                                                 case 0x00000300: regB=divALU(regB,regD);break;
                                             }break;
            case 0x00020000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regC=divALU(regC,regA);break;
                                                 case 0x00000100: regC=divALU(regC,regB);break;
                                                 case 0x00000200: regC=divALU(regC,regC);break;
                                                 case 0x00000300: regC=divALU(regC,regD);break;
                                             }break;
            case 0x00030000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regD=divALU(regD,regA);break;
                                                 case 0x00000100: regD=divALU(regD,regB);break;
                                                 case 0x00000200: regD=divALU(regD,regC);break;
                                                 case 0x00000300: regD=divALU(regD,regD);break;
                                             }break;
        }
            break;
        case 0x0d000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: if(regIR&0x00008000)
                regA=divALU(regA,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regA=divALU(regA,(regIR&0x0000FFFF));
                break;
            case 0x00010000: if(regIR&0x00008000)
                regB=divALU(regB,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regB=divALU(regB,(regIR&0x0000FFFF));
                break;
            case 0x00020000: if(regIR&0x00008000)
                regC=divALU(regC,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regC=divALU(regC,(regIR&0x0000FFFF));
                break;
            case 0x00030000: if(regIR&0x00008000)
                regD=divALU(regD,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regD=divALU(regD,(regIR&0x0000FFFF));
                break;				 
        }
            break;
        case 0x0e000000:	
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regA=modALU(regA,regA);break;	
                                                 case 0x00000100: regA=modALU(regA,regB);break;
                                                 case 0x00000200: regA=modALU(regA,regC);break;
                                                 case 0x00000300: regA=modALU(regA,regD);break;
                                             }break;
            case 0x00010000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regB=modALU(regB,regA);break;
                                                 case 0x00000100: regB=modALU(regB,regB);break;
                                                 case 0x00000200: regB=modALU(regB,regC);break;
                                                 case 0x00000300: regB=modALU(regB,regD);break;
                                             }break;
            case 0x00020000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regC=modALU(regC,regA);break;
                                                 case 0x00000100: regC=modALU(regC,regB);break;
                                                 case 0x00000200: regC=modALU(regC,regC);break;
                                                 case 0x00000300: regC=modALU(regC,regD);break;
                                             }break;
            case 0x00030000: switch(regIR&0x0000FF00)
											 {
                                                 case 0x00000000: regD=modALU(regD,regA);break;
                                                 case 0x00000100: regD=modALU(regD,regB);break;
                                                 case 0x00000200: regD=modALU(regD,regC);break;
                                                 case 0x00000300: regD=modALU(regD,regD);break;
                                             }break;				 
        }
            break;
        case 0x0f000000:
            switch(regIR&0x00FF0000)
        {
            case 0x00000000: if(regIR&0x00008000)
                regA=modALU(regA,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regA=modALU(regA,(regIR&0x0000FFFF));
                break;
            case 0x00010000: if(regIR&0x00008000)
                regB=modALU(regB,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regB=modALU(regB,(regIR&0x0000FFFF));
                break;
            case 0x00020000: if(regIR&0x00008000)
                regC=modALU(regC,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regC=modALU(regC,(regIR&0x0000FFFF));
                break;
            case 0x00030000: if(regIR&0x00008000)
                regD=modALU(regD,((regIR&0x0000FFFF)+0xFFFF0000));
											 else
                                                 regD=modALU(regD,(regIR&0x0000FFFF));
                break;				 
        }
            break;
        case (0x10000000):
                          printf("\n Entered 09000000 (stack) case and regIR=0x%x \n",regIR);
                          if((regIR&0x00FFF0000)/4096<256)
                             printf("\nMemory Access Denied");
                          else{
                             printf("%d",((regIR&0x00FFF000)/4096));
                             switch(regIR&0x00FF0000)
                             {
                                case 0x00000000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regA; push(stack,regMDR); display(stack); break;
                                case 0x00010000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regB; push(stack,regMDR); display(stack); break;
                                case 0x00100000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regC; push(stack,regMDR); display(stack); break;
                                case 0x00110000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regD; push(stack,regMDR); display(stack); break;
                                case 0x000A0000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regDS; push(stack,regMDR); display(stack); break;
                                case 0x000B0000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regES; push(stack,regMDR); display(stack); break;
                                case 0x000C0000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regSS; push(stack,regMDR); display(stack); break;
                                case 0x000D0000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regIR&0x0000FFFF; push(stack,regMDR); display(stack); break;
                                case 0x000E0000: regMAR=(regIR&0x00FFF000)/4096; regMDR = regIR&0x0000FFFF; regMAR=regMDR;push(stack,mainMemory[regMAR]);display(stack);break;
                                default: printf("\nStore Instruction Error!");break;
                             };
                             }
                            break;

        case (0x11000000):
                              printf("\n Entered 0A000000 (stack) POP case and regIR=0x%x \n",regIR);
                              if((regIR&0x00FFF0000)/4096<256)
                              printf("\nMemory Access Denied");
                              else
                              {
                              printf("%d",((regIR&0x00FFF000)/4096));
                              switch(regIR&0x00FF0000)
                              {
                    //printf("inside pop");
                                case 0x00000000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regA=regMDR;display(stack); break;
                                case 0x00010000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regB=regMDR; display(stack); break;
                                case 0x00100000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regC=regMDR; display(stack); break;
                                case 0x00110000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regD=regMDR;display(stack); break;
                                case 0x001A0000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regDS=regMDR; display(stack); break;
                                case 0x001B0000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regES=regMDR; display(stack); break;
                                case 0x001C0000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack); regSS=regMDR; display(stack); break;
                                case 0x001D0000: regMAR=(regIR&0x00FFF000)/4096;
                                printf("\n Immediate value is",pop(stack)); display(stack); break;
                                case 0x001E0000: regMAR=(regIR&0x00FFF000)/4096; regMDR=pop(stack);regMAR = regIR&0x0000FFFF ; 
                                printf("\n Immediate value is",mainMemory[regMAR]); display(stack); break;
        
                                default: printf("\nStore Instruction Error!");break;
                   // printf("outside pop");
                                };}
                                break;
        case (0x12000000):if(regF&0x00000001)regPC=regIR&0x00FFFFFF;break;
        case (0x13000000):if(!(regF&0x00000001))regPC=regIR&0x00FFFFFF;break;
        case (0x14000000):if(regF&0x00000002)regPC=regIR&0x00FFFFFF;break;
        case (0x15000000):if(!(regF&0x00000002))regPC=regIR&0x00FFFFFF;break;
        case (0x16000000):if(regF&0x00000004)regPC=regIR&0x00FFFFFF;break;
        case (0x17000000):if(!(regF&0x00000004))regPC=regIR&0x00FFFFFF;break;
        case (0x18000000):if(regF&0x00000008)regPC=regIR&0x00FFFFFF;break;
        case (0x19000000):if(!(regF&0x00000008))regPC=regIR&0x00FFFFFF;break;
        case (0x1a000000):printf("CALLED%d",regF);if(regF&0x00000008){regPC=regIR&0x00FFFFFF;printf("!!!!!ZF!!!!!! %d !!!!!!!!!!!!",regPC);}break;
        case (0x1b000000):if(!(regF&0x00000008))regPC=regIR&0x00FFFFFF;break;
        case (0x1c000000):if(regF&0x00000008 & (regF&0x00000002==regF&0x00000004))regPC=regIR&0x00FFFFFF;break;
        case (0x1d000000):if(!regF&0x00000008 & !(regF&0x00000002==regF&0x00000004))regPC=regIR&0x00FFFFFF;break;
        case (0x1e000000):regPC= regIR&0x00FFFFFF;break;
    
        case (0x1f000000):  switch(regIR&0x00FF0000)
                            {
                               case 0x00000000: switch(regIR&0x00001000)
                                                {
                                                    case 0x00000000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regA=lea(regA,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regA=lea(regA,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regA=lea(regA,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regA=lea(regA,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00001000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regA=lea(regB,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regA=lea(regB,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regA=lea(regB,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regA=lea(regB,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00002000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regA=lea(regC,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regA=lea(regC,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regA=lea(regC,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regA=lea(regC,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00003000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regA=lea(regD,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regA=lea(regD,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regA=lea(regD,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regA=lea(regD,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                }break;                                             
                               case 0x00010000: switch(regIR&0x00001000)
                                                {
                                                    case 0x00000000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regB=lea(regA,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regB=lea(regA,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regB=lea(regA,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regB=lea(regA,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00001000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regB=lea(regB,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regB=lea(regB,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regB=lea(regB,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regB=lea(regB,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00002000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regB=lea(regC,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regB=lea(regC,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regB=lea(regC,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regB=lea(regC,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00003000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regB=lea(regD,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regB=lea(regD,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regB=lea(regD,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regB=lea(regD,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                }break;   
                               case 0x00020000: switch(regIR&0x00001000)
                                                {
                                                    case 0x00000000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regC=lea(regA,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regC=lea(regA,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regC=lea(regA,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regC=lea(regA,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00001000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regC=lea(regB,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regC=lea(regB,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regC=lea(regB,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regC=lea(regB,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00002000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regC=lea(regC,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regC=lea(regC,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regC=lea(regC,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regC=lea(regC,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00003000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regC=lea(regD,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regC=lea(regD,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regC=lea(regD,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regC=lea(regD,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                }break;
                               case 0x00030000: switch(regIR&0x00001000)
                                                {
                                                    case 0x00000000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regD=lea(regA,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regD=lea(regA,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regD=lea(regA,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regD=lea(regA,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00001000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regD=lea(regB,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regD=lea(regB,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regD=lea(regB,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regD=lea(regB,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00002000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regD=lea(regC,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regD=lea(regC,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regD=lea(regC,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regD=lea(regC,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                    case 0x00003000:switch(regIR&0x00000100)
                                                                    {
                                                                        case 0x00000000:regD=lea(regD,regA,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000100:regD=lea(regD,regB,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000200:regD=lea(regD,regC,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                        case 0x00000300:regD=lea(regD,regD,(regIR&0x000000F0)/16,regIR&0x0000000F);break;
                                                                    }break;
                                                }break; 
                            }break;
        case (0x20000000):
                            break;
        case (0x21000000):break;
        default: printf("\nInstruction Error!"); break;
    }
}

char regInputBufferString[50]={'0','0','0','0','0','0','0','0','0','0'};
    //unsigned regInputBuffer;
    //int i=0;


void inputDevice()
{
    
    //printf("\nEnter your Code:\n");
    //while(strcmp(gets(regInputBufferString), "0xff000000"))
    {
        
        //As the instructions are entered, it is stored in the InputBuffer in the Input device and then moved to regIR.
        regInputBuffer=convertStringToHex(regInputBufferString);
        
        //Onpressing Enter an interrupt is sent by the input device to the CPU
        
        //Data is placed in the data line on the next cycle and the instruction register gets the input instruction.
        cache[regIP]=regInputBuffer;
        regIP++;
    }
    //printf("<%d>",regPC);
}

void fetch()
{
    //printf("%d",regPC);
    while(regPC<regIP)
    {
    regIR = cache[regPC];
    regPC++;
    decode();
    //printf("<%d>",regIR);
    printMemory();
    }
    
}

void bootXtar()
{
    //All registers will have random values when switched on.
    //Boot memeory
    //0 - 255 Boot Index of the mainMemory[]; 256 - 511 Operating System ; 512 - 1024 Application
    //Operating System is loaded and the address values are set.
    
    //Initialization of the registers are done by the Operating System
    regMAR=0x00000201; 
    regSP=0x00000200;
    regES=0x00000234;
    regBP=0x00000100;
    regF=0x00000000;
    mainMemory[300] = 0x00000221;
    regIP=0;
    regPC=0;
}
void loadInterpreter()
{
    //memory space 101 - 150 reserved for interpreter
    regIR=0x01000500;
    decode();
    regIR=0x0515b000;
    decode();
    printf("||%d||",regIR);
    regMAR;
}
unsigned hexInp;

void Interpreter()
{ 
    char line[20],variables[10],hexInp;
    int i=0,numberOfvariables,index=0,inp,hashKey;
    printf("\nEnter T-code:\n");
    //scanf("%d",numberOfvariables);
    //gets(variables);
    gets(line);
    while(strcmp(line,"eol"))
    {
    //scanf("%s",&line);
    //printf("33");
    inp = line[0];
    //printf("%d",inp);
    while(i!=sizeof(line))
    {
        inp = line[i];
        //printf("%d",inp);
        //printf("%d",inp);
        if(inp=='[')
        {
                hexInp = line[i];
                hashKey = 256+hexInp;
                regB = (mainMemory[hashKey])*16*16*16*16;
                //printf("L%dL",regB);
                i++; 
                hexInp = line[i];
                hashKey = 256+hexInp;
                regB = regB + hashKey*4096;
                
        }
        if(inp=='{')
        {
            i++;
            hexInp=line[i];
            hashKey=256+hexInp;
            //output the data
            printf("%d",mainMemory[hashKey]);
        }
        if(inp=='=')
        {
            i++;
            hexInp=line[i];
            if(hexInp=='(')
            {
                i++;
                hexInp=line[i];
                hashKey=256+hexInp;
                regD=mainMemory[hashKey];
                i++;
                hexInp=line[++i];
                hashKey=256+hexInp;
                regD=regD + mainMemory[hashKey];
                regA = regD;
            }
            else
            {
                hexInp = hexInp-48;
                regA = hexInp;
                //regB = regB + regC*16;
                //printf("<<<%d>>>",regB);
            }
        }
        if(inp == ']')
        {
            regIR=regB;
            decode();
        }
        //if(inp == ')')
        {
            //regIR=regD;
            //decode();
        }
        i++;
    }
    gets(line);
    i=0;
    }
}
int main()
{
    char instruction;
    int hexNumber=0;
    
    printf("\n32-bit Xtar CPU Manual :");
    printf("\n");
    printf("\n Instructions:");
    printf("\n MOVE DestinationRegister, SourceRegister;            :   0x00DDSS--");
    printf("\n MOVE DestinationRegister, Value;                     :   0x01DDVVVV");
    printf("\n LOAD DestinationRegister, SourceRegister;            :   0x02DDSS--");
    printf("\n LOAD DestinationRegister, DirectSourceMemoryAddress; :   0x03DDSSS-");
    printf("\n STORE RegisterDestinationMemAddress, SourceRegister; :   0x04DDSS--");
    printf("\n STORE DirectDestinationMemAddress, SourceRegister;   :   0x05DDDSS-");
    printf("\n ADD DestinationRegister, SecondRegister              :   0x06DDSS--");
    printf("\n ADD DestinationRegister, ImmediateValue              :   0x07DDVVVV");
    printf("\n SUB DestinationRegister, SecondRegister              :   0x08DDSS--");
    printf("\n SUB DestinationRegister, ImmediateValue              :   0x09DDVVVV");
    printf("\n MUL DestinationRegister, SecondRegister              :   0x0aDDSS--");
    printf("\n MUL DestinationRegister, ImmediateValue              :   0x0bDDVVVV");
    printf("\n DIV DestinationRegister, SecondRegister              :   0x0cDDSS--");
    printf("\n DIV DestinationRegister, ImmediateValue              :   0x0dDDVVVV");
    printf("\n MOD DestinationRegister, SecondRegister              :   0x0eDDSS--");
    printf("\n MOD DestinationRegister, ImmediateValue              :   0x0fDDVVVV");
    printf("\n Stack operation PUSH, From reg to memory;            :   0x10DDSSSS");
    printf("\n Stack operation PUSH, From SpecialReg to Stack;      :   0x10DDSSSS");
    printf("\n Stack operation PUSH, immediate data to Stack;       :   0x10DDSSSS");
    printf("\n Stack operation PUSH, From memory to Stack;          :   0x10DDSSSS");
    printf("\n Stack operation POP, From reg to memory;             :   0x11DDSSSS");
    printf("\n Stack operation POP, From SpecialReg to Stack;       :   0x11DDSSSS");
    printf("\n Stack operation POP, immediate data to Stack;        :   0x11DDSSSS");
    printf("\n Stack operation POP, From memory to Stack;           :   0x11DDSSSS");
    printf("\n JUMP IF CARRY                                        :   0x12DDSSSS");
    printf("\n JUMP IF NO CARRY                                     :   0x13DDSSSS");
    printf("\n JUMP IF SIGN                                         :   0x14DDSSSS");
    printf("\n JUMP IF NO SIGN                                      :   0x15DDSSSS");
    printf("\n JUMP IF OVERFLOW                                     :   0x16DDSSSS");
    printf("\n JUMP IF NO OVERFLOW                                  :   0x17DDSSSS");
    printf("\n JUMP IF EQUAL                                        :   0x18DDSSSS");
    printf("\n JUMP IF NOT EQUAL                                    :   0x19DDSSSS");
    printf("\n JUMP IF ZERO                                         :   0x1aDDSSSS");
    printf("\n JUMP IF NOT ZERO                                     :   0x1bDDSSSS");
    printf("\n JUMP IF GREATER THAN                                 :   0x1cDDSSSS");
    printf("\n JUMP IF LESSER THAN                                  :   0x1dDDSSSS");
    printf("\n JUMP                                                 :   0x1eDDSSSS");
    printf("\n LOAD EFFECTIVE ADDRESS (S1-Scale(1,2,4 or 8) & D1-Displacement(0,1,2,4))  :   0x1fDDRrSD");
    printf("\n LABEL                                                :   0x20LLLLLL");
    
    printf("\n Registers:\n regA - 00 \n regB - 01 \n regC - 02 \n regD - 03");
    printf("\n DirectDestinationMemAddress and DirectSourceMemoryAddress should be 3 hex digits long(12 bits).");
    
    printf("\n\n\nBooting the CPU...<%d>",((7%2)&(3%2)));
    bootXtar();
    printf("Booted!");
    printf("Loaing Interpreter ... !");
    char ch;
    //scanf("%c",&ch);
    //printf("%d",ch);
    loadInterpreter();
    Interpreter();
   //inputDevice();
    //fetch();
    int inp=91,index=0;
    char hexInp[10];
    while(inp) {
        hexInp[index++] = inp % 16 + '0';
        inp /= 16;}
        printf("%s",&hexInp);
    return 0;
}

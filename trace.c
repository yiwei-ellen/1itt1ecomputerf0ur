/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

int ReadObjectFile(char* filename, MachineState* CPU);
void Reset(MachineState* CPU);
int UpdateMachineState(MachineState* CPU, FILE* output);
int main(int argc, char** argv)
{
    // this assignment does NOT require dynamic memory - do not use 'malloc() or free()'
    // create a local varible in main to represent the CPU and then pass it around
    // to subsequent functions using a pointer!
    
    /* notice: there are test cases for part 1 & part 2 here on codio
       they are located under folders called: p1_test_cases & p2_test_cases
    
       once you code main() to open up files, you can access the test cases by typing:
       ./trace p1_test_cases/divide.obj   - divide.obj is just an example
    
       please note part 1 test case files have an OBJ and a TXT file
       the .TXT shows you what's in the .OBJ files
    
       part 2 test case files have an OBJ, an ASM, and a TXT files
       the .OBJ is what you must read in
       the .ASM is the original assembly file that the OBJ was produced from
       the .TXT file is the expected output of your simulator for the given OBJ file
    */
    int i;
    char* input_file;
    MachineState *ptr, CPU;
    FILE *output;
    char *ext;
    ptr = &CPU;   
    if (argc<3){
        printf("command line error: not long enough input");
        return (-1);
    }
    //check txt file name input

    ext = strrchr(argv[1], '.');
    //test if the file is of .txt format
    if (!ext||(strcmp(ext, ".TXT")!=0 && (strcmp(ext, ".txt")!=0) )){ 
        printf("error: input txt file name not fit format\n");
        return (-1);
    } else {
        output = fopen(argv[1],"w");
    }
    for (i = 0;i<65536;i++){
        ptr->memory[i] = 0;
    }
    
    //load memory and return if there is filename error
    for(i=2;i<argc;i++){
        input_file = argv[i];
        ext = strrchr(argv[i], '.');
        if (!ext||(strcmp(ext, ".OBJ")!=0 && (strcmp(ext, ".obj")!=0) )){ 
            printf("error: input obj file name not fit format");
            return (-1);
        } 
        if ( ReadObjectFile(input_file, ptr) == -1){//immediate check if read file successful
            return(-1);
        }
    }
    Reset(ptr);
    while(ptr->PC != 0x80FF){
        /*
        if(UpdateMachineState(ptr,output)==-1){
            break;
        };*/
        UpdateMachineState(ptr,output);
    }
    fclose(output);
    return 0;
}
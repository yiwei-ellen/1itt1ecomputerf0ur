 /*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
  FILE *src_file;
  int prog_type;
  int b1,b2; //creating 16 bit instruction
  int len; //to save <n>
  int i; //counter

  src_file = fopen (filename, "rb");
  if (src_file == NULL){
      printf("error: empty file or wrong filename \n");
      return (-1);
  }
  b1 = fgetc(src_file);
  b2 = fgetc(src_file);
  prog_type = (b1 << 8)| b2;


  //printf("something running, 0x%x \n",prog_type);
  while(feof(src_file)==0){
    //printf("new program, 0x%x \n",prog_type);
    
    //match prog_type with all cases possible then initialize memory or do nothing as needed
    if(prog_type == 0xCADE || prog_type==0xDADA){
      b1 = fgetc(src_file);
      b2 = fgetc(src_file); 
      memoryAddress = (b1 << 8)| b2;
      b1 = fgetc(src_file);
      b2 = fgetc(src_file); 
      len = (b1<<8)| b2;

      for (i=0; i<len; i++){
        b1 = fgetc(src_file) ;
        b2 = fgetc(src_file) ;
        /*
        if (((b1 << 8) | b2)!=0){
          printf("address: %d content: 0x%x \n",memoryAddress+i,(b1 << 8) | b2);
        }
        */
        CPU->memory[memoryAddress + i] = (b1 << 8) | b2;
      }

    } else if (prog_type == 0xC3B7){
      b1 = fgetc(src_file);
      b2 = fgetc(src_file); 
      b1 = fgetc(src_file);
      b2 = fgetc(src_file); 
      len = (b1<<8)| b2;

      for (i=0; i<len; i++){
        fgetc(src_file) ;
      }

    } else if (prog_type ==0xF17E){
      b1 = fgetc(src_file);
      b2 = fgetc(src_file); 
      len = (b1<<8)| b2;

      for (i=0; i<len; i++){
        fgetc(src_file) ;
      }
    } else if (prog_type ==0x715E){
        fgetc(src_file) ;
        fgetc(src_file) ;
        fgetc(src_file) ;
        fgetc(src_file) ;
        fgetc(src_file) ;
        fgetc(src_file) ;
    }
    //check if there are new sections coming up
    if (feof(src_file)==0){
      b1 = fgetc(src_file) ;
    }
    if (feof(src_file)==0){
      b2 = fgetc(src_file) ;
      prog_type = (b1<<8)| b2;
    }
  }
  fclose(src_file);
  return 0;
}

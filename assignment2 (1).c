#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* extractopcode(char *buff);
int getfunct(char *inst);
int getcode(char *inst);
int getregcode(char *reg);

//to keep labels and corresponding address
typedef struct{
    char name[20];
    int loca;
} labels;
labels mylabels[10];

int location(char* label, labels mylabels[], int numlabels);
   
int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage: ./l1 <filename>\n");
        exit(0);
    }
    
    char *buff= (char*)malloc(sizeof(char)*1024);
    printf("Assembly Language Program:\n");
    FILE *f = fopen(argv[1],"r");
    if (f == NULL)
    {
        printf("Error in opening file\n");
        exit(0);
    }
    while (fgets(buff,1023,f) != NULL){
    	fputs(buff,stdout);
    }
    fclose(f);
    
    FILE *f1 = fopen(argv[1], "r");
    if (f1 == NULL)
    {
        printf("Error in opening file\n");
        exit(0);
    }
    int value= 0x00400000;
    int numlabels=0;
    while (fgets(buff,1023,f1) != NULL){
    	int l=0;
    	while (buff[l] != '\n'){ l++;} //read line
        if (buff[l-1] == ':'){ //check if last char is : then get code
            int k=0;
            while (buff[k] != '\n'){
                mylabels[numlabels].name[k++] = buff[k++];
            }
            mylabels[numlabels].loca = value;
            numlabels++;
            continue;
        }
        value += 0x04;
    }
    fclose(f1);
    
    printf("\n");
    FILE *f2 = fopen(argv[1], "r");
    value= 0x00400000;
    while (fgets(buff,1023,f2) != NULL){
	
	unsigned int machine_code = 0;
        int i=0;
        int j=0;
        int target=0;
        while (buff[i] != '\n'){ i++;} //read line
        if (buff[i-1] == ':'){ //check if last char is : then get code
            continue;
        }

        printf("0x%X   ",value);
        char *opcode = extractopcode(buff);
        int mopcode=getcode(opcode);
        int n=0;
	while (opcode[n] != '\0'){ n++;}
	n--;
	if (!strcmp(opcode,"j")){
		i++;
            	j=0;
            	char *label1=(char*)malloc(sizeof(char)*1024);
            	while(buff[i] != '\n'){
                	label1[j++] = buff[i++];
            	}
            	target = location(label1,mylabels,numlabels)/4;
		machine_code= (mopcode << 26) | target;
	}
        else { //means addi or andi with immediate
            i =0;
            while (buff[i] != '$'){ i++;}
            char *val=(char*)malloc(sizeof(char)*6);
            j=0;
            while(buff[i] != ','){
                val[j++] = buff[i++];
            }
            int reg1 = getregcode(val);
            int offsetval =0;
            while (buff[i] != '$'){ i++;}
            j=0;
            char *val1=(char*)malloc(sizeof(char)*6);
            while((buff[i] != ',' && buff[i] != ')')){
                val1[j++] = buff[i++];
            }
            int reg2 = getregcode(val1);
            //check if for diff inst
            if (opcode[n] == 'i'){ //immediate instructions addi,andi,ori
            	i++;
            	j=0;
            	char *val2=(char*)malloc(sizeof(char)*6);
            	while(buff[i] != '\n'){
                	val2[j++] = buff[i++];
            	}
            	int imm = atoi(val2);
            	machine_code=(mopcode << 26) | (reg1 <<21) | (reg2 << 16) | imm;
            }
            else if(!strcmp(opcode,"beq")){
            	i++;
            	j=0;
            	char *label=(char*)malloc(sizeof(char)*1024);
            	while(buff[i] != '\n'){
                	label[j++] = buff[i++];
            	}
            	offsetval=value-location(label,mylabels,numlabels); //find offset val through label
            	free(label);
            	machine_code=(mopcode << 26) | (reg1 <<21) | (reg2 << 16) | offsetval;
            }else if(opcode[n] == 'w'){  //for load or store instruction
            	machine_code=(mopcode << 26) | (reg1 <<21) | (reg2 << 16) | offsetval;
            }else{ //for other instructions
             	int shamt=0x0;
             	int funct= getfunct(opcode);
             	//call reg3
             	while (buff[i] != '$'){ i++;}
            	j=0;
            	char *val4=(char*)malloc(sizeof(char)*6);
            	while(buff[i] != '\n'){
                	val4[j++] = buff[i++];
            	}
            	int reg3 = getregcode(val4);
            	machine_code=(mopcode << 26) | (reg1 <<21) | (reg2 << 16) | (reg3 << 11) | (shamt<< 6) | funct;
            }
        }
        printf("0x%08X\t",machine_code);
        fputs(buff,stdout);
        value += 0x04;
    }

    fclose(f2);
    free(buff); // Free allocated memory
    return 0;
}

int location(char* label, labels mylabels[], int numlabels){
	for (int i=0; i<numlabels; i++){
		if (!strcmp(mylabels[i].name, label)){
			return mylabels[i].loca;
		}
	}
	return -1;
}

char* extractopcode(char *buff){
    int i=0;
    int j=0;
    char *inst= (char*)malloc(sizeof(char)*45);
    while (buff[i] < 'a' || buff[i] > 'z'){ i++;}
    while (buff[i] != ' '){ //read and store instruction name in inst
        inst[j++] = buff[i++];
    }
    inst[j] = '\0';
    return inst;
}

int getregcode(char *reg){
    if (!strcmp(reg,"$zero")){ return 0;}
    else if (!strcmp(reg,"$v0")){ return 1;}
    else if (!strcmp(reg,"$v1")){ return 2;}
    else if (!strcmp(reg,"$v2")){ return 3;}
    else if (!strcmp(reg,"$a0")){ return 4;}
    else if (!strcmp(reg,"$a1")){ return 5;}
    else if (!strcmp(reg,"$a2")){ return 6;}
    else if (!strcmp(reg,"$a3")){ return 7;}
    else if (!strcmp(reg,"$t0")){ return 8;}
    else if (!strcmp(reg,"$t1")){ return 9;}
    else if (!strcmp(reg,"$t2")){ return 10;}
    else if (!strcmp(reg,"$t3")){ return 11;}
    else if (!strcmp(reg,"$t4")){ return 12;}
    else if (!strcmp(reg,"$t5")){ return 13;}
    else if (!strcmp(reg,"$t6")){ return 14;}
    else if (!strcmp(reg,"$t7")){ return 15;}
    else if (!strcmp(reg,"$s0")){ return 16;}
    else if (!strcmp(reg,"$s1")){ return 17;}
    else if (!strcmp(reg,"$s2")){ return 18;}
    else if (!strcmp(reg,"$s3")){ return 19;}
    else if (!strcmp(reg,"$s4")){ return 20;}
    else if (!strcmp(reg,"$s5")){ return 21;}
    else if (!strcmp(reg,"$s6")){ return 22;}
    else if (!strcmp(reg,"$s7")){ return 23;}
    else if (!strcmp(reg,"$t8")){ return 24;}
    else if (!strcmp(reg,"$t9")){ return 25;}
    else if (!strcmp(reg,"$k0")){ return 26;}
    else if (!strcmp(reg,"$k1")){ return 27;}
    else if (!strcmp(reg,"$gp")){ return 28;}
    else if (!strcmp(reg,"$sp")){ return 29;}
    else if (!strcmp(reg,"$fp")){ return 30;}
    else if (!strcmp(reg,"$ra")){ return 31;}
    else return -1; // Invalid register
}
int getfunct(char *inst){
    if (inst[0]=='a' && inst[1]=='n' && inst[2]=='d'){ return 0x24; }
    else if (inst[0]=='a' && inst[1]=='d' && inst[2]=='d'){ return 0x20; }
    else if (inst[0]=='o' && inst[1]=='r'){ return 0X25; }
    else if (inst[0]=='x' && inst[1]=='o' && inst[2]=='r'){ return 0X26; }
    else if (inst[0]=='s' && inst[1]=='l' && inst[2]=='t'){ return 0X2A; }
    else { return 0x0;}
}
int getcode(char *inst){
    if (inst[0]=='a' && inst[1]=='d' && inst[2]=='d' && inst[3]=='i'){ return 0x08; }
    else if (inst[0]=='a' && inst[1]=='n' && inst[2]=='d' && inst[3]=='i'){ return 0x0C; }
    else if (inst[0]=='o' && inst[1]=='r' && inst[2]=='i'){ return 0x0D; }
    else if (inst[0]=='b' && inst[1]=='e' && inst[2]=='q'){ return 0X04; }
    else if (inst[0]=='l' && inst[1]=='w'){ return 0X23; }
    else if (inst[0]=='s' && inst[1]=='w'){ return 0X2B; }
    else if (inst[0]=='j'){ return 0X02; }
    else { return 0x0;}
}

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "mymat.h"
#define NUM_SEG 3
#define NUM_COMMANDS 7
#define MAX_LEN 9
#define MAX_COMMAND 10
#define MAX_ERROR 50
#define MAX_INPUT 50

typedef struct
{
    char* input;
    char* parsed;
    char* command;
    char* error;
    char** seg;
    int type;
    int valid;
    int args;
} ParsedLine;

typedef struct
{
    void (*parse)(ParsedLine*, Matrix**);
} TypeParser;


char* commands[] = {"read_mat", "print_mat", "mul_scalar", "trans_mat", "add_mat", "sub_mat", "mul_mat"};
char* matrices[] = {"MAT_A", "MAT_B", "MAT_C", "MAT_D", "MAT_E", "MAT_F"};


Matrix* mat_by_name(char* name, Matrix* arr[]) {
    int i;

    for(i = 0; i < 3; i++) {
        if(!strcmp(name, arr[i]->name)) {
            return arr[i];
        }
    }

    return arr[0];
}


/*
Clears all fields of a pointer to ParsedLine 
*/
void clear_line_mem(ParsedLine* line) {
    int i;

    memset(line->input,0,strlen(line->input));
    memset(line->parsed,0,strlen(line->parsed));
    
    for(i = 0; i < NUM_SEG; i++) {
        memset(line->seg[i],0,strlen(line->seg[i]));
    }

    line->valid = 1;
}

/*
Checks if a string is a valid command, returns 1 if it is and 0 if it isn't 
*/
int check_command(char* command) {
    int i;

    for(i = 0; i < NUM_COMMANDS; i++) {
        if(!strcmp(command, commands[i])) return 1;
    }
    return 0;
}
/*
Checks if a string is a valid matrix name, returns 1 if it is and 0 if it isn't 
*/
int check_name(char* name) {
    int i;

    for(i = 0; i < 5; i++) {
        if(!strcmp(name, matrices[i])) return 1;
    }
    return 0;
}
/*
Removes spaces and store the result in line->seg 
*/
void remove_spaces(ParsedLine* line) {
    int i,len,j = 0;
    len = strlen(line->input);

    for(i = 0; i < len; i++) {
        if(!isspace(line->input[i])) {
            line->parsed[j++] = line->input[i];
        }
    }
}
void error(ParsedLine* line, char* error) {
    line->valid = 0;
    line->error = error;
    return;
}


/*
Check if the spacing are valid
*/
int valid_space(ParsedLine* line, int idx, int pos, int first_pass) {
    int i = pos; /* pos is the position that we'll start at */
    int j;
    while(isspace(line->input[i])) i++; /* Pass on the spaces */
    
    if(i == strlen(line->input) + 1) return 1;

    for(j = 0; j < idx; j++) {
        if(isspace(line->input[i + j])) {
           error(line,"Invalid space position");
            return 0;
        } /* Check if there are a space in between characters */
    }

    if(first_pass) {
        if(!isspace(line->input[i + idx])) { /* For the first pass, check if there is at least one space after the command */
            error(line,"Missing spaces between the command and the arguments");
            return 0;
        }
    } 

    if(i + idx == strlen(line->input)) return 1; /* Breaking condition for the recursion */

    if(!first_pass) {
        while(isspace(line->input[i + idx])) i++; /* Pass on the spaces, until the next batch */
        i += 1;
    }

    valid_space(line, 5, i + idx, 0); /* After each call, we'll want to check the same thing, and since all of the matrix names are of length 5, we're good */

    return 0;
}
/*
A modified version of valid_space, adjusted for all five types of parsing
*/
int valid_space_modified(ParsedLine* line) {
    int i = 0, j = 0, k = 0;

    while(isspace(line->input[i])) i++;

    for(j = 0; j < strlen(line->command); j++) {
        if(isspace(line->input[i + j])) {
            error(line, "Invalid spacing");            
            return 0;
        }
    }

    i += j;

    if(!isspace(line->input[i]) && line->type != 2) {
        error(line,"Missing space between command and arguments");
        return 0;
    }

    while(i < strlen(line->input) && k < line->args) {
        while(isspace(line->input[i])) i++;
        
        for(j = 0; j < strlen(line->seg[k]); j++) {
            if(isspace(line->input[i + j])) {
                error(line, "Invalid spacing");
                return 0;
            }                       
        }

        k += 1;

        while(isspace(line->input[i])) i++;
        
        i += (j + 1);

            
    }

    return 1;
}

/*
Split input into 4 strings: command, name1, name2, name3
*/
int parse_command(ParsedLine* line) {
    int i = 0,j = 0;
    char tmp_str[11];
    line->valid = 0;

    /* First pass, check the command */
    while(i < 10) {
        tmp_str[j++] = line->parsed[i++];
        tmp_str[j] = '\0';
        
        if(check_command(tmp_str)) {
            line->valid = 1;
            break;
        }
    }
    /* Bad command? return the exception */
    if(!line->valid) {
        error(line,"Undefined command name");
        return 0;
    }
    /* Good command? copy it and continue */
    tmp_str[j] = '\0';
    strcpy(line->command, tmp_str);

    return 1;
}
/*
Parse the real argument, auxiliary function for parse_three()
*/
float check_numeric(ParsedLine* line, int* i) {
    char tmp_str[16] = {'\0'};
    int j = 0,dot = 0,negative = 0;
    float res;
    *i -= 1;

    if(*i == strlen(line->parsed)) {
        error(line,"Missing real value argument");
        return 0;
    };

    if(line->parsed[(*i)++] != ',') {
        error(line,"Missing comma");
        return 0;
    }

    if(*i == strlen(line->parsed)) {
        error(line,"Extraneous comma after aruments");
        return 0;           
    }
    
    if(line->parsed[*i] == '-') {
        negative = 1;
        *i += 1;
    }

    while(line->parsed[*i] != ',' && *i < strlen(line->parsed)) {
        if(j > 15) {
            error(line,"The scalar argument is too big");
            return 0;
        }

        if(line->parsed[*i] == '.') {
            if(dot) {
                error(line,"Too many dots");
                return 0;
            }

            dot = 1;
            tmp_str[j++] = line->parsed[(*i)++];
            continue;
        }

        if(!isdigit(line->parsed[*i])) {
            error(line,"Invalid real argument");
            return 0;       
        }
        
        tmp_str[j++] = line->parsed[(*i)++];
    }

    strcpy(line->seg[1],tmp_str);

    res = atof(tmp_str);

    if(negative) res *= -1;
    
    return res;

}

/*
Parse the line, in case that the command's type is: read_mat MAT_X,x1,x2....
*/
void parse_one(ParsedLine* line, Matrix* arr[]) {
    int i = strlen(line->command);
    int j = 0, k = 0, dot = 0, negative = 0;
    char tmp_str[16] = "";
    float values[16] = {0};

    if(i == strlen(line->parsed)) {
        error(line,"Missing arguments");
        return;
    }

    for(j = 0; j < 5; j++) {
        tmp_str[j] = line->parsed[i++]; 
    }

    tmp_str[j + 1] = '\0';

    if(!check_name(tmp_str)) {
        error(line,"Undefined matrix name");
        return;
    }

    strcpy(line->seg[0], tmp_str);
    
    memset(tmp_str, 0, sizeof(tmp_str));


    for(k = 0; k < 16; k++) {
        negative = 0;
        dot = 0;
        j = 0;

        if(i == strlen(line->parsed)) {
            if(k) break;

            error(line,"Missing arguments");
            return;
        }

        if(line->parsed[i++] != ',' && k < 15) {
            error(line,"Missing comma");
            return;
        }

        if(i == strlen(line->parsed)) {
            error(line,"Extraneous comma after aruments");
            return;           
        }
        
        if(line->parsed[i] == '-') {
            negative = 1;
            i += 1;
        }

        while(line->parsed[i] != ',' && i < strlen(line->parsed)) {
            if(j > 15) {
                error(line,"At least one of the arguments is too big");
                return;
            }
            if(line->parsed[i] == '.') {
                if(dot) {
                    error(line,"Too many dots");
                    return;
                }

                dot = 1;
                tmp_str[j++] = line->parsed[i++];
                continue;
            }

            if(!isdigit(line->parsed[i])) {
                error(line,"Invalid argument");
                return;       
            }

            if(line->parsed[i + 1] == ',' && line->parsed[i + 2] == ',') {
                error(line,"Multiple commas");
                return;                  
            }
            
            tmp_str[j++] = line->parsed[i++];
        }

        values[k] = atof(tmp_str);

        if(negative) values[k] *= -1;

        memset(tmp_str, 0, sizeof(tmp_str));
    }

    
    if(line->parsed[i] != '\0') {
        error(line,"Extraneous text after end of command");
        return;
    }

    read_mat(arr[line->seg[0][4] - 'A'], values);

    return;
}
/*
Parse the line, in case that the command's type is: print_mat MAT_X, i.e type=2
*/
void parse_two(ParsedLine* line, Matrix* arr[]) {
    int i = strlen(line->command);
    int j = 0;
    char tmp_str[5] = {'\0'};

    if(i == strlen(line->parsed)) {
        error(line,"Missing arguments");
        return;
    }

    for(j = 0; j < 5; j++) {
        tmp_str[j] = line->parsed[i++]; 
    }


    if(!check_name(tmp_str)) {
        error(line,"Undefined matrix name");
        return;
    }

    strcpy(line->seg[0], tmp_str);

    if(i != strlen(line->parsed)) {
        error(line,"Extraneous text after argument");
        return;       
    }

    print_mat(arr[line->seg[0][4] - 'A']);

    return;
}

/*
Parse the line, in case that the command's type is: mul_scalar MAT_X,x,MAT_Y, i.e type=3
*/
void parse_three(ParsedLine* line, Matrix* arr[]) {
    int i = strlen(line->command);
    int j = 0,k = 0;
    float res;
    char tmp_str[16] = "";

    if(i == strlen(line->parsed)) {
        error(line,"Missing arguments");
        return;
    }

    for(k = 0; k < NUM_SEG; k++) {
        if(k == 1) {
            res = check_numeric(line,&i);
            
            if(!line->valid) {
                error(line,"Invalid real argument");
                return;
            }


            continue;
        }

        if(k == 2) {
            if(line->parsed[i++] != ',') {
                error(line,"Missing comma");
                return;
            }
        }

        for(j = 0; j < 5; j++) {
            tmp_str[j] = line->parsed[i++]; 
        }
    
        tmp_str[j + 1] = '\0';

        if(!check_name(tmp_str)) {
            error(line,"Undefined matrix name");
            return;
        }
    
        strcpy(line->seg[k], tmp_str);
        
        memset(tmp_str, 0, sizeof(tmp_str));

        if(k == 0 && i == strlen(line->parsed)) {
            error(line,"Missin second argument");
            return;
        }

        if(line->parsed[i] != ',' && k < NUM_SEG - 2) {
            error(line,"Missing comma");
            return;
        }

        i += 1;
    }

    if(line->parsed[i - 1] != '\0') {
        error(line,"Extraneous end of command");
        return;
    } 

    mul_scalar(arr[line->seg[0][4] - 'A'], arr[line->seg[2][4] - 'A'], res);

    return;
}

/*
Parse the line, in case that the command's type is: trans_mat MAT_X,MAT_Y, i.e type=4
*/
void parse_four(ParsedLine* line, Matrix* arr[]) {
    int i = strlen(line->command);
    int j = 0,k = 0;
    char tmp_str[12] =  {'\0'};

    if(i == strlen(line->parsed)) {
        error(line,"Missing arguments");
        return;
    }

    for(k = 0; k < NUM_SEG - 1; k++) {
        for(j = 0; j < 5; j++) {
            tmp_str[j] = line->parsed[i++]; 
        }
    
        tmp_str[j + 1] = '\0';

        if(!check_name(tmp_str)) {
            error(line,"Undefined matrix name");
            return;
        }
    
        strcpy(line->seg[k], tmp_str);
        
        memset(tmp_str, 0, sizeof(tmp_str));

        if(k == 0 && i == strlen(line->parsed)) {
            error(line,"Missin second argument");
            return;
        }

        if(line->parsed[i] != ',' && k < NUM_SEG - 2) {
            error(line,"Missing comma");
            return;
        }

            i += 1;
    }

    if(line->parsed[i - 1] != '\0') {
        error(line,"Extraneous text after end of command");
        return;
    } 


    trans_mat(arr[line->seg[0][4] - 'A'],arr[line->seg[1][4] - 'A']);

    return;
}

/*
Parse the line, in case that the command's type is: command MAT_X,MAT_Y,MAT_Z, i.e type=5
*/
void parse_five(ParsedLine* line, Matrix* arr[]) {
    int i = strlen(line->command);
    int j = 0,k = 0;
    char tmp_str[11] = "";

    if(i == strlen(line->parsed)) {
        error(line,"Missing arguments");
        return;
    }

    for(k = 0; k < NUM_SEG; k++) {
        for(j = 0; j < 5; j++) {
            tmp_str[j] = line->parsed[i++]; 
        }
    
        tmp_str[j + 1] = '\0';

        if(!check_name(tmp_str)) {
            error(line,"Undefined matrix name");
            return;
        }
    
        strcpy(line->seg[k], tmp_str);
        
        memset(tmp_str, 0, sizeof(tmp_str));

        if(line->parsed[i] != ',' && k < NUM_SEG - 1) {
            error(line,"Missing comma");
            return;
        }

        i += 1;
    }

    if(line->parsed[i - 1] != '\0') {
        error(line,"Extraneous text after end of command");
        return;
    } 

    if(line->command[0] == 'a') {
        add_mat(arr[line->seg[0][4] - 'A'], arr[line->seg[1][4] - 'A'], arr[line->seg[2][4] - 'A']);
    } else if (line->command[1] == 's')
    {
        sub_mat(arr[line->seg[0][4] - 'A'], arr[line->seg[1][4] - 'A'], arr[line->seg[2][4] - 'A']);
    } else {
        mul_mat(arr[line->seg[0][4] - 'A'], arr[line->seg[1][4] - 'A'], arr[line->seg[2][4] - 'A']);
    }
    

    return;
}

int check_type(ParsedLine* line) {
    int i;
    
    for(i = 0; i < NUM_COMMANDS; i++) {
        if(!strcmp(line->command, commands[i])) {
            if(i < 4) {
                line->type = i + 1;
            } else {
                line-> type = 5;
            }

            break;
        }
    }

    return 0;
}


int main() {
    TypeParser types[] = {{&parse_one}, {&parse_two}, {&parse_three}, {&parse_four}, {&parse_five}};
    Matrix* MAT_A = (Matrix*)malloc(sizeof(Matrix));
    Matrix* MAT_B = (Matrix*)malloc(sizeof(Matrix));
    Matrix* MAT_C = (Matrix*)malloc(sizeof(Matrix));
    Matrix* MAT_D = (Matrix*)malloc(sizeof(Matrix));
    Matrix* MAT_E = (Matrix*)malloc(sizeof(Matrix));
    Matrix* MAT_F = (Matrix*)malloc(sizeof(Matrix));
    Matrix* arr[6];
    int i;
    char* input;
    size_t size;
    ssize_t len;
    ParsedLine* line;

    arr[0] = MAT_A;
    arr[1] = MAT_B;
    arr[2] = MAT_C;
    arr[3] = MAT_D;
    arr[4] = MAT_E;
    arr[5] = MAT_F;

    for(i = 0; i < 6; i++) {
        mul_scalar(arr[i], arr[i], 0);
    }

    line = (ParsedLine*)malloc(sizeof(ParsedLine));

    line->valid = 0;
    
    /*
    Allocate the segments | TODO: write a helper function
    */
    line->input = (char*)malloc(MAX_INPUT * sizeof(char));
    line->parsed = (char*)malloc(MAX_INPUT * sizeof(char));
    line->command = (char*)malloc(MAX_COMMAND * sizeof(char));
    line->error = (char*)malloc(MAX_ERROR * sizeof(char));
    line->seg = (char**)malloc(4 * sizeof(char*));

    for(i = 0; i < NUM_SEG; i++) {
        line->seg[i] = (char*)malloc(16 * sizeof(char));
    }

    while(1) {
        clear_line_mem(line);
        /*
        Make sure that readline() will allocate a buffer
        */
        input = NULL;
        size = 0;

        puts("\nEnter your line:\n");

        len = getline(&input, &size, stdin);

        if(len < 0) {
            puts("Couldn't read the input");
            free(input);
            continue;
        } else {
            strncpy(line->input, input, strlen(input) - 1);
            line->input[strlen(input) - 1] = '\0';
            free(input);
        }

        printf("\nYour input: %s\n\n", line->input);


        remove_spaces(line);

        if(!strcmp(line->parsed,"stop")) return 0;

        parse_command(line);

        if(line->parsed[strlen(line->command)] == ',') {
            error(line,"Illegal comma");
        }

        if(!line->valid) {
            printf("%s\n", line->error);
            continue;
        }

        check_type(line);

        if(line->type <= 2) line->args = 1;
        if(line->type > 2) line->args = 2;
        if(line->type == 5) line->args = 3;

        types[line->type - 1].parse(line, arr);

        
        if(!line->valid) {
            printf("%s\n", line->error);
            continue;
        }

        i = valid_space_modified(line);

        if(!line->valid) {
            printf("%s\n", line->error);
            continue;
        }
    }   

    return 0;    
}

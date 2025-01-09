#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/******************************************************************************
*utilies 
*******************************************************************************/




/******************************************************************************
* Function: HexCharToInt
*
* Description: Hex Char -> Int  
*******************************************************************************/
int HexCharToInt(char h)
{
  short res;

  switch (h) {
	case 'A': res = 10;	break;
	case 'B': res = 11; break;
	case 'C': res = 12;	break;
	case 'D': res = 13;	break;
	case 'E': res = 14;	break;
	case 'F': res = 15;	break;
	case 'a': res = 10;	break;
	case 'b': res = 11;	break;
	case 'c': res = 12;	break;
	case 'd': res = 13;	break;
	case 'e': res = 14;	break;
	case 'f': res = 15;	break;
	default:
		res = atoi(&h); // if char < 10 change it to int
		break;
	}
	return res;
}

/******************************************************************************
* Function: Hex_2_Int_2s_Comp
*
* Description: Signed Hex -> Int 
*******************************************************************************/
int Hex_2_Int_2s_Comp(char * h)

{
    int i;
    int res = 0;
    int len = strlen(h);
    int msc; /* most significant chrachter */
	#ifdef change
    	if (ind) printf("***** string %s\n",h);
	#endif
    for(i=0;i<len-1;i++)
    {
        res += HexCharToInt(h[len - 1 - i]) * (1<<(4*i));
    }

    msc = HexCharToInt(h[0]);

    if (msc < 8)
    {
        res += msc * (1<<(4*(len-1)));
    }
    else
    {
        msc = msc - 8;
        res += msc * (1<<(4*(len-1))) + -8*(1<<(4*(len-1)));
    }
	#ifdef change
		if (ind){
			printf("*** res is %d\n", res);
			ind = 0;
		}
    #endif

    return res;
}

/******************************************************************************
* Function: Int_2_Hex
*
* Description: signed integer -> hex char array of length 8
*******************************************************************************/

void Int_2_Hex(int dec_num, char hex_num[9])
{
  if (dec_num < 0) 
	dec_num = dec_num + 4294967296; 

  sprintf(hex_num, "%08X", dec_num); 
}


/******************************************************************************
* Function: modify_file_line
*
* Description: change line line_number in a file
*******************************************************************************/

void modify_file_line(const char *filename, int line_number, const char *new_content) {
 FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char buffer[1024];
    int current_line = 1;

    // Copy the original file to the temporary file, appending to the target line.
    while (fgets(buffer, sizeof(buffer), file)) {
        if (current_line == line_number) {
            // Remove the newline character from the existing line before appending.
            buffer[strcspn(buffer, "\n")] = '\0';
            fprintf(temp, "%s%s\n", buffer, new_content);  // Append new content.
        } else {
            fprintf(temp, "%s", buffer);  // Copy the original line.
        }
        current_line++;
    }

    // If the target line is beyond the end of the file, append new lines and the content.
    while (current_line <= line_number) {
        if (current_line == line_number) {
            fprintf(temp, "%s\n", new_content);
        } else {
            fprintf(temp, "\n");
        }
        current_line++;
    }

    fclose(file);
    fclose(temp);

    // Replace the original file with the temporary file.
    if (remove(filename) != 0 || rename("temp.txt", filename) != 0) {
        perror("Error replacing the original file");
    }
}
// Function to count the number of lines in a file
int count_lines(FILE *file) {
    if (file == NULL) {
        fprintf(stderr, "Invalid file pointer.\n");
        return -1; // Indicate an error
    }

    int line_count = 0;
    char ch;

    // Read each character and count newline characters
    while ((ch = getc(file)) != EOF) {
        if (ch == '\n') {
            line_count++;
        }
    }

    return line_count;
}

// Command* initialize_command()
// {
//         // Initialize current_instruction fields

//     Command* cmd = (Command *)malloc(sizeof(Command));
//     strcpy(cmd->inst, "NOP");
//     cmd->opcode = 0;
//     cmd->rd = 0;
//     cmd->rs = 0;
//     cmd->rt = 0;
//     cmd->rm = 0;
//     cmd->imm = 0;
//     cmd->state = 0;
//      if (cmd == NULL) {
//             perror("Memory allocation failed for instruction_array[i]");
//             exit(1);
//      }
//     return cmd; 
// }


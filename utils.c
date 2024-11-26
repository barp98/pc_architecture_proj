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


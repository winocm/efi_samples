#include <stdio.h>

_int16 var;

int CheckLostCode (int value)
{
  switch (value) {
  case 0:
    break;

  default:
    _asm {
      mov bx, 1
      mov var, bx
    }
    return 1;
  }
  
  _asm {
    mov bx, 0
    mov var, bx
  }
  return 0;
}


int main ()
{
  int     result;
  char    select;

  var     = 0xFF;
  result  = 0; 

  CheckLostCode (0);
  result += (var == 0) ? 0 : 1;

  CheckLostCode (1);
  result += (var == 1) ? 0 : 1;

  if (result != 0) {
    printf ("Warning: C2.dll is incorrect. Would you want to continue?(Y/N)");
    
    scanf ("%c", &select);
    if ((select == 'Y') || (select == 'y')) {
      return 0;
    }
    else {
      return 1;
    }
  }

  return 0;
}
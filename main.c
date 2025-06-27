#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <stdio_ext.h>
#include "pipeline.h"

#define MI char mi[256][17] = {{'\0'}}
#define MD int md[256] = {0}

// PROGRAMA PRINCIPAL
int main() {

  Sinais sinais;
  UF uf = {0};
  Inst inst;
  Decod decod;
  Stack stack;
  Reg reg = {0};
  ULA_Out ula_out = {0};
  MI;
  MD;
  int ciclo = 1;

  inicia_pilha(&stack);

  int op, nl, resul;
  reg.pc = 0;

  do {
    op = menu();
    if (op == 3 || op == 4)
    {
      clear();         
      refresh();
    }
    printf("\n");
    switch (op) {
    case 1:
      carregaMemInst(mi);
      break;
    case 2:
      carregarMemoriaDados(md);
      break;
    case 3:
      initscr();             
      cbreak();              
      noecho();              
      curs_set(0);           
      keypad(stdscr, TRUE); 
      printImemory(mi, &inst, &decod);
      printDmemory(md);
      getch();
      endwin();
      clear();         
      refresh();
      printf("\033[H\033[J");
      break;
    case 4:
      infoWin(&reg, &decod, &inst);
      clear();         
      refresh();
      printf("\033[H\033[J");
      break;
    case 5:
      salvarAssembly(mi);
      break;
    case 6:
      salvarMemDados(md);
      break;
   case 7:
      initscr();             
      cbreak();              
      noecho();              
      curs_set(0);           
      keypad(stdscr, TRUE); 
      executa_ciclo(mi,&inst,&decod,&reg,md,&sinais,&ula_out,&ciclo,&stack,&uf);
      endwin();
      clear();         
      refresh();
      printf("\033[H\033[J");
      break;
    case 8:
      step_back(&stack,&reg,md,&ciclo);
      break;
    case 9:
      printf("Voce saiu!!!\n");
      break;
    }
  } while(op != 9);
  return 0;
}

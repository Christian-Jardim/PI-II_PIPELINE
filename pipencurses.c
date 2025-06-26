#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <stdio_ext.h>

#define MI char mi[256][17] = {{'\0'}}
#define MD int md[256] = {0}

typedef struct sinais {
  int EscPC,
      EscIF_ID,
      RegDest,
      ULAOp,
      ULAFonte,
      DC,
      DI,
      EscMem,
      EscReg,
      MemParaReg;
} Sinais;

typedef struct unidaded_forwarding {
  int a,b;
} UF;

typedef struct inst {
  char instIF_ID[17],
       instID_EX[17],
       instEX_MEM[17],
       instMEM_WB[17];
} InstView;

typedef struct if_id {
  int pc;
  char inst[17];
} IF_ID;

typedef struct id_ex {
  int memreg,
      escreg,
      branch,
      jump,
      escmem,
      regdest,
      ulafonte,
      opula,
      opcode,
      a,
      b,
      imm,
      rs,
      rt,
      rd;
} ID_EX;

typedef struct ex_mem {
  int memreg,
      escreg,
      escmem,
      saidaula,
      b,
      rd;
} EX_MEM;

typedef struct mem_wb {
  int memreg,
      escreg,
      dadomem,
      saidaula,
      rd;
} MEM_WB;

typedef struct registradores {
  int pc,
      br[8];
  IF_ID if_id;
  ID_EX id_ex;
  EX_MEM ex_mem;
  MEM_WB mem_wb;
} Reg;

//STRUCTS e ENUMS
typedef enum tipo {
  Tipo_R=0,
  Tipo_I=1,
  Tipo_J=2,
  Tipo_OUTROS=3
} Type;

typedef struct instrucao {
  char opcode[5],
       rs[4],
       rt[4],
       rd[4],
       funct[4],
       imm[7],
       addr[8];
} Inst;

typedef struct decodificador {
  int opcode,
      rs,
      rt,
      rd,
      funct,
      imm,
      addr;
  Type tipo;
} Decod;

typedef struct ULA_Out {
  int resultado,
      flag_zero,
      overflow;
} ULA_Out;

typedef struct Nodo {
  int pc,
      br[8],
      md[256],
      ciclo,
      cont;
  IF_ID if_id;
  ID_EX id_ex;
  EX_MEM ex_mem;
  MEM_WB mem_wb;
  struct Nodo *prox;
} Nodo;

typedef struct pilha {
  Nodo *topo;
} Stack;

//ASSINATURA DAS FUNCOES

int menu();
void infoWin(Reg *reg, InstView *instView, Decod *decod, Inst *inst);
void printAssemblyNcurses(WINDOW *win, int linha, int index, char *bin, Decod *decod);
void inputJanelaArquivo(char *buffer, int maxlen);
void errorwin();
void printInstrucaoNcurses(WINDOW *win, int linha, int index, char *bin, Decod *decod);
void printImemory(char mi[256][17], Inst *inst, Decod *decod);
void printDmemory(int *md);
void infoPipeline(Reg *reg, int ciclo, UF *uf, WINDOW *win);

int carregaMemInst(char mi[256][17],int *pc,int *cont,int *ciclo);
void carregarMemoriaDados(int md[256]);

void printMemory(char mi[256][17], Inst *inst, Decod *decod);
void printmemory(int *md);
void printReg(Reg *reg);
void printInstrucao(Decod *decod);

void decodificarInstrucao(const char *bin, Inst *inst, Decod *decod);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);

void inicia_pilha(Stack *stack);
int step_back(Stack *stack,Reg *reg, int *md,int *ciclo,int *cont);
void empilha(Stack *stack,Reg *reg, int *md,int *ciclo,int *cont);
int limite_back(Stack *stack);

void ULA(int op1, int op2, int opULA, ULA_Out *ula_out);
void Forward(int rs, int rt,int rd_mem,int rd_wb,int opcode,int ulafonte, UF *uf);
int somador(int op1, int op2);
int comparador(int op1, int op2);

void salvarAssembly(char mi[256][17]);
void salvarMemDados(int *md);

void controle(int opcode, int funct, Sinais *sinais);

int executa_ciclo(char mi[256][17],Inst *inst,Decod *decod,Reg *reg,int *md,Sinais *sinais,ULA_Out *ula_out,int *ciclo,Stack *stack,int *cont,UF *uf, InstView *instView);

void escreve_br(int *reg, int dado, int EscReg);
void escreve_md(int *index, int dado, int EscMem);
void escreve_pc(int *pc, int op2, int EscPC);

//MUX
int MemReg(int op2, int op1, int MemParaReg);
int RegDest(int op2, int op1, int Reg_Dest);
int ULAFonteA(int a,int saidaula,int dado,int ULAFonte);
int ULAFonteB(int b,int imm,int saidaula,int dado,int ULAFonte);
int NOP(int opcode, int op_ant, int rt, int rd, int escreg, int escmem, int flag, int *reg, int *mem, int *escif, Decod *decod, int *cont);
int ESC_IF(int op_ant, int EscIF_ID, int comparacao);

// PROGRAMA PRINCIPAL
int main() {

  Sinais sinais;
  UF uf = {0};
  Inst inst;
  InstView instView;
  Decod decod;
  Stack stack;
  Reg reg = {0};
  ULA_Out ula_out = {0};
  MI;
  MD;
  int ciclo = 1, cont = 4;

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
      carregaMemInst(mi,&reg.pc,&cont,&ciclo);
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
      endwin();
      clear();         
      refresh();
      printf("\033[H\033[J");
      break;
    case 4:
      infoWin(&reg, &instView, &decod, &inst);
      clear();         
      refresh();
			printf("\033[H\033[J");
      break;
    case 5:
      //printMemory(mi, &inst, &decod);
      //printmemory(md);
      //printReg(&reg);
      //printf("\n\nPC: %d\n", reg.pc);
      break;
    case 6:
      salvarAssembly(mi);
      break;
    case 7:
      salvarMemDados(md);
      break;
    case 8:
      break;
    case 9:
      initscr();             
      cbreak();              
      noecho();              
      curs_set(0);           
      keypad(stdscr, TRUE); 
      executa_ciclo(mi,&inst,&decod,&reg,md,&sinais,&ula_out,&ciclo,&stack,&cont,&uf,&instView);
      endwin();
      clear();         
      refresh();
      printf("\033[H\033[J");
      break;
    case 10:
      step_back(&stack,&reg,md,&ciclo,&cont);
      break;
    case 11:
      printf("Voce saiu!!!\n");
      break;
    }
  } while(op != 11);
  return 0;
}

//FUNCOES IMPLEMENTADAS

// carrega memoria de instrucoes a partir de um "arquivo.mem"
int carregaMemInst(char mi[256][17],int *pc, int *cont,int *ciclo) {
  char arquivo[20],extensao[5];
  int tam;

  inputJanelaArquivo(arquivo, sizeof(arquivo));
  printf("\033[H\033[J"); // Limpa tela no modo texto (ANSI escape)

  tam = strlen(arquivo);
  strncpy(extensao,arquivo + tam - 4,4);
  extensao[4] = '\0';

  if(strcmp(extensao, ".mem") != 0) {
    errorwin();
    clear();         
    refresh();
  } else {
    *pc = 0;
    *cont = 4;
    *ciclo = 1;
    FILE *arq = fopen (arquivo, "r");
    if (!arq)
    {
      perror ("Erro ao abrir arquivo") ;
      exit (1) ;
    }
    int i = 0;
    char linha[20]; // Buffer para leitura
    while (i < 256 && fgets(linha, sizeof(linha), arq)) {
      // Remover quebras de linha e caracteres extras
      linha[strcspn(linha, "\r\n")] = '\0';

      // Ignorar linhas vazias
      if (strlen(linha) == 0) {
        continue;
      }

      strncpy(mi[i], linha, 16); // Copia ate 16 caracteres
      mi[i][16] = '\0'; // Garante terminacao de string
      i++; // Avanca corretamente para a proxima posicao
    }
    fclose(arq);
    return 1;
  }
}

//carrega memoria de dados a partir de um "arquivo.dat"
void carregarMemoriaDados(int md[256]) {
  char arquivo[20],extensao[5];
  int tam;

  inputJanelaArquivo(arquivo, sizeof(arquivo));
  printf("\033[H\033[J"); // Limpa tela no modo texto (ANSI escape)

  tam = strlen(arquivo);
  strncpy(extensao,arquivo + tam - 4,4);
  extensao[4] = '\0';

  if(strcmp(extensao, ".dat") != 0) {
    errorwin();
    clear();         
    refresh();
  } else {
    FILE *arq = fopen(arquivo, "r");
    if (!arq) {
      perror ("Erro ao abrir arquivo") ;
      exit (1) ;
    }
    int i = 0;
    while (fscanf(arq, "%d", &md[i]) != EOF) {
      i++;
    }
  }
}

// imprime memoria de instrucoes
void printMemory(char mi[256][17], Inst *inst, Decod *decod) {
  printf("\n############## MEMORIA DE INSTRUCOES ##############\n");
  for (int i = 0; i < 256; i++)
  {
    printf("\nInstrucao: %s\n", mi[i]);
    printf("[%d].  ", i);
    decodificarInstrucao(mi[i], inst, decod);
    printInstrucao(decod);
    printf("\n");
  }
}

// imprime memoria de dados
void printmemory(int *md) {
  printf("\n############## MEMORIA DE DADOS ##############\n\n");
  for(int i=0; i<256; i++) {
    printf("[%d]. %d   ", i, md[i]);
    if (i % 8 == 7)
    {
      printf("\n");
    }
  }
}

// imprime banco de registradores
void printReg(Reg *reg) {
  for(int i=0; i<8; i++) {
    printf("\nREGISTRADOR [%d] - %d", i, reg->br[i]);
  }
}

// decodifica a instrucao e armazena os valores na struct do tipo Deco ja no formato int
void decodificarInstrucao(const char *bin, Inst *inst, Decod *decod) {
  copiarBits(bin, inst->opcode, 0, 4);    // Copia os 4 bits do opcode (4 bits)
  decod->opcode = binarioParaDecimal(inst->opcode, 0);
  copiarBits(bin, inst->rs, 4, 3);        // Copia os 3 bits do rs
  decod->rs = binarioParaDecimal(inst->rs, 0);
  copiarBits(bin, inst->rt, 7, 3);        // Copia os 3 bits do rt
  decod->rt = binarioParaDecimal(inst->rt, 0);
  copiarBits(bin, inst->rd, 10, 3);       // Copia os 3 bits do rd
  decod->rd = binarioParaDecimal(inst->rd, 0);
  copiarBits(bin, inst->funct, 13, 3);    // Copia os 3 bits do funct
  decod->funct = binarioParaDecimal(inst->funct, 0);
  copiarBits(bin, inst->imm, 10, 6);      // Copia os 6 bits do imm
  decod->imm = binarioParaDecimal(inst->imm, 1);
  copiarBits(bin, inst->addr, 9, 7);     // Copia os 7 bits do addr
  decod->addr = binarioParaDecimal(inst->addr, 0);
}

// copia os bits da instrucao para cada campo da struct instrucao
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho) {
  strncpy(destino, instrucao + inicio, tamanho); // Copia os bits desejados
  destino[tamanho] = '\0';  // Adiciona o terminador de string
}

// converte de binario para decimal
int binarioParaDecimal(const char *bin, int sinal) {
  int valor = (int)strtol(bin, NULL, 2);
  int bits = strlen(bin);

  if (sinal && bits > 0 && (valor & (1 << (bits - 1)))) {
    valor = valor - (1 << bits);
  }
  return valor;
}

// funcao para imprimir a instrucao
void printInstrucao(Decod *decod) {
  switch (decod->opcode) {
  case 0: // Tipo R (add, sub, and, or)
    switch (decod->funct) {
    case 0:
      printf("add $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
      break;
    case 2:
      printf("sub $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
      break;
    case 4:
      printf("and $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
      break;
    case 5:
      printf("or $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
      break;
    }
    break;
  case 4: // addi
    printf("addi $%d, $%d, %d", decod->rt, decod->rs, decod->imm);
    break;
  case 11: // lw
    printf("lw $%d, %d($%d)", decod->rt, decod->imm, decod->rs);
    break;
  case 15: // sw
    printf("sw $%d, %d($%d)", decod->rt, decod->imm, decod->rs);
    break;
  case 8: // beq
    printf("beq $%d, $%d, %d", decod->rs, decod->rt, decod->imm);
    break;
  case 2: // j
    printf("j %d", decod->addr);
    break;
  }
}

// funcao para conversao das instrucoes para assembly e salvar "arquivo.asm"
void salvarAssembly(char mi[256][17]) {
  char arquivo[20];

  inputJanelaArquivo(arquivo, sizeof(arquivo));
	printf("\033[H\033[J"); // Limpa a tela no terminal padrão (modo texto)

  FILE *arq = fopen(arquivo, "w");
  if (!arq) {
    perror("Erro ao criar arquivo");
    return;
  }

  for (int i = 0; i < 256; i++) {
    if (mi[i][0] == '\0') continue; // Ignora posicoes vazias

    struct instrucao inst;
    Decod decod;
    decodificarInstrucao(mi[i], &inst, &decod);

    // Converte para assembly e escreve no arquivo
    switch (decod.opcode) {
    case 0: // Tipo R (add, sub, and, or)
      switch (decod.funct) {
      case 0:
        fprintf(arq, "add $%d, $%d, $%d\n", decod.rd, decod.rs, decod.rt);
        break;
      case 2:
        fprintf(arq, "sub $%d, $%d, $%d\n", decod.rd, decod.rs, decod.rt);
        break;
      case 4:
        fprintf(arq, "and $%d, $%d, $%d\n", decod.rd, decod.rs, decod.rt);
        break;
      case 5:
        fprintf(arq, "or $%d, $%d, $%d\n", decod.rd, decod.rs, decod.rt);
        break;
      }
      break;
    case 4: // addi
      fprintf(arq, "addi $%d, $%d, %d\n", decod.rt, decod.rs, decod.imm);
      break;
    case 11: // lw
      fprintf(arq, "lw $%d, %d($%d)\n", decod.rt, decod.imm, decod.rs);
      break;
    case 15: // sw
      fprintf(arq, "sw $%d, %d($%d)\n", decod.rt, decod.imm, decod.rs);
      break;
    case 8: // beq
      fprintf(arq, "beq $%d, $%d, %d\n", decod.rs, decod.rt, decod.imm);
      break;
    case 2: // j
      fprintf(arq, "j %d\n", decod.addr);
      break;
    }
  }
  fclose(arq);
  printf("Arquivo %s salvo com sucesso!\n", arquivo);
}

// salva memoria de dados em um "arquivo.dat"
void salvarMemDados(int *md) {
  FILE *arquivo;
  char nomeArquivo[20];

  inputJanelaArquivo(nomeArquivo, sizeof(nomeArquivo));
	printf("\033[H\033[J"); // Limpa a tela no terminal padrão (modo texto)

  if ((arquivo = fopen(nomeArquivo, "w")) == NULL)
  {
    printf("Erro ao gerar o arquivo!");
  }
  for (int i = 0; i < 256; i++)
  {
    fprintf(arquivo, "%d\n", md[i]);
  }
  fclose(arquivo);
}

// inicia pilha apontando para NULL
void inicia_pilha(Stack *stack) {
  stack->topo=NULL;
}

//salva registradores, memoria de dados e pc na pilha
void empilha(Stack *stack,Reg *reg,int *md,int *ciclo,int *cont) {
  Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));

  for (int i = 0; i < 8; i++) {
    nNodo->br[i] = reg->br[i];
  }
  for (int i = 0; i < 256; i++) {
    nNodo->md[i] = md[i];
  }

  nNodo->pc = reg->pc;

  nNodo->if_id.pc = reg->if_id.pc;
  strcpy(nNodo->if_id.inst,reg->if_id.inst);

  nNodo->id_ex.memreg = reg->id_ex.memreg;
  nNodo->id_ex.escreg = reg->id_ex.escreg;
  nNodo->id_ex.branch = reg->id_ex.branch;
  nNodo->id_ex.jump = reg->id_ex.jump;
  nNodo->id_ex.escmem = reg->id_ex.escmem;
  nNodo->id_ex.regdest = reg->id_ex.regdest;
  nNodo->id_ex.ulafonte = reg->id_ex.ulafonte;
  nNodo->id_ex.opula = reg->id_ex.opula;
  nNodo->id_ex.a = reg->id_ex.a;
  nNodo->id_ex.b = reg->id_ex.b;
  nNodo->id_ex.opcode = reg->id_ex.opcode;
  nNodo->id_ex.imm = reg->id_ex.imm;
  nNodo->id_ex.rs = reg->id_ex.rs;
  nNodo->id_ex.rt = reg->id_ex.rt;
  nNodo->id_ex.rd = reg->id_ex.rd;

  nNodo->ex_mem.memreg = reg->ex_mem.memreg;
  nNodo->ex_mem.escreg = reg->ex_mem.escreg;
  nNodo->ex_mem.escmem = reg->ex_mem.escmem;
  nNodo->ex_mem.saidaula = reg->ex_mem.saidaula;
  nNodo->ex_mem.b = reg->ex_mem.b;
  nNodo->ex_mem.rd = reg->ex_mem.rd;

  nNodo->mem_wb.memreg = reg->mem_wb.memreg;
  nNodo->mem_wb.escreg = reg->mem_wb.escreg;
  nNodo->mem_wb.dadomem = reg->mem_wb.dadomem;
  nNodo->mem_wb.saidaula = reg->mem_wb.saidaula;
  nNodo->mem_wb.rd = reg->mem_wb.rd;

  nNodo->ciclo = *ciclo;
  nNodo->cont = *cont;

  nNodo->prox = stack->topo;
  stack->topo = nNodo;
}

// funcao de execucao do step back
int step_back(Stack *stack, Reg *reg, int *md,int *ciclo,int *cont) {
  Nodo *remover = stack->topo;

  for (int i = 0; i < 8; i++) {
    reg->br[i] = remover->br[i];
  }

  for (int i = 0; i < 256; i++) {
    md[i] = remover->md[i];
  }

  reg->pc = remover->pc;

  reg->if_id.pc = remover->if_id.pc;
  strcpy(reg->if_id.inst,remover->if_id.inst);

  reg->id_ex.memreg = remover->id_ex.memreg;
  reg->id_ex.escreg = remover->id_ex.escreg;
  reg->id_ex.branch = remover->id_ex.branch;
  reg->id_ex.jump = remover->id_ex.jump;
  reg->id_ex.escmem = remover->id_ex.escmem;
  reg->id_ex.regdest = remover->id_ex.regdest;
  reg->id_ex.ulafonte = remover->id_ex.ulafonte;
  reg->id_ex.opula = remover->id_ex.opula;
  reg->id_ex.a = remover->id_ex.a;
  reg->id_ex.b = remover->id_ex.b;
  reg->id_ex.opcode = remover->id_ex.opcode;
  reg->id_ex.imm = remover->id_ex.imm;
  reg->id_ex.rs = remover->id_ex.rs;
  reg->id_ex.rt = remover->id_ex.rt;
  reg->id_ex.rd = remover->id_ex.rd;

  reg->ex_mem.memreg = remover->ex_mem.memreg;
  reg->ex_mem.escreg = remover->ex_mem.escreg;
  reg->ex_mem.escmem = remover->ex_mem.escmem;
  reg->ex_mem.saidaula = remover->ex_mem.saidaula;
  reg->ex_mem.b = remover->ex_mem.b;
  reg->ex_mem.rd = remover->ex_mem.rd;

  reg->mem_wb.memreg = remover->mem_wb.memreg;
  reg->mem_wb.escreg = remover->mem_wb.escreg;
  reg->mem_wb.dadomem = remover->mem_wb.dadomem;
  reg->mem_wb.saidaula = remover->mem_wb.saidaula;
  reg->mem_wb.rd = remover->mem_wb.rd;

  *ciclo = remover->ciclo;
  *cont = remover->cont;

  stack->topo = remover->prox;
  free(remover);
  return 0;
}

// somador
int somador(int op1, int op2) {
  return op1 + op2;
}

void Forward(int rs, int rt, int rd_mem, int rd_wb, int opcode, int ulafonte, UF *uf) {

  if (opcode == 4 || opcode == 11 || opcode == 15) {
    uf->b = ulafonte;

    if (rs != 0) {
      if (rs == rd_mem) {
        uf->a = 1;
        printf("\nHazard de dados detectado para: rs! \n");
      } else {

        if (rs == rd_wb) {
          uf->a = 2;
          printf("\nHazard de dados detectado para: rs! \n");
        } else {
          uf->a = 0;
        }
      }

    } else {
      uf->a = 0;
    }
  } else {
    if (rt != 0) {
      if (rt == rd_mem) {
        printf("\nHazard de dados detectado para: rt! \n");
        uf->b = 1;
      } else {
        if (rt == rd_wb) {
          printf("\nHazard de dados detectado para: rt! \n");
          uf->b = 2;
        } else {
          uf->b = ulafonte;
        }
      }
    } else {
      uf->b = ulafonte;
    }

    if (rs != 0) {
      if (rs == rd_mem) {
        printf("\nHazard de dados detectado para: rs! \n");
        uf->a = 1;
      } else {
        if (rs == rd_wb) {
          printf("\nHazard de dados detectado para: rs! \n");
          uf->a = 2;
        } else {
          uf->a = 0;
        }
      }
    } else {
      uf->a = 0;
    }
  }
}

// limite do step back, termina desempilhamento na primeira instrucao executada
int limite_back(Stack *stack) {
  if(stack->topo==NULL) {
    printf("\nVoce voltou ao inicio!");
    return 1;
  }
}

void controle(int opcode, int funct, Sinais *sinais) {

  memset(sinais, 0, sizeof(Sinais));

  switch(opcode) {
  case 0:
    sinais->EscPC = 1;
    sinais->EscIF_ID = 1;
    sinais->RegDest = 1;
    sinais->ULAOp = funct;
    sinais->ULAFonte = 0;
    sinais->DC = 0;
    sinais->DI = 0;
    sinais->EscMem = 0;
    sinais->EscReg = 1;
    sinais->MemParaReg = 1;
    break;

  case 2:
    sinais->EscPC = 1;
    sinais->EscIF_ID = 1;
    sinais->RegDest = 0;
    sinais->ULAOp = 0;
    sinais->ULAFonte = 0;
    sinais->DC = 0;
    sinais->DI = 1;
    sinais->EscMem = 0;
    sinais->EscReg = 0;
    sinais->MemParaReg = 0;
    break;

  case 4:
    sinais->EscPC = 1;
    sinais->EscIF_ID = 1;
    sinais->RegDest = 0;
    sinais->ULAOp = 0;
    sinais->ULAFonte = 3;
    sinais->DC = 0;
    sinais->DI = 0;
    sinais->EscMem = 0;
    sinais->EscReg = 1;
    sinais->MemParaReg = 1;
    break;

  case 8:
    sinais->EscPC = 1;
    sinais->EscIF_ID = 1;
    sinais->RegDest = 0;
    sinais->ULAOp = 2;
    sinais->ULAFonte = 0;
    sinais->DC = 1;
    sinais->DI = 0;
    sinais->EscMem = 0;
    sinais->EscReg = 0;
    sinais->MemParaReg = 0;
    break;

  case 11:
    sinais->EscPC = 1;
    sinais->EscIF_ID = 1;
    sinais->RegDest = 0;
    sinais->ULAOp = 0;
    sinais->ULAFonte = 3;
    sinais->DC = 0;
    sinais->DI = 0;
    sinais->EscMem = 0;
    sinais->EscReg = 1;
    sinais->MemParaReg = 0;
    break;

  case 15:
    sinais->EscPC = 1;
    sinais->EscIF_ID = 1;
    sinais->RegDest = 0;
    sinais->ULAOp = 0;
    sinais->ULAFonte = 3;
    sinais->DC = 0;
    sinais->DI = 0;
    sinais->EscMem = 1;
    sinais->EscReg = 0;
    sinais->MemParaReg = 0;
    break;
  }
}

int MemReg(int op2, int op1, int MemParaReg) {
  switch (MemParaReg) {
  case 0:
    return op1;
  case 1:
    return op2;
  }
}

int RegDest(int op2, int op1, int Reg_Dest) {
  switch (Reg_Dest) {
  case 0:
    return op1;
  case 1:
    return op2;
  }
}

int ULAFonteA(int a,int saidaula,int dado,int ULAFonte) {
  switch (ULAFonte) {
  case 0:
    return a;
    break;
  case 1:
    return saidaula;
    break;
  case 2:
    return dado;
    break;
  }
}

int ULAFonteB(int b,int saidaula,int dado,int imm,int ULAFonte) {
  switch (ULAFonte) {
  case 0:
    return b;
    break;
  case 1:
    return saidaula;
  case 2:
    return dado;
    break;
  case 3:
    return imm;
    break;
  }
}

//NOP(decod->opcode, reg->id_ex.opcode, decod->rt, decod->rs, sinais->EscReg, sinais->EscMem,ula_out->flag_zero,&reg->id_ex.escreg,&reg->id_ex.escmem,&sinais->EscIF_ID);

int NOP(int opcode, int op_ant, int rt, int rd, int escreg, int escmem, int flag, int *reg, int *mem, int *escif, Decod *decod, int *cont) {
  if(op_ant == 8) {
    if(flag) {
      printf("\nDESVIO FEITO!\nInstrucao [");
      printInstrucao(decod);
      printf("] foi tranformada em NOP!\n");
      *reg = 0;
      *mem = 0;
      *escif = 0;
      (*cont)--;
    } else {
      if(opcode == 0) {
        if(rd == 0) {
          *reg = 0;
          *mem = escmem;
          *escif = 1;
        } else {
          *reg = escreg;
          *mem = escmem;
          *escif = 1;
        }
      } else if(opcode == 4) {
        if(rt == 0) {
          *reg = 0;
          *mem = escmem;
          *escif = 1;
        } else {
          *reg = escreg;
          *mem = escmem;
          *escif = 1;
        }
      } else {
        *reg = escreg;
        *mem = escmem;
        *escif = 1;
      }
    }
  } else {
    *reg = escreg;
    *mem = escmem;
    *escif = 1;
  }
}

// Funcao ULA
void ULA(int op1, int op2, int opULA, ULA_Out *ula_out) {
  ula_out->flag_zero = 0;

  switch(opULA) {
  case 0:
    ula_out->resultado = op1 + op2;
    if(ula_out->resultado == 0) {
      ula_out->flag_zero = 1;
    }
    if ((op1 > 0 && op2 > 0 && ula_out->resultado < 0) || (op1 < 0 && op2 < 0 && ula_out->resultado > 0)) {
      ula_out->overflow = 1;
      printf("OVERFLOW - ADD: %d + %d = %d\n", op1, op2, ula_out->resultado);
    }
    break;
  case 2:
    ula_out->resultado = op1 - op2;
    if(ula_out->resultado == 0) {
      ula_out->flag_zero = 1;
    }
    if ((op1 > 0 && op2 < 0 && ula_out->resultado < 0) || (op1 < 0 && op2 > 0 && ula_out->resultado > 0)) {
      ula_out->overflow = 1;
      printf("OVERFLOW - SUB: %d - %d = %d\n", op1, op2, ula_out->resultado);
    }
    break;
  case 4:
    ula_out->resultado = op1 & op2;
    break;
  case 5:
    ula_out->resultado = op1 | op2;
    break;
  }
}

int executa_ciclo(char mi[256][17],Inst *inst,Decod *decod,Reg *reg,int *md,Sinais *sinais,ULA_Out *ula_out,int *ciclo,Stack *stack,int *cont, UF *uf, InstView *instView) {

  if(*cont == 0) {
    printf("\n\nPROGRAMA ATUAL FINALIZADO!\n\n");
    return 0;
  }

  int dado,entradaA,entradaB,hazard_controle,uf_out;

  printf("\nEXECUTADO O CICLO %d\n",*ciclo);

  empilha(stack,reg,md,ciclo,cont);

  // escreve no banco de registradores

  strcpy(instView->instMEM_WB, instView->instEX_MEM); ///////////////////////////////////

  dado = MemReg(reg->mem_wb.saidaula, reg->mem_wb.dadomem, reg->mem_wb.memreg);
  if (reg->mem_wb.escreg) {
    reg->br[reg->mem_wb.rd] = dado;
    printf("[WB] Registrador[%d] = %d\n", reg->mem_wb.rd, dado);
  }


  // acessa memoria
  if (reg->ex_mem.escmem) {
    md[reg->ex_mem.saidaula] = reg->ex_mem.b;
    printf("[MEM] Memoria[%d] = %d\n", reg->ex_mem.saidaula, reg->ex_mem.b);
  }

  Forward(reg->id_ex.rs,reg->id_ex.rt,reg->ex_mem.rd,reg->mem_wb.rd,reg->id_ex.opcode,reg->id_ex.ulafonte,uf);

  reg->mem_wb.memreg = reg->ex_mem.memreg;
  reg->mem_wb.escreg = reg->ex_mem.escreg;
  reg->mem_wb.dadomem = md[reg->ex_mem.saidaula];
  reg->mem_wb.saidaula = reg->ex_mem.saidaula;
  reg->mem_wb.rd = reg->ex_mem.rd;

  if(sinais->DC && ula_out->flag_zero) {
    reg->pc = reg->if_id.pc + decod->imm;
    printf("[EX] Branch:\n\nPC = %d + %d = %d\n", reg->if_id.pc, decod->imm, reg->pc);
  }

  if(sinais->DI) {
    reg->pc = decod->addr;
    printf("[ID] Jump:\n\nPC = %d\n", reg->pc);
  }

  // executa

  strcpy(instView->instEX_MEM, instView->instID_EX); ///////////////////////////////////

  entradaA = ULAFonteA(reg->id_ex.a,reg->ex_mem.saidaula,dado,uf->a);
  entradaB = ULAFonteB(reg->id_ex.b,reg->ex_mem.saidaula,dado,reg->id_ex.imm,uf->b);

  ULA(entradaA, entradaB, reg->id_ex.opula, ula_out);

  // tratamento de hazard de controle para beq
  if (reg->id_ex.branch && ula_out->flag_zero){
    printf("\n[HAZARD DE CONTROLE] - BEQ\n");
    reg->pc = reg->if_id.pc + reg->id_ex.imm;
    sinais->EscIF_ID = 0;
  }

  // tratamento de hazard de controle para jump
  if (reg->id_ex.jump) {
    printf("\n[HAZARD DE CONTROLE] - JUMP\n");
    reg->pc = reg->id_ex.imm;
    sinais->EscIF_ID = 0;
  }

  reg->ex_mem.escreg = reg->id_ex.escreg;
  reg->ex_mem.escmem = reg->id_ex.escmem;

  reg->ex_mem.memreg = reg->id_ex.memreg;
  reg->ex_mem.saidaula = ula_out->resultado;
  reg->ex_mem.b = reg->id_ex.b;
  reg->ex_mem.rd = RegDest(reg->id_ex.rd, reg->id_ex.rt, reg->id_ex.regdest);

  // decodifica
  decodificarInstrucao(reg->if_id.inst,inst,decod);

  strcpy(instView->instID_EX, instView->instIF_ID); ///////////////////////////////////

  if(decod->opcode == 0 && decod->rd == 0) {
    (*cont)--;
  }

  controle(decod->opcode, decod->funct, sinais);

  if(sinais->EscPC == 1) {
    reg->pc = reg->if_id.pc;
  }

  NOP(decod->opcode, reg->id_ex.opcode, decod->rt, decod->rd, sinais->EscReg, sinais->EscMem,ula_out->flag_zero,&reg->id_ex.escreg,&reg->id_ex.escmem,&sinais->EscIF_ID, decod, cont);

  /*if(reg->id_ex.opcode == 8 && ula_out->flag_zero == 1) {
      reg->id_ex.escreg = 0;
      reg->id_ex.escmem = 0;
  }*/

  reg->id_ex.opcode = decod->opcode;
  reg->id_ex.memreg = sinais->MemParaReg;
  reg->id_ex.branch = sinais->DC;
  reg->id_ex.jump = sinais->DI;
  reg->id_ex.regdest = sinais->RegDest;
  reg->id_ex.ulafonte = sinais->ULAFonte;

  reg->id_ex.a = reg->br[decod->rs];
  reg->id_ex.b = reg->br[decod->rt];
  reg->id_ex.imm = decod->imm;
  reg->id_ex.rs = decod->rs;
  reg->id_ex.rt = decod->rt;
  reg->id_ex.rd = decod->rd;

  // busca

  if(reg->pc <= 256) {
    if(sinais->EscIF_ID == 1)
      strcpy(reg->if_id.inst, mi[reg->pc]);
    else {
      strcpy(reg->if_id.inst, "0000000000000000");
      printf("\nDESVIO FEITO!\nInstrucao [%s] foi tranformada em NOP!\n",mi[reg->pc]);
    }
  } else {
    strcpy(reg->if_id.inst, "0000000000000000");
  }
  reg->if_id.pc = somador(reg->pc, 1);
  printf("**** [IF] ****\nPC = %d\nInstrucao = %s\n", reg->pc, reg->if_id.inst);

  strcpy(instView->instIF_ID, reg->if_id.inst); ///////////////////////////////////

  printf("\n**** [ID] ****\nInstrucao: ");
  printInstrucao(decod);
  printf("\n");
  printf("\n**** [EX] ****\n");
  printf("\nOperacao feita: %d ",entradaA);
  switch(reg->id_ex.opula) {
  case 0:
    printf("+");
    break;
  case 2:
    printf("-");
    break;
  case 4:
    printf("and");
    break;
  case 5:
    printf("or");
  }
  printf(" %d = %d\n",entradaB,ula_out->resultado);
  if(*cont == 0) {
    printf("\n\nFIM DO PROGRAMA ATUAL!\n\n");
  }

    reg->id_ex.opula = sinais->ULAOp;

  (*ciclo)++;

  WINDOW *pipewin = newwin(30, 76, 15, 0);  // altura, largura, y, x
  infoPipeline(reg, *ciclo - 1, uf, pipewin);
  delwin(pipewin);
}

//funções ncusrses
//MENU
int menu() {
	initscr();              // Inicia ncurses
	noecho();               // Naoo exibe teclas digitadas
	cbreak();               // Leitura imediata de teclas
	curs_set(0);            // Oculta o cursor
	keypad(stdscr, TRUE);   // Habilita teclas especiais como setas

	const char *opcoes[] = {
		"1 - Carregar memoria de instrucoes",
		"2 - Carregar memoria de dados",
		"3 - Imprimir memorias",
		"4 - Imprimir banco de registradores",
		"5 - Imprimir todo o simulador",
		"6 - Salvar .asm",
		"7 - Salvar .dat",
		"8 - Executar programa",
		"9 - Executar instrucao",
		"10 - Volta uma instrucao",
		"11 - Sair"
	};
	int n_opcoes = sizeof(opcoes) / sizeof(opcoes[0]);
	int escolha = 0;

	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);

	WINDOW *menuwin = newwin(n_opcoes + 4, 50, 0, 0);
	box(menuwin, 0, 0);
	keypad(menuwin, TRUE);

	int ch;
	while (1) {
		// Titulo
		mvwprintw(menuwin, 1, 18, "*** MENU ***");

		// Imprimir opcoeses
		for (int i = 0; i < n_opcoes; i++) {
			if (i == escolha)
				wattron(menuwin, A_REVERSE); // Destaque
			mvwprintw(menuwin, i + 2, 2, "%s", opcoes[i]);
			wattroff(menuwin, A_REVERSE);
		}

		wrefresh(menuwin);

		ch = wgetch(menuwin);
		switch (ch) {
		case KEY_UP:
			escolha = (escolha - 1 + n_opcoes) % n_opcoes;
			break;
		case KEY_DOWN:
			escolha = (escolha + 1) % n_opcoes;
			break;
		case 10: // Enter
			delwin(menuwin);
			endwin();
			return escolha + 1; // retorna a opcao escolhida (1 a 11)
		}
	}
  delwin(menuwin);
  endwin();
  printf("\033[H\033[J");
}

//cria janela para inputar texto
void inputJanelaArquivo(char *buffer, int maxlen) {
	initscr();              // Inicia ncurses
  int altura = 7, largura = 50;
  int yMax, xMax;
  getmaxyx(stdscr, yMax, xMax);

  WINDOW *inputwin = newwin(altura, largura, 16, 0);
  box(inputwin, 0, 0);

  mvwprintw(inputwin, 1, 2, "Digite o nome do arquivo de instrucoes:");
  mvwprintw(inputwin, 3, 2, ">>> ");
  echo(); // Mostra o que o usuário digita

  wrefresh(inputwin);

  // Captura até maxlen-1 caracteres
  wgetnstr(inputwin, buffer, maxlen - 1);

  noecho(); // Volta ao modo sem eco
  werase(inputwin);
  wrefresh(inputwin);
  delwin(inputwin);
  endwin();
}

//janela para erro de extensão
void errorwin() {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  WINDOW *errorwin = newwin(7, 50, 16, 0);
  box(errorwin, 0, 0);
  mvwprintw(errorwin, 2, 16.5, "Erro de extensao!");
  mvwprintw(errorwin, 3, 11.5, "Carregar arquivo novamento!");
  mvwprintw(errorwin, 4, 11, "Pressione qualquer tecla ...");
  wrefresh(errorwin);
  wgetch(errorwin);
  delwin(errorwin);
  endwin();
}

//janela de registradores e informacoes
void infoWin(Reg *reg, InstView *instView, Decod *decod, Inst *inst) {
	initscr();
	curs_set(0);
	
  int altura = 21;
  int largura = 34;
  int y = 1;  // Todas começam na mesma linha

  WINDOW *infowin0 = newwin(altura, 21, y, 0);      // coluna 1
  WINDOW *infowin1 = newwin(altura, largura, y, 21);     // coluna 2
  WINDOW *infowin2 = newwin(altura, largura, y, 55);     // coluna 3
  WINDOW *infowin3 = newwin(altura, largura, y, 89);    // coluna 4
  WINDOW *infowin4 = newwin(altura, largura, y, 123);    // coluna 5

	// REGISTRADORES
	int l0 = 1;
	box(infowin0, 0, 0);
	mvwprintw(infowin0, l0++, 1, ">> REGISTRADORES <<");
  l0++;
	mvwprintw(infowin0, l0++, 2, "PC -------- %d", reg->pc);
	for (int i = 0; i < 8; i++) {
		mvwprintw(infowin0, l0++, 2, "REG[%d] ---- %d", i, reg->br[i]);
	}

	// IF -> ID
	int l1 = 1;
	box(infowin1, 0, 0);
	mvwprintw(infowin1, l1++, 1, ">>>>>>>>>>> IF -> ID <<<<<<<<<<<");
  l1++;
  mvwprintw(infowin1, l1++, 3, "INST ------- %s", instView->instIF_ID);
  decodificarInstrucao(instView->instIF_ID, inst, decod);
	printAssemblyNcurses(infowin1, l1++, 3, instView->instIF_ID, decod);
  //mvwprintw(infowin1, l1++, 3, "INST ------- %s", reg->if_id.inst);

	// ID -> EX
	int l2 = 1;
	box(infowin2, 0, 0);
	mvwprintw(infowin2, l2++, 1, ">>>>>>>>>>> ID -> EX <<<<<<<<<<<");
  l2++;
  mvwprintw(infowin2, l2++, 3, "INST ------- %s", instView->instID_EX);
  decodificarInstrucao(instView->instID_EX, inst, decod);
  printAssemblyNcurses(infowin2, l2++, 3, instView->instID_EX, decod);
	mvwprintw(infowin2, l2++, 3, "memreg ----- %d", reg->id_ex.memreg);
	mvwprintw(infowin2, l2++, 3, "escreg ----- %d", reg->id_ex.escreg);
	mvwprintw(infowin2, l2++, 3, "branch ----- %d", reg->id_ex.branch);
	mvwprintw(infowin2, l2++, 3, "jump ------- %d", reg->id_ex.jump);
	mvwprintw(infowin2, l2++, 3, "escmem ----- %d", reg->id_ex.escmem);
	mvwprintw(infowin2, l2++, 3, "regdest ---- %d", reg->id_ex.regdest);
	mvwprintw(infowin2, l2++, 3, "ulafonte --- %d", reg->id_ex.ulafonte);
	mvwprintw(infowin2, l2++, 3, "opula ------ %d", reg->id_ex.opula);
	mvwprintw(infowin2, l2++, 3, "opcode ----- %d", reg->id_ex.opcode);
	mvwprintw(infowin2, l2++, 3, "a ---------- %d", reg->id_ex.a);
	mvwprintw(infowin2, l2++, 3, "b ---------- %d", reg->id_ex.b);
	mvwprintw(infowin2, l2++, 3, "imm -------- %d", reg->id_ex.imm);
	mvwprintw(infowin2, l2++, 3, "rs --------- %d", reg->id_ex.rs);
	mvwprintw(infowin2, l2++, 3, "rt --------- %d", reg->id_ex.rt);
	mvwprintw(infowin2, l2++, 3, "rd --------- %d", reg->id_ex.rd);

	// EX -> MEM
	int l3 = 1;
	box(infowin3, 0, 0);
	mvwprintw(infowin3, l3++, 1, ">>>>>>>>>>> EX -> MEM <<<<<<<<<<");
  l3++;
  mvwprintw(infowin3, l3++, 3, "INST ------- %s", instView->instEX_MEM);
  decodificarInstrucao(instView->instEX_MEM, inst, decod);
  printAssemblyNcurses(infowin3, l3++, 3, instView->instEX_MEM, decod);
	mvwprintw(infowin3, l3++, 3, "memreg ----- %d", reg->ex_mem.memreg);
	mvwprintw(infowin3, l3++, 3, "escreg ----- %d", reg->ex_mem.escreg);
	mvwprintw(infowin3, l3++, 3, "escmem ----- %d", reg->ex_mem.escmem);
	mvwprintw(infowin3, l3++, 3, "saidaula --- %d", reg->ex_mem.saidaula);
	mvwprintw(infowin3, l3++, 3, "b ---------- %d", reg->ex_mem.b);
	mvwprintw(infowin3, l3++, 3, "rd --------- %d", reg->ex_mem.rd);

	// MEM -> WB
	int l4 = 1;
	box(infowin4, 0, 0);
	mvwprintw(infowin4, l4++, 1, ">>>>>>>>>>> MEM -> WB <<<<<<<<<<");
  l4++;
  mvwprintw(infowin4, l4++, 3, "INST ------- %s", instView->instMEM_WB);
  decodificarInstrucao(instView->instMEM_WB, inst, decod);
  printAssemblyNcurses(infowin4, l4++, 3, instView->instMEM_WB, decod);
	mvwprintw(infowin4, l4++, 3, "memreg ----- %d", reg->mem_wb.memreg);
	mvwprintw(infowin4, l4++, 3, "escreg ----- %d", reg->mem_wb.escreg);
	mvwprintw(infowin4, l4++, 3, "dadomem ---- %d", reg->mem_wb.dadomem);
	mvwprintw(infowin4, l4++, 3, "saidaula --- %d", reg->mem_wb.saidaula);
	mvwprintw(infowin4, l4++, 3, "rd --------- %d", reg->mem_wb.rd);

	// Exibe tudo
	wrefresh(infowin0);
	wrefresh(infowin1);
	wrefresh(infowin2);
	wrefresh(infowin3);
	wrefresh(infowin4);

	// Espera 1 tecla
	getch();

	// Finaliza
	delwin(infowin0); delwin(infowin1); delwin(infowin2); delwin(infowin3); delwin(infowin4);
	endwin();
}

//imprime instrução decodificada
void printAssemblyNcurses(WINDOW *win, int linha, int index, char *bin, Decod *decod) {
	char instrucao[64];

	switch (decod->opcode) {
	case 0:
		switch (decod->funct) {
		case 0:
			snprintf(instrucao, sizeof(instrucao), "add $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		case 2:
			snprintf(instrucao, sizeof(instrucao), "sub $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		case 4:
			snprintf(instrucao, sizeof(instrucao), "and $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		case 5:
			snprintf(instrucao, sizeof(instrucao), "or $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		default:
			snprintf(instrucao, sizeof(instrucao), "funct invalido");
		}
		break;
	case 4:
		snprintf(instrucao, sizeof(instrucao), "addi $%d, $%d, %d", decod->rt, decod->rs, decod->imm);
		break;
	case 11:
		snprintf(instrucao, sizeof(instrucao), "lw $%d, %d($%d)", decod->rt, decod->imm, decod->rs);
		break;
	case 15:
		snprintf(instrucao, sizeof(instrucao), "sw $%d, %d($%d)", decod->rt, decod->imm, decod->rs);
		break;
	case 8:
		snprintf(instrucao, sizeof(instrucao), "beq $%d, $%d, %d", decod->rs, decod->rt, decod->imm);
		break;
	case 2:
		snprintf(instrucao, sizeof(instrucao), "j %d", decod->addr);
		break;
	default:
		snprintf(instrucao, sizeof(instrucao), "opcode invalido");
	}

	// Imprime tudo junto na mesma linha
	mvwprintw(win, linha, index, "%s", instrucao);
}

//imprime instrução decodificada
void printInstrucaoNcurses(WINDOW *win, int linha, int index, char *bin, Decod *decod) {
	char instrucao[64];

	switch (decod->opcode) {
	case 0:
		switch (decod->funct) {
		case 0:
			snprintf(instrucao, sizeof(instrucao), "add $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		case 2:
			snprintf(instrucao, sizeof(instrucao), "sub $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		case 4:
			snprintf(instrucao, sizeof(instrucao), "and $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		case 5:
			snprintf(instrucao, sizeof(instrucao), "or $%d, $%d, $%d", decod->rd, decod->rs, decod->rt);
			break;
		default:
			snprintf(instrucao, sizeof(instrucao), "funct invalido");
		}
		break;
	case 4:
		snprintf(instrucao, sizeof(instrucao), "addi $%d, $%d, %d", decod->rt, decod->rs, decod->imm);
		break;
	case 11:
		snprintf(instrucao, sizeof(instrucao), "lw $%d, %d($%d)", decod->rt, decod->imm, decod->rs);
		break;
	case 15:
		snprintf(instrucao, sizeof(instrucao), "sw $%d, %d($%d)", decod->rt, decod->imm, decod->rs);
		break;
	case 8:
		snprintf(instrucao, sizeof(instrucao), "beq $%d, $%d, %d", decod->rs, decod->rt, decod->imm);
		break;
	case 2:
		snprintf(instrucao, sizeof(instrucao), "j %d", decod->addr);
		break;
	default:
		snprintf(instrucao, sizeof(instrucao), "opcode invalido");
	}

	// Imprime tudo junto na mesma linha
	mvwprintw(win, linha, 2, "[%d]. %s -> %s", index, bin, instrucao);
}

//imprime memoria de instruções
void printImemory(char mi[256][17], Inst *inst, Decod *decod) {
	int count = 0;
  while (strcmp(mi[count], "0000000000000000" ) != 0 && count < 256) {
    count ++;
  }

	int height = count + 5;
	int width = 44;
  int deslocamentoX = 2;

	WINDOW *memWin = newwin(height, width, 1, 1);
	box(memWin, 0, 0);

	int linha = 1;
  int i = 0;

  mvwprintw(memWin, linha++, deslocamentoX, "############# INSTRUCOES #############");
	while (strcmp(mi[i], "0000000000000000") != 0) {
		decodificarInstrucao(mi[i], inst, decod);
		printInstrucaoNcurses(memWin, linha++, i, mi[i], decod);
    i ++;
	}

	mvwprintw(memWin, linha++, deslocamentoX, "------ Fim das instrucoes validas ------");
	wrefresh(memWin);
	wgetch(memWin);
	delwin(memWin);
}

// imprime memoria de dados
void printDmemory(int *md) {
  WINDOW *memWinD = newwin(36, 83, 1, 45);
	box(memWinD, 0, '=');

  mvwprintw(memWinD, 1, 18.5, "############## MEMORIA DE DADOS ##############");
  int linha = 3;
  for(int i=0; i<256; i++) {
    int col = (i % 8) * 10 + 2;  // espaçamento horizontal
    mvwprintw(memWinD, linha, col, "[%d].%d", i, md[i]);
    if (i % 8 == 7)
    {
      linha ++;
    }
  }
	wrefresh(memWinD);
	wgetch(memWinD);
	delwin(memWinD);
}

void infoPipeline(Reg *reg, int ciclo, UF *uf, WINDOW *win) {
    werase(win);
    box(win, 0, 0);
    int linha = 1;

    Inst inst_temp;
    Decod decod_temp;
    int valor;

    mvwprintw(win, linha++, 2, "================== VISUALIZACAO DO PIPELINE (Ciclo %d) ==================", ciclo);
    //linha++;
    mvwprintw(win, linha++, 1, "+-----------+---------------------+--------------------------------------+");
    mvwprintw(win, linha++, 1, "| Estagio   | Instrucao Atual     | Detalhes da Execucao                 |");
    mvwprintw(win, linha++, 1, "+-----------+---------------------+--------------------------------------+");

    // IF - Busca
    mvwprintw(win, linha++, 1, "| Busca     | ");
    if (strcmp(reg->if_id.inst, "0000000000000000") != 0) {
        mvwprintw(win, linha - 1, 15, "%-19s | ", reg->if_id.inst);
        decodificarInstrucao(reg->if_id.inst, &inst_temp, &decod_temp);
        char buffer[64];
        if (decod_temp.opcode > 9)
        {
          snprintf(buffer, sizeof(buffer), "Busca: opcode=%d rd=%d rs=%d rt=%d      |", decod_temp.opcode, decod_temp.rd, decod_temp.rs, decod_temp.rt);
        }
        else
        {
          snprintf(buffer, sizeof(buffer), "Busca: opcode=%d rd=%d rs=%d rt=%d       |", decod_temp.opcode, decod_temp.rd, decod_temp.rs, decod_temp.rt);
        }
        mvwprintw(win, linha - 1, 37, "%s", buffer);
    } else {
        mvwprintw(win, linha - 1, 15, "NOP                 | Estagio vazio ou instrucao NOP       |");
    }

    // ID - Decodifica
    mvwprintw(win, linha++, 1, "| Decodifica| ");
    if (reg->id_ex.opcode != 0 || reg->id_ex.rd != 0) {
        char *nome;
        switch (reg->id_ex.opcode) {
            case 0:  nome = "Tipo R"; break;
            case 4:  nome = "ADDI"; break;
            case 8:  nome = "BEQ"; break;
            case 11: nome = "LW"; break;
            case 15: nome = "SW"; break;
            case 2:  nome = "J"; break;
            default: nome = "Desconhecida"; break;
        }
        mvwprintw(win, linha - 1, 15, "%-19s | rd=$%d                                |", nome, reg->id_ex.rd);
    } else {
        mvwprintw(win, linha - 1, 15, "NOP                 | Estagio vazio ou instrucao NOP       |");
    }

    // EX - Executa
    mvwprintw(win, linha++, 1, "| Executa   | ");
    if (reg->ex_mem.rd != 0 || reg->ex_mem.escmem) {
        if (reg->ex_mem.escmem) {
            mvwprintw(win, linha - 1, 15, "SW                  | Endereco: %d + %d       |", reg->id_ex.a, reg->id_ex.imm);
        } else {
            char *op;
            int b = ULAFonteB(reg->id_ex.b, reg->ex_mem.saidaula,
                              MemReg(reg->mem_wb.saidaula, reg->mem_wb.dadomem, reg->mem_wb.memreg),
                              reg->id_ex.imm, reg->id_ex.ulafonte);

            switch (reg->id_ex.opula) {
                case 0: op = "ADD"; break;
                case 2: op = "SUB"; break;
                case 4: op = "AND"; break;
                case 5: op = "OR"; break;
                default: op = "Desconhecida"; break;
            }
            mvwprintw(win, linha - 1, 15, "Operacao ULA        | %s: %d e %d                           |", op, reg->id_ex.a, b);
        }
    } else {
        mvwprintw(win, linha - 1, 15, "NOP                 | Estagio vazio ou instrucao NOP       |");
    }

    // MEM - Memória
    mvwprintw(win, linha++, 1, "| Memoria   | ");
    if (reg->ex_mem.escmem) {
        mvwprintw(win, linha - 1, 15, "SW                  | Escrevendo %d em mem[%d]       |", reg->ex_mem.b, reg->ex_mem.saidaula);
    } else if (reg->ex_mem.escreg && reg->ex_mem.memreg == 0) {
        mvwprintw(win, linha - 1, 15, "LW                  | Lendo mem[%d]                         |", reg->ex_mem.saidaula);
    } else if (reg->ex_mem.rd != 0) {
        mvwprintw(win, linha - 1, 15, "Operacao ULA        | Resultado: %d para $%d                 |", reg->ex_mem.saidaula, reg->ex_mem.rd);
    } else {
        mvwprintw(win, linha - 1, 15, "NOP                 | Estagio vazio ou instrucao NOP       |");
    }

    // WB - Writeback
    mvwprintw(win, linha++, 1, "| Escreve   | ");
    if (reg->mem_wb.escreg) {
        if (reg->mem_wb.memreg) {
            valor = reg->mem_wb.saidaula;
            mvwprintw(win, linha - 1, 15, "Resultado ULA       | Escrevendo %d em $%d                   |", valor, reg->mem_wb.rd);
        } else {
            valor = reg->mem_wb.dadomem;
            mvwprintw(win, linha - 1, 15, "Dado da Memoria     | Escrevendo %d em $%d                   |", valor, reg->mem_wb.rd);
        }
    } else {
        mvwprintw(win, linha - 1, 15, "NOP                 | Estagio vazio ou instrucao NOP       |");
    }

    mvwprintw(win, linha++, 1, "+-----------+---------------------+--------------------------------------+");

    // Informações adicionais
    mvwprintw(win, linha++, 2, "Informacoes Adicionais:");
    mvwprintw(win, linha++, 4, "- PC atual: %d", reg->pc);
    mvwprintw(win, linha++, 4, "- Proxima instrucao: %d", reg->if_id.pc);

    if (uf->a != 0 || uf->b != 0) {
        mvwprintw(win, linha++, 2, "HAZARD Dependencia de dados:");
        if (uf->a == 1 || uf->b == 1)
            mvwprintw(win, linha++, 4, "- Hazard com estagio MEM");
        if (uf->a == 2 || uf->b == 2)
            mvwprintw(win, linha++, 4, "- Hazard com estagio WB");
    }

    if ((reg->id_ex.branch && reg->ex_mem.saidaula == 0) || reg->id_ex.jump)
        mvwprintw(win, linha++, 2, "[HAZARD] Controle detectado");

    mvwprintw(win, linha++, 2, "Legenda:");
    mvwprintw(win, linha++, 4, "- NOP: Estagio vazio ou instrucao que nao faz nada");
    mvwprintw(win, linha++, 4, "- Tipo R: Instrucoes como add, sub, and, or");
    mvwprintw(win, linha++, 4, "- lw/sw: Acesso a memoria (load/store)");
    mvwprintw(win, linha++, 4, "- beq: Desvio condicional");
    mvwprintw(win, linha++, 4, "- j: Salto incondicional");

    linha++;

    mvwprintw(win, linha++, 4, "Pressione qualquer tecla para continuar...");
    wrefresh(win);
    wgetch(win);
}

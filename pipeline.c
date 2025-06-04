#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funcao para iniciar pilha
void inicia_pilha(Pilha *p) {
  p->topo=NULL;
}

void menu() {
  printf("\n ******* MENU *******\n");
  printf("\n 1 - Carregar memoria\n");
  printf(" 2 - Imprimir memoria\n");
  printf(" 3 - Imprimir banco de registradores\n");
  printf(" 4 - Imprimir todo o simulador\n");
  printf(" 5 - Executar step\n");
  printf(" 6 - Executar run\n");
  printf(" 7 - Salvar .asm\n");
  printf(" 8 - Volta um step\n");
  printf(" 9 - Sair\n");
}

// Imprime banco de registradores
void print_br(int *r) {
  printf("\n ------ BANCO DE REGISTRADORES ------ \n\n");
  for(int i=0; i<8; i++) {
    printf(" REGISTRADOR [%d]: %d\n", i, r[i]);
  }
}

// Funcao para pilha usada no step back
void empilha(Pilha *p, Decodificador *d, char (*mem)[17], Registradores *r, int *est) {
  Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
  if (nNodo == NULL) {
    printf("Erro ao alocar memC3ria para novo nC3\n");
    exit(1);
  }
  int i;
  nNodo->est_a = *est;
  nNodo->pca = r->pc;
  for(i = 0; i < 128; i++) {
    strncpy(nNodo->da[i], mem[i+128], 16);
    nNodo->da[i][16] = '\0';
  }
  strncpy(nNodo->ria, r->ri, 16);
  nNodo->ria[16] = '\0';
  strncpy(nNodo->rdma, r->rdm, 16);
  nNodo->rdma[16] = '\0';
  for(i = 0; i < 8; i++) {
    nNodo->bra[i] = r->br[i];
  }
  nNodo->aa = r->a;
  nNodo->ba = r->b;
  nNodo->ula_saidaa = r->ula_saida;
  nNodo->prox = p->topo;
  p->topo = nNodo;
}

// Funcao para decodificar instrucao
void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d) {
  copiarBits(bin, in->opcode, 0, 4);    // Copia os 4 bits do opcode (4 bits)
  d->opcode = binarioParaDecimal(in->opcode, 0);
  copiarBits(bin, in->rs, 4, 3);        // Copia os 3 bits do rs
  d->rs = binarioParaDecimal(in->rs, 0);
  copiarBits(bin, in->rt, 7, 3);        // Copia os 3 bits do rt
  d->rt = binarioParaDecimal(in->rt, 0);
  copiarBits(bin, in->rd, 10, 3);       // Copia os 3 bits do rd
  d->rd = binarioParaDecimal(in->rd, 0);
  copiarBits(bin, in->funct, 13, 3);    // Copia os 3 bits do funct
  d->funct = binarioParaDecimal(in->funct, 0);
  copiarBits(bin, in->imm, 10, 6);      // Copia os 6 bits do imm
  d->imm = binarioParaDecimal(in->imm, 1);
  copiarBits(bin, in->addr, 8, 8);      // Copia os 8 bits do addr
  d->addr = binarioParaDecimal(in->addr, 0);
}

// Copia os bits da instrucao para cada campo da struct instrucao
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho) {
  strncpy(destino, instrucao + inicio, tamanho); // Copia os bits desejados
  destino[tamanho] = '\0';  // Adiciona o terminador de string
}

// Converte de binario para decimal
int binarioParaDecimal(const char *bin, int sinal) {
  int valor = (int)strtol(bin, NULL, 2);
  int bits = strlen(bin);

  if (sinal && bits > 0 && (valor & (1 << (bits - 1)))) {
    valor = valor - (1 << bits);
  }
  return valor;
}

// MUX para o operando 1 da ULA
int ULA_fontA(int pc,int a,int ULAFontA) {
  switch(ULAFontA) {
  case 0:
    return pc;
  case 1:
    return a;
  }
}

// MUX para o operando 2 da ULA
int ULA_fontB(int b,int imm, int ULAFontB) {
  switch(ULAFontB) {
  case 0:
    return b;
  case 1:
    return 1;
  case 2:
    return imm;
  }
}

// Funcao ULA
void ULA(int op1, int op2, int opULA, ALUout *saida) {
  switch(opULA) {
  case 0:
    saida->resultado = op1 + op2;

    if(saida->resultado == 0) {
      saida->flag_zero = 1;
    }

    if ((op1 > 0 && op2 > 0 && saida->resultado < 0) || (op1 < 0 && op2 < 0 && saida->resultado > 0)) {
      saida->overflow = 1;
      printf("OVERFLOW - ADD: %d + %d = %d\n", op1, op2, saida->resultado);
    }
    break;

  case 2:
    saida->resultado = op1 - op2;

    if(saida->resultado == 0) {
      saida->flag_zero = 1;
    }

    if ((op1 > 0 && op2 < 0 && saida->resultado < 0) || (op1 < 0 && op2 > 0 && saida->resultado > 0)) {
      saida->overflow = 1;
      printf("OVERFLOW - SUB: %d - %d = %d\n", op1, op2, saida->resultado);
    }
    break;

  case 4:
    saida->resultado = op1 & op2;
    break;

  case 5:
    saida->resultado = op1 | op2;
    break;
  }
}

// Funcao para imprimir a instrucao
void printInstrucao(Decodificador *d) {
  switch (d->opcode) {
  case 0: // Tipo R (add, sub, and, or)
    switch (d->funct) {
    case 0:
      printf(" add $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
      break;
    case 2:
      printf(" sub $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
      break;
    case 4:
      printf(" and $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
      break;
    case 5:
      printf(" or $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
      break;
    }
    break;
  case 4: // addi
    printf(" addi $%d, $%d, %d\n", d->rt, d->rs, d->imm);
    break;
  case 11: // lw
    printf(" lw $%d, %d($%d)\n", d->rt, d->imm, d->rs);
    break;
  case 15: // sw
    printf(" sw $%d, %d($%d)\n", d->rt, d->imm, d->rs);
    break;
  case 8: // beq
    printf(" beq $%d, $%d, %d\n", d->rs, d->rt, d->imm);
    break;
  case 2: // j
    printf(" j %d\n", d->addr);
    break;
  }
}

//Funcao para converter os bits do imediato para decimal
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d) {
  copiarBits(data, in->dado, 8, 8);    // Copia os 4 bits do opcode (4 bits)
  d->dado = binarioParaDecimal(in->dado, 1);
}

// Funcao auxiliar recursiva
void int_para_binario_recursiva(int valor, char *binario, int pos) {
  if(pos < 0) {
    return;
  }

  if(valor & 1) {
    binario[pos] = '1';
  }
  else {
    binario[pos] = '0';
  }

  int_para_binario_recursiva(valor >> 1, binario, pos-1);
}

// Funcao principal
void int_para_binario(int valor, char *binario) {
  int i;

  for(i = 0; i < 16; i++) {
    binario[i] = '0';
  }

  binario[16] = '\0';

  int_para_binario_recursiva(valor, binario, 15);
}

// Funcao para conversao das instrucoes para assembly e salvar "arquivo.asm"
void salvarAssembly(char mem[256][17]) {
  char arquivo[20];

  printf("Nome do arquivo .asm: ");
  scanf("%s", arquivo);

  FILE *arq = fopen(arquivo, "w");
  if (!arq) {
    perror("Erro ao criar arquivo");
    return;
  }

  for (int i = 0; i < 128; i++) {
    if (mem[i][0] == '\0') continue; // Ignora posiC'C5es vazias

    struct instrucao inst;
    Decodificador d;
    decodificarInstrucao(mem[i], &inst, &d);

    // Converte para assembly e escreve no arquivo
    switch (d.opcode) {
    case 0: // Tipo R (add, sub, and, or)
      switch (d.funct) {
      case 0:
        fprintf(arq, "add $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
        break;
      case 2:
        fprintf(arq, "sub $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
        break;
      case 4:
        fprintf(arq, "and $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
        break;
      case 5:
        fprintf(arq, "or $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
        break;
      }
      break;
    case 4: // addi
      fprintf(arq, "addi $%d, $%d, %d\n", d.rt, d.rs, d.imm);
      break;
    case 11: // lw
      fprintf(arq, "lw $%d, %d($%d)\n", d.rt, d.imm, d.rs);
      break;
    case 15: // sw
      fprintf(arq, "sw $%d, %d($%d)\n", d.rt, d.imm, d.rs);
      break;
    case 8: // beq
      fprintf(arq, "beq $%d, $%d, %d\n", d.rs, d.rt, d.imm);
      break;
    case 2: // j
      fprintf(arq, "j %d\n", d.addr);
      break;
    }
  }
  fclose(arq);
  printf("Arquivo %s salvo com sucesso!\n", arquivo);
}

// Funcao de execucao do step back
int step_back(Pilha *p, Registradores *r, char (*mem)[17], int *est) {
  int i;

  if(limite_back(p) == 1) {
    return 1;
  } else {
    Nodo *remover = p->topo;
    *est = remover->est_a;
    r->pc = remover->pca;
    r->a = remover->aa;
    r->b = remover->ba;
    r->ula_saida = remover->ula_saidaa;
    for(i = 0; i < 8; i++) {
      r->br[i] = remover->bra[i];
    }
    strncpy(r->ri, remover->ria, 16);
    r->ri[16] = '\0';
    strncpy(r->rdm, remover->rdma, 16);
    r->rdm[16] = '\0';
    for(i = 0; i < 128; i++) {
      strncpy(mem[i+128], remover->da[i], 16);
      mem[i][16] = '\0';
    }
    p->topo = remover->prox;
    free(remover);
    return 0;
  }
}

// Limite do step back, termina desempilhamento na primeira instrucao executada
int limite_back(Pilha *p) {
  if(p->topo==NULL) {
    printf("\nVoce voltou ao inicio!");
    return 1;
  }
}


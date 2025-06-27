typedef struct sinais {
  int RegDest,
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
pc,
      a,
      b,
      imm,
      rs,
      rt,
      rd;
  char instID_EX[17];
} ID_EX;

typedef struct ex_mem {
  int memreg,
      escreg,
      escmem,
      saidaula,
      b,
      rd;
  char instEX_MEM[17];
} EX_MEM;

typedef struct mem_wb {
  int memreg,
      escreg,
      dadomem,
      saidaula,
      rd;
  char instMEM_WB[17];
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
      ciclo;
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
void infoWin(Reg *reg, Decod *decod, Inst *inst);
void printAssemblyNcurses(WINDOW *win, int linha, int index, char *bin, Decod *decod);
void inputJanelaArquivo(char *buffer, int maxlen);
void errorwin();
void printInstrucaoNcurses(WINDOW *win, int linha, int index, char *bin, Decod *decod);
void printImemory(char mi[256][17], Inst *inst, Decod *decod);
void printDmemory(int *md);
void infoPipeline(Reg *reg, int ciclo, UF *uf, WINDOW *win, int entradaA, int entradaB);

int carregaMemInst(char mi[256][17]);
void carregarMemoriaDados(int md[256]);

void printMemory(char mi[256][17], Inst *inst, Decod *decod);
void printmemory(int *md);
void printReg(Reg *reg);
void printInstrucao(Decod *decod);

void decodificarInstrucao(const char *bin, Inst *inst, Decod *decod);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);

void inicia_pilha(Stack *stack);
int step_back(Stack *stack,Reg *reg, int *md,int *ciclo);
void empilha(Stack *stack,Reg *reg, int *md,int *ciclo);
int limite_back(Stack *stack);

void ULA(int op1, int op2, int opULA, ULA_Out *ula_out);
void Forward(int rs, int rt,int rd_mem,int rd_wb,int opcode,int ulafonte, UF *uf);
int somador(int op1, int op2);
int AND(int op1, int op2);

void salvarAssembly(char mi[256][17]);
void salvarMemDados(int *md);

void controle(int opcode, int funct, Sinais *sinais);

int executa_ciclo(char mi[256][17],Inst *inst,Decod *decod,Reg *reg,int *md,Sinais *sinais,ULA_Out *ula_out,int *ciclo,Stack *stack,UF *uf);

//MUX
int MemReg(int op2, int op1, int MemParaReg);
int RegDest(int op2, int op1, int Reg_Dest);
int ULAFonteA(int a,int saidaula,int dado,int ULAFonte);
int ULAFonteB(int b,int imm,int saidaula,int dado,int ULAFonte);
int NOP(int opcode, int op_ant, int rd, int escreg, int escmem, int flag, int *reg, int *mem, Decod *decod);
int ESC_IF(int op_ant, int EscIF_ID, int comparacao);
int FontePC1(int pc1, int pcimm, int branch);
int FontePC2(int fonte, int imm, int jump);

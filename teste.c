#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define MI char mi[256][17] = {{'\0'}}
#define MD int md[256] = {0}

typedef struct sinais {
	int EscPC,
	    RegDest,
	    ULAOp,
	    ULAFonte,
	    DC,
	    DI,
	    EscMem,
	    EscReg,
	    MemParaReg;
} Sinais;

typedef struct if_id {
	int pc;
	char inst[17];
} IF_ID;

typedef struct registradores {
	int pc,
	    br[8],
	    id_ex[16],
	    ex_mem[8],
	    mem_wb[5];
	IF_ID if_id;
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
	    if_id[17],
	    id_ex[14],
	    ex_mem[8],
	    mem_wb[5],
	    md[256];
	struct Nodo *prox;
} Nodo;

typedef struct pilha {
	Nodo *topo;
} Stack;

//ASSINATURA DAS FUNCOES
int menu(WINDOW *menuwin);

int carregaMemInst(char mi[256][17]);
void carregarMemoriaDados(int md[256]);

void printMemory(char mi[256][17], Inst *inst, Decod *decod);
void printmemory(int *md);
void printReg(Reg *reg, WINDOW *regwin);
void printInstrucao(Decod *decod);

void decodificarInstrucao(const char *bin, Inst *inst, Decod *decod);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);

void inicia_pilha(Stack *stack);
int step_back(Stack *stack,Reg *reg, int *md);
void empilha(Stack *stack,Reg *reg, int *md);
int limite_back(Stack *stack);

void ULA(int op1, int op2, int opULA, ULA_Out *ula_out);
int somador(int op1, int op2);

void salvarAssembly(char mi[256][17]);
void salvarMemDados(int *md);

void executa_ciclo(char mi[256][17], Inst *inst, Decod *decod, Reg *reg, int *md, Stack *stack, Sinais *sinais, ULA_Out *ula_out);
void controle(int opcode, int funct, Sinais *sinais);

void escreve_br(int *reg, int dado, int EscReg);
void escreve_md(int *index, int dado, int EscMem);
void escreve_pc(int *pc, int op2, int EscPC);

//MUX
int MemReg(int op2, int op1, int MemParaReg);
int RegDest(int op2, int op1, int Reg_Dest);
int ULAFonte(int b, int imm, int ULAFonte);

// PROGRAMA PRINCIPAL
int main() {
	
	//==================================================================
	// controle ncurses, inicializacao das janelas
	initscr();              // Inicia ncurses
    	noecho();               // Não exibe teclas digitadas
    	cbreak();               // Leitura imediata de teclas
    	curs_set(0);            // Oculta o cursor
    	keypad(stdscr, TRUE);   // Habilita teclas especiais como setas
    
    	int yMax, xMax;
    	getmaxyx(stdscr, yMax, xMax);

    	// Janela do menu (largura 50, altura suficiente para opções)
    	WINDOW *menuwin = newwin(16, 50, (yMax - 16) / 2, 0);
    	keypad(menuwin, TRUE);

    	// Janela dos registradores ao lado do menu
    	WINDOW *regwin = newwin(15, 40, (yMax - 15) / 2, 52);
	//==================================================================
	
	Sinais sinais;
	Inst inst;
	Decod decod;
	Stack stack;
	Reg reg = {0};
	ULA_Out ula_out;
	MI;
	MD;

	inicia_pilha(&stack);

	int op, nl, resul;
	reg.pc = 0;

	do {
		op = menu(menuwin);
		printf("\n\nVocê escolheu a opção: %d\n", op);
		printf("\n");
		switch (op) {
		case 1:
			carregaMemInst(mi);
			getchar();
			break;
		case 2:
			carregarMemoriaDados(md);
			getchar();
			break;
		case 3:
			printMemory(mi, &inst, &decod);
			printmemory(md);
			getchar();
			break;
		case 4:
			printReg(&reg, regwin);
			getchar();
			werase(regwin);      // Limpa a janela
			wrefresh(regwin);    // Atualiza com o conteúdo limpo
			break;
		case 5:
			printMemory(mi, &inst, &decod);
			printmemory(md);
			printReg(&reg, regwin);
			printf("\n\nPC: %d", reg.pc);
			getchar();
			break;
		case 6:
			salvarAssembly(mi);
			getchar();
			break;
		case 7:
			salvarMemDados(md);
			getchar();
			break;
		case 8:

			break;
		case 9:
			executa_ciclo(mi, &inst, &decod, &reg, md, &stack, &sinais, &ula_out);
			getchar();
			break;
		case 10:
			step_back(&stack,&reg,md);
			getchar();
			break;
		case 11:
			printf("Voce saiu!!!\n");
			break;
		}
	} while(op != 11);
	
	delwin(menuwin);
    delwin(regwin);
    endwin();
	return 0;
}

//FUNCOES IMPLEMENTADAS

//MENU
int menu(WINDOW *menuwin) {
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
    int ch;

    while (1) {
        werase(menuwin);        // Limpa só a janela do menu
        box(menuwin, 0, 0);     // Borda

        mvwprintw(menuwin, 1, 18, "*** MENU ***");

        for (int i = 0; i < n_opcoes; i++) {
            if (i == escolha)
                wattron(menuwin, A_REVERSE);
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
            case 10:  // Enter
                return escolha + 1;
        }
    }
}

// carrega memoria de instrucoes a partir de um "arquivo.mem"
int carregaMemInst(char mi[256][17]) {
	char arquivo[20],extensao[5];
	int tam;

	// abre o arquivo em modo leitura
	printf("Nome do arquivo: ");
	scanf("%s", arquivo);

	tam = strlen(arquivo);
	strncpy(extensao,arquivo + tam - 4,4);
	extensao[4] = '\0';

	if(strcmp(extensao, ".mem") != 0) {
		printf("Extensao de arquivo nao suportada!\n");
	} else {
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
	}
}

//carrega memoria de dados a partir de um "arquivo.dat"
void carregarMemoriaDados(int md[256]) {
	char arquivo[20],extensao[5];
	int tam;

	printf("Nome do arquivo: ");
	scanf("%s", arquivo);

	tam = strlen(arquivo);
	strncpy(extensao,arquivo + tam - 4,4);
	extensao[4] = '\0';

	if(strcmp(extensao, ".dat") != 0) {
		printf("Extensao de arquivo nao suportada!\n");
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
void printReg(Reg *reg, WINDOW *regwin) {
    werase(regwin);
    box(regwin, 0, 0);

    mvwprintw(regwin, 1, 2, "BANCO DE REGISTRADORES");

    for (int i = 0; i < 8; i++) {
        mvwprintw(regwin, i + 3, 2, "REGISTRADOR [%d] = %d", i, reg->br[i]);
    }

    mvwprintw(regwin, 13, 2, "Pressione qualquer tecla...");
    wrefresh(regwin);

    wgetch(regwin);
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
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho)
{
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

	printf("Nome do arquivo .asm: ");
	scanf("%s", arquivo);

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

	printf("Salvar como: ");
	scanf("%s", nomeArquivo);

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

/* salva registradores, memoria de dados e pc na pilha
void empilha(Stack *stack,Reg *reg, int *md) {
	Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
	int i;
	for(i=0; i<8; i++) {
		nNodo->br[i]=reg->br[i];
	}
	for(i=0; i<17; i++) {
		nNodo->if_id[i]=reg->if_id[i];
	}
	for(i=0; i<14; i++) {
		nNodo->id_ex[i]=reg->id_ex[i];
	}
	for(i=0; i<8; i++) {
		nNodo->ex_mem[i]=reg->ex_mem[i];
	}
	for(i=0; i<6; i++) {
		nNodo->mem_wb[i]=reg->mem_wb[i];
	}
	for(i=0; i<256; i++) {
		nNodo->md[i]=md[i];
	}
	nNodo->pc=reg->pc;
	nNodo->prox=stack->topo;
	stack->topo=nNodo;
}*/

// funcao de execucao do step back
int step_back(Stack *stack, Reg *reg, int *md) {
	int i;

	if(limite_back(stack) == 1) {
		return 1;
	} else {
		Nodo *remover = stack->topo;
		for(i = 0; i < 8; i++) {
			reg->br[i] = remover->br[i];
		}
		for(i = 0; i < 8; i++) {
			md[i] = remover->md[i];
		}
		reg->pc = remover->pc;
		stack->topo = remover->prox;
		free(remover);
		return 0;
	}
}

// somador
int somador(int op1, int op2) {
	return op1 + op2;
}

// limite do step back, termina desempilhamento na primeira instrucao executada
int limite_back(Stack *stack) {
	if(stack->topo==NULL) {
		printf("\nVoce voltou ao inicio!");
		return 1;
	}
}

void executa_ciclo(char mi[256][17], Inst *inst, Decod *decod, Reg *reg, int *md, Stack *stack, Sinais *sinais, ULA_Out *ula_out) {
	if(strcmp(mi[reg->pc], "0000000000000000") == 0) {
		printf("########## EXECUCAO CONCLUIDA! ##########\n");
		return;
	} else {
	int num_pc;
	char char_pc;

	//empilha(stack,reg,md);

	escreve_br(&reg->br[reg->mem_wb[0]], MemReg(reg->mem_wb[1], reg->mem_wb[2], reg->mem_wb[4]), reg->mem_wb[3]);

	reg->mem_wb[0] = reg->ex_mem[0];
	reg->mem_wb[1] = reg->ex_mem[2];
	reg->mem_wb[2] = reg->ex_mem[2];
	reg->mem_wb[3] = reg->ex_mem[4];
	reg->mem_wb[4] = reg->ex_mem[5];

	escreve_md(&md[reg->ex_mem[2]], reg->ex_mem[1], reg->ex_mem[3]);

	reg->ex_mem[0] = RegDest(reg->id_ex[0], reg->id_ex[1], reg->id_ex[8]);
	reg->ex_mem[1] = reg->id_ex[4];
	ULA(reg->id_ex[5], ULAFonte(reg->id_ex[3], reg->id_ex[4], reg->id_ex[7]), reg->ex_mem[6], ula_out);
	reg->ex_mem[2] = ula_out->resultado;
	reg->ex_mem[3] = reg->id_ex[9];
	reg->ex_mem[4] = reg->id_ex[10];
	reg->ex_mem[5] = reg->id_ex[11];
	reg->ex_mem[6] = reg->id_ex[12];
	reg->ex_mem[7] = reg->id_ex[13];

	reg->id_ex[0] = decod->rd;
	reg->id_ex[1] = decod->rs;
	reg->id_ex[2] = reg->if_id.pc;
	reg->id_ex[3] = decod->imm;
	reg->id_ex[4] = reg->br[decod->rt];
	reg->id_ex[5] = reg->br[decod->rs];
	reg->id_ex[6] = sinais->ULAOp;
	reg->id_ex[7] = sinais->ULAFonte;
	reg->id_ex[8] = sinais->RegDest;
	reg->id_ex[9] = sinais->EscMem;
	reg->id_ex[10] = sinais->DI;
	reg->id_ex[11] = sinais->DC;
	reg->id_ex[12] = sinais->EscReg;
	reg->id_ex[13] = sinais->MemParaReg;
	controle(decod->opcode, decod->funct, sinais);
	escreve_pc(&reg->pc,reg->if_id.pc,sinais->EscPC);
	decodificarInstrucao(reg->if_id.inst, inst, decod);

	strcpy(reg->if_id.inst, mi[reg->pc]);
	reg->if_id.pc = somador(reg->pc,1);
//}
}

void controle(int opcode, int funct, Sinais *sinais) {
	if(opcode == 0) {
		sinais->EscPC = 1;
		sinais->RegDest = 1;
		sinais->ULAOp = funct;
		sinais->ULAFonte = 0;
		sinais->DC = 0;
		sinais->DI = 0;
		sinais->EscMem = 0;
		sinais->EscReg = 1;
		sinais->MemParaReg = 1;
	}
	else if(opcode == 2) {
		sinais->EscPC = 1;
		sinais->RegDest = 0;
		sinais->ULAOp = 0;
		sinais->ULAFonte = 0;
		sinais->DC = 0;
		sinais->DI = 1;
		sinais->EscMem = 0;
		sinais->EscReg = 0;
		sinais->MemParaReg = 0;
	}
	else if(opcode == 4) {
		sinais->EscPC = 1;
		sinais->RegDest = 1;
		sinais->ULAOp = 0;
		sinais->ULAFonte = 1;
		sinais->DC = 0;
		sinais->DI = 0;
		sinais->EscMem = 0;
		sinais->EscReg = 1;
		sinais->MemParaReg = 1;
	}
	else if(opcode == 8) {
		sinais->EscPC = 1;
		sinais->RegDest = 0;
		sinais->ULAOp = funct;
		sinais->ULAFonte = 0;
		sinais->DC = 1;
		sinais->DI = 0;
		sinais->EscMem = 0;
		sinais->EscReg = 0;
		sinais->MemParaReg = 0;
	}
	else if(opcode == 11) {
		sinais->EscPC = 1;
		sinais->RegDest = 0;
		sinais->ULAOp = 0;
		sinais->ULAFonte = 1;
		sinais->DC = 0;
		sinais->DI = 0;
		sinais->EscMem = 0;
		sinais->EscReg = 1;
		sinais->MemParaReg = 0;
	}
	else if(opcode == 15) {
		sinais->EscPC = 0;
		sinais->RegDest = 0;
		sinais->ULAOp = 0;
		sinais->ULAFonte = 1;
		sinais->DC = 0;
		sinais->DI = 0;
		sinais->EscMem = 1;
		sinais->EscReg = 0;
		sinais->MemParaReg = 0;
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

int ULAFonte(int b, int imm, int ULAFonte) {
	switch (ULAFonte) {
	case 0:
		return b;
		break;
	case 1:
		return imm;
	}
}

void escreve_pc(int *pc, int op2, int EscPC) {
	if(EscPC == 1) {
		*pc = op2;
	}
}

void escreve_br(int *reg, int dado, int EscReg) {
	if(EscReg == 1) {
		*reg = dado;
	}
}

void escreve_md(int *index, int dado, int EscMem) {
	if(EscMem == 1) {
		*index = dado;
	}
}

// Funcao ULA
void ULA(int op1, int op2, int opULA, ULA_Out *ula_out) {
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

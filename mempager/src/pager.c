#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "pager.h"
#include "uvm.h"
#include "mmu.h"
#include "mmuproto.h"
#include "cyc.h"
#include "log.h"

//estrutura de dados que representa um quadro na memoria fisica
typedef struct Quadro {
    pid_t pid; //id do processo que utiliza o quadro
    int numPagina;
    bool disponivel; //true se disponivel, false caso contrario
} Quadro;

//estrutura de dados que representa uma tabela de paginas de um processo
typedef struct TabelaDePaginas {
	int qtdPaginas; // Usado como tamanho dos dois vetores quadros e blocos.
	int *quadros;
	int *blocos;
} TabelaDePaginas;

typedef struct ListaDeTabelas {
	pid_t pid;
	TabelaDePaginas *tabela;
} ListaDeTabelas;

//vetor de quadros da memoria fisica
Quadro *quadros;
int numeroQuadros;

//vetor de blocos do disco
int *vetorDeBlocos;
int qtdBlocosLivres; // informa quantos blocos ainda estão livres.
int tamanhoVetorBlocos;

//vetor de tabelas
ListaDeTabelas *listaDeTabelas;
int tamanhoListaDeTabelas;

//gu-
//inicializacao do paginador
void pager_init(int nquadros, int nblocos){
    int i;

    //inicializacao do vetor de quadros da memoria fisica
    numeroQuadros = nquadros;
    quadros = (Quadro*) malloc(nquadros* sizeof(Quadro));
    for(i=0; i<nquadros; i++){
        quadros[i].disponivel = true;
        quadros[i].numPagina = 0;
        quadros[i].pid = -1;
    }

    //inicializacao do vetor de blocos
    tamanhoVetorBlocos = nblocos;
    qtdBlocosLivres = nblocos;
    vetorDeBlocos = (int*) malloc(nblocos* sizeof(int));
    for(i = 0; i < nblocos; i++){
		vetorDeBlocos[i] = 0; //setando todas os blocos como vazios
    }

    //inicializacao da lista de tabela de paginas
    tamanhoListaDeTabelas = 1;
    listaDeTabelas = (ListaDeTabelas*) malloc(tamanhoListaDeTabelas* sizeof(ListaDeTabelas));
}
//-gu


void pager_create(pid_t pid)
{
    int i, j, qtdPaginas;
    // Calcula o número de páginas dos vetores quadros e blocos.
    qtdPaginas = (UVM_MAXADDR - UVM_BASEADDR + 1) / sysconf(_SC_PAGESIZE);

    // Procurando tabela vazia na lista de tabelas.
    for (i = 0; i < tamanhoListaDeTabelas; i++){
        // Se a tabela estiver vazia.
        if (listaDeTabelas[i].tabela == NULL){
            listaDeTabelas[i].pid = pid;
            listaDeTabelas[i].tabela = malloc(sizeof(TabelaDePaginas));
            listaDeTabelas[i].tabela->qtdPaginas = qtdPaginas;
            listaDeTabelas[i].tabela->quadros = malloc(qtdPaginas* sizeof(int));
            listaDeTabelas[i].tabela->blocos = malloc(qtdPaginas* sizeof(int));

            // Seta os valores para -1 (convenção de vazio).
            for (j = 0; j < qtdPaginas; j++){
                listaDeTabelas[i].tabela->quadros[j] = -1;
                listaDeTabelas[i].tabela->blocos[j] = -1;
            }

            break;
        }
    }
}

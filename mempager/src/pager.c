#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
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
	int qtdPaginasBlocos; // Usado como tamanho dos dois vetores quadros e blocos.
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
		vetorDeBlocos[i] = 0; //setando todas os blocos como vazios (0 como conveção)
    }

    //inicializacao da lista de tabela de paginas
    tamanhoListaDeTabelas = 1;
    listaDeTabelas = (ListaDeTabelas*) malloc(tamanhoListaDeTabelas* sizeof(ListaDeTabelas));
}
//-gu

//ga-
void pager_create(pid_t pid){
    int i, j, qtdPaginas;

    // Calcula o número de páginas dos vetores frames e blocks.
    qtdPaginas = (UVM_MAXADDR - UVM_BASEADDR + 1) / sysconf(_SC_PAGESIZE);

    // Procurando tabela vazia na lista de tabelas.
    for(i = 0; i < tamanhoListaDeTabelas; i++){
        // Se a tabela estiver vazia.
        if (listaDeTabelas[i].tabela == NULL){
            listaDeTabelas[i].pid = pid;
            listaDeTabelas[i].tabela = malloc(sizeof(TabelaDePaginas));
            listaDeTabelas[i].tabela->qtdPaginasBlocos = qtdPaginas;
            listaDeTabelas[i].tabela->quadros = malloc(qtdPaginas* sizeof(int));
            listaDeTabelas[i].tabela->blocos = malloc(qtdPaginas* sizeof(int));

            // Seta os valores para -1 (convenção de vazio).
            for(j = 0; j < qtdPaginas; j++){
                listaDeTabelas[i].tabela->quadros[j] = -1;
                listaDeTabelas[i].tabela->blocos[j] = -1;
            }

            return;
        }
    }

    // Aumenta o tamanho da lista de tabelas.
    tamanhoListaDeTabelas += 100;
    listaDeTabelas = realloc(listaDeTabelas, tamanhoListaDeTabelas * sizeof(listaDeTabelas));

    // Aloca o novo processo na primeira posição vazia.
    listaDeTabelas[tamanhoListaDeTabelas - 100].pid = pid;
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela = malloc(sizeof(listaDeTabelas));
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->qtdPaginasBlocos = qtdPaginas;
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->quadros = malloc(qtdPaginas* sizeof(int));
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->blocos = malloc(qtdPaginas* sizeof(int));

	for (j = 0; j < qtdPaginas; j++){
        listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->quadros[j] = -1;
        listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->blocos[j] = -1;
    }

    // Define as tabelas subsequentes como vazias.
    for (j = tamanhoListaDeTabelas - 99; j < tamanhoListaDeTabelas; j++){
        listaDeTabelas[j].tabela = NULL;
    }
}
//-ga

//gu-
//aloca um quadro de memoria a um processo
void *pager_extend(pid_t pid){
    //se nao houverem mais blocos disponiveis, retorna NULL
    if(qtdBlocosLivres == 0){
        return NULL;
    }

    int i, j, posicaoBlocoAlocado;
    TabelaDePaginas *tabelaProcesso;

    //alocacao do bloco do disco
    for(i=0; i<tamanhoVetorBlocos; i++){
        if(vetorDeBlocos[i] == 0){
            vetorDeBlocos[i] = 1; //seta bloco para em uso
            qtdBlocosLivres--;
            posicaoBlocoAlocado = i;

            break;
        }
    }

    //busca tabela de paginas do processo
    for(i=0; i<tamanhoListaDeTabelas; i++){
        
        //tabela localizada
        if(listaDeTabelas[i].pid == pid){
            tabelaProcesso = listaDeTabelas[i].tabela;

            //busca bloco livre na tabela do processo
            for(j=0; j<tabelaProcesso->qtdPaginasBlocos; j++){

                //encontrou bloco livre na tabela do processo
                if(tabelaProcesso->blocos[j] == -1){

                    //referencia ao bloco alocado
                    tabelaProcesso->blocos[j] = posicaoBlocoAlocado;

                    break;
                }

                // se nao ha blocos livres na tabela do processo, retorna NULL
				if(j == (tabelaProcesso->qtdPaginasBlocos) - 1){
					return NULL;
                }
            }
            break;
        }
    }

    //retorna o endereço (inicio + posicao * tamanho da pagina)
	return (void*) (UVM_BASEADDR + (intptr_t) (j * sysconf(_SC_PAGESIZE)));

}
//-gu
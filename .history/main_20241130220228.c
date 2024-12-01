#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSOS 100
#define TEMPO_CICLO_CLOCK 10

typedef struct {
    int id;
    int tamanho;
    int tempo_chegada;
    int R;
    int M;
    int presente_na_memoria;
    int classe;
} Processo;

typedef struct {
    int inicio;
    int tamanho;
    int livre;
    Processo *processo;
} BlocoMemoria;

int tempo_atual = 0;

// Inicializa a memória com um único bloco livre
void inicializaMemoria(BlocoMemoria memoria[], int tamanho_memoria, int *num_blocos_memoria) {
    memoria[0].inicio = 0;
    memoria[0].tamanho = tamanho_memoria;
    memoria[0].livre = 1;
    memoria[0].processo = NULL;
    *num_blocos_memoria = 1;
}

// Insere um processo na fila de processos
void inserirProcessoNaFila(Processo fila[], int *qtd_processos, Processo processo) {
    fila[*qtd_processos] = processo;
    (*qtd_processos)++;
}

// Atualiza a classe do processo com base nos bits R e M
void atualizarClasse(Processo *processo) {
    processo->classe = (processo->R << 1) | processo->M;
}

// Compacta a memória após a remoção de um processo
void compactarMemoria(BlocoMemoria memoria[], int *num_blocos_memoria, int tamanho_memoria) {
    int posicao_atual = 0;
    BlocoMemoria memoria_compactada[MAX_PROCESSOS];
    int num_blocos_compactados = 0;

    // Compacta os processos para o início da memória
    for (int i = 0; i < *num_blocos_memoria; i++) {
        if (!memoria[i].livre && memoria[i].processo != NULL) {
            memoria_compactada[num_blocos_compactados] = memoria[i];
            memoria_compactada[num_blocos_compactados].inicio = posicao_atual;
            posicao_atual += memoria[i].tamanho;
            num_blocos_compactados++;
        }
    }

    // Adiciona um único bloco livre com o espaço restante
    memoria_compactada[num_blocos_compactados].inicio = posicao_atual;
    memoria_compactada[num_blocos_compactados].tamanho = tamanho_memoria - posicao_atual;
    memoria_compactada[num_blocos_compactados].livre = 1;
    memoria_compactada[num_blocos_compactados].processo = NULL;
    num_blocos_compactados++;

    // Atualiza a memória principal
    for (int i = 0; i < num_blocos_compactados; i++) {
        memoria[i] = memoria_compactada[i];
    }

    *num_blocos_memoria = num_blocos_compactados;
}

// Remove um processo da memória e compacta a memória
void removerProcessoDaMemoria(BlocoMemoria memoria[], int indice, int *num_blocos_memoria, int tamanho_memoria) {
    printf("%d     | processo %d| Removendo o processo %d da classe mais baixa (Classe %d: R=%d, M=%d)\n",
           tempo_atual, memoria[indice].processo->id, memoria[indice].processo->id,
           memoria[indice].processo->classe, memoria[indice].processo->R, memoria[indice].processo->M);
    memoria[indice].livre = 1;
    memoria[indice].processo->presente_na_memoria = 0;
    memoria[indice].processo = NULL;

    // Compacta a memória após remover o processo
    compactarMemoria(memoria, num_blocos_memoria, tamanho_memoria);
}

// Divide um bloco de memória se houver espaço suficiente para o processo
void dividirBloco(BlocoMemoria memoria[], int *num_blocos_memoria, int indice, int tamanho_processo) {
    if (memoria[indice].tamanho > tamanho_processo) {
        // Desloca os blocos para abrir espaço para o novo bloco
        for (int i = *num_blocos_memoria; i > indice + 1; i--) {
            memoria[i] = memoria[i - 1];
        }
        (*num_blocos_memoria)++;

        memoria[indice + 1].inicio = memoria[indice].inicio + tamanho_processo;
        memoria[indice + 1].tamanho = memoria[indice].tamanho - tamanho_processo;
        memoria[indice + 1].livre = 1;
        memoria[indice + 1].processo = NULL;

        memoria[indice].tamanho = tamanho_processo;
    }
}

// Aloca um processo usando o First Fit
int alocarProcessoFirstFit(BlocoMemoria memoria[], int *num_blocos_memoria, Processo *processo, int tamanho_memoria) {
    for (int i = 0; i < *num_blocos_memoria; i++) {
        if (memoria[i].livre && memoria[i].tamanho >= processo->tamanho) {
            dividirBloco(memoria, num_blocos_memoria, i, processo->tamanho);
            memoria[i].livre = 0;
            memoria[i].processo = processo;
            processo->presente_na_memoria = 1;
            atualizarClasse(processo);
            printf("%d     | processo %d| Processo %d foi inserido\n", tempo_atual, processo->id, processo->id);
            return 1;
        }
    }
    return 0;
}

// Aplica aleatoriedade para alterar um bit (R ou M) de um processo com 5% de chance
void aplicarAleatoriedade(Processo *processo) {
    int chance_aleatoria = rand() % 100;
    if (chance_aleatoria < 5) { // 5% de chance
        // Escolhe aleatoriamente qual bit alterar (0 para R, 1 para M)
        int bit_escolhido = rand() % 2;
        if (bit_escolhido == 0) {
            // Alterar bit R
            processo->R = processo->R ? 0 : 1;
            atualizarClasse(processo);
            printf("%d     | processo %d| Processo %d foi escolhido pela aleatoriedade e modificou seu bit R para %d, indo para classe %d\n",
                   tempo_atual, processo->id, processo->id, processo->R, processo->classe);
        } else {
            // Alterar bit M
            processo->M = processo->M ? 0 : 1;
            atualizarClasse(processo);
            printf("%d     | processo %d| Processo %d foi escolhido pela aleatoriedade e modificou seu bit M para %d, indo para classe %d\n",
                   tempo_atual, processo->id, processo->id, processo->M, processo->classe);
        }
    }
}

// Seleciona o processo de menor classe para remover (NUR)
Processo* selecionarProcessoParaRemover(BlocoMemoria memoria[], int num_blocos_memoria, int *indice_removido) {
    // Classes: 0 (R=0, M=0), 1 (R=0, M=1), 2 (R=1, M=0), 3 (R=1, M=1)
    for (int classe = 0; classe <= 3; classe++) {
        for (int i = 0; i < num_blocos_memoria; i++) {
            if (!memoria[i].livre && memoria[i].processo != NULL) {
                if (memoria[i].processo->classe == classe) {
                    *indice_removido = i;
                    return memoria[i].processo;
                }
            }
        }
    }
    return NULL;
}

// Função para realizar o ciclo de clock, invertendo todos os bits R e M de todos os processos
void cicloDeClock(BlocoMemoria memoria[], int num_blocos_memoria) {
    printf("%d     |           | Ciclo de clock ocorreu, bits R e M foram modificados de todos os processos\n", tempo_atual);
    for (int i = 0; i < num_blocos_memoria; i++) {
        if (!memoria[i].livre && memoria[i].processo != NULL) {
            Processo *processo = memoria[i].processo;
            // Inverte o bit R
            processo->R = processo->R ? 0 : 1;
            // Inverte o bit M
            processo->M = processo->M ? 0 : 1;
            // Atualiza a classe
            atualizarClasse(processo);
        }
    }
    // Exibe mensagem resumida após modificar os bits
    printf("%d     |           | Bits R e M foram invertidos para todos os processos na memória\n", tempo_atual);
}

int main() {
    srand(time(NULL));

    int tamanho_memoria;
    printf("Insira o tamanho da memoria: ");
    scanf("%d", &tamanho_memoria);

    BlocoMemoria memoria[MAX_PROCESSOS];
    int num_blocos_memoria;
    inicializaMemoria(memoria, tamanho_memoria, &num_blocos_memoria);

    Processo fila_processos[MAX_PROCESSOS];
    int qtd_processos = 0;

    int quantidade_processos;
    printf("Insira a quantidade de processos: ");
    scanf("%d", &quantidade_processos);

    for (int i = 0; i < quantidade_processos; i++) {
        Processo processo;
        processo.id = i + 1;
        printf("\nProcesso %d:\n", processo.id);
        printf("Tamanho: ");
        scanf("%d", &processo.tamanho);
        printf("Tempo que ele devera entrar: ");
        scanf("%d", &processo.tempo_chegada);
        processo.R = 1;
        processo.M = 1;
        processo.presente_na_memoria = 0;
        atualizarClasse(&processo);
        inserirProcessoNaFila(fila_processos, &qtd_processos, processo);
    }

    printf("\nTempo | Processo  | Acoes\n");

    int processos_inseridos = 0;
    int ultimo_processo_inserido = quantidade_processos;

    while (1) {
        // Chegada de processos
        for (int i = 0; i < qtd_processos; i++) {
            if (!fila_processos[i].presente_na_memoria && fila_processos[i].tempo_chegada == tempo_atual) {
                printf("%d     | processo %d| Processo %d deseja ser alocado\n", tempo_atual, fila_processos[i].id, fila_processos[i].id);
                int alocado = alocarProcessoFirstFit(memoria, &num_blocos_memoria, &fila_processos[i], tamanho_memoria);
                while (!alocado) {
                    printf("%d     | processo %d| Memoria cheia\n", tempo_atual, fila_processos[i].id);
                    printf("%d     | processo %d| Aplicando o NUR\n", tempo_atual, fila_processos[i].id);
                    int indice_removido = -1;
                    Processo *processo_removido = selecionarProcessoParaRemover(memoria, num_blocos_memoria, &indice_removido);
                    if (processo_removido == NULL) {
                        printf("%d     | processo %d| Nao foi possivel alocar o processo %d.\n", tempo_atual, fila_processos[i].id, fila_processos[i].id);
                        break;
                    }
                    removerProcessoDaMemoria(memoria, indice_removido, &num_blocos_memoria, tamanho_memoria);
                    alocado = alocarProcessoFirstFit(memoria, &num_blocos_memoria, &fila_processos[i], tamanho_memoria);
                }
                // Aplicar aleatoriedade imediatamente após a inserção
                aplicarAleatoriedade(&fila_processos[i]);

                processos_inseridos++;

                // Se foi o último processo a ser inserido, processar ciclo de clock (se for múltiplo de 10) e terminar
                if (processos_inseridos == ultimo_processo_inserido) {
                    // Verifica se o tempo atual é múltiplo de 10 para aplicar ciclo de clock
                    if (tempo_atual % TEMPO_CICLO_CLOCK == 0 && tempo_atual != 0) {
                        cicloDeClock(memoria, num_blocos_memoria);
                    }
                    printf("Todos os processos foram inseridos, simulação finalizada\n");
                    return 0;
                }
            }
        }

        // Aplicar ciclo de clock a cada 10 unidades de tempo
        if (tempo_atual % TEMPO_CICLO_CLOCK == 0 && tempo_atual != 0) {
            cicloDeClock(memoria, num_blocos_memoria);
        }

        // Aplicar aleatoriedade aos processos na memória a cada tempo
        for (int i = 0; i < num_blocos_memoria; i++) {
            if (!memoria[i].livre && memoria[i].processo != NULL) {
                aplicarAleatoriedade(memoria[i].processo);
            }
        }

        tempo_atual++;

        // Limite de tempo para evitar loop infinito (caso haja algum erro)
        if (tempo_atual > 1000) {
            printf("Simulacao encerrada devido ao limite de tempo.\n");
            break;
        }
    }

    return 0;
}

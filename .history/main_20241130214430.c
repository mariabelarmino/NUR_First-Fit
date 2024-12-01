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

void inicializaMemoria(BlocoMemoria memoria[], int tamanho_memoria, int *num_blocos_memoria) {
    memoria[0].inicio = 0;
    memoria[0].tamanho = tamanho_memoria;
    memoria[0].livre = 1;
    memoria[0].processo = NULL;
    *num_blocos_memoria = 1;
}

void inserirProcessoNaFila(Processo fila[], int *qtd_processos, Processo processo) {
    fila[*qtd_processos] = processo;
    (*qtd_processos)++;
}

void atualizarClasse(Processo *processo) {
    processo->classe = (processo->R << 1) | processo->M;
}

void compactarMemoria(BlocoMemoria memoria[], int *num_blocos_memoria) {
    int posicao_atual = 0;
    for (int i = 0; i < *num_blocos_memoria; i++) {
        if (!memoria[i].livre) {
            if (i != posicao_atual) {
                // Move o bloco para a posição atual
                memoria[posicao_atual] = memoria[i];
                memoria[posicao_atual].inicio = (posicao_atual == 0) ? 0 : memoria[posicao_atual - 1].inicio + memoria[posicao_atual - 1].tamanho;
            }
            posicao_atual++;
        }
    }

    // Atualiza o número de blocos
    *num_blocos_memoria = posicao_atual + 1;

    // Cria um único bloco livre com o restante da memória
    memoria[posicao_atual].inicio = (posicao_atual == 0) ? 0 : memoria[posicao_atual - 1].inicio + memoria[posicao_atual - 1].tamanho;
    memoria[posicao_atual].tamanho = memoria[0].tamanho + memoria[memoria[0].inicio].tamanho - memoria[posicao_atual].inicio;
    memoria[posicao_atual].livre = 1;
    memoria[posicao_atual].processo = NULL;
}

void removerProcessoDaMemoria(BlocoMemoria memoria[], int indice, int *num_blocos_memoria) {
    printf("%d     | processo %d| Removendo o processo %d da classe mais baixa (Classe %d: R=%d, M=%d)\n",
           tempo_atual, memoria[indice].processo->id, memoria[indice].processo->id,
           memoria[indice].processo->classe, memoria[indice].processo->R, memoria[indice].processo->M);
    memoria[indice].livre = 1;
    memoria[indice].processo->presente_na_memoria = 0;
    memoria[indice].processo = NULL;

    // Compacta a memória após remover o processo
    compactarMemoria(memoria, num_blocos_memoria);
}

void dividirBloco(BlocoMemoria memoria[], int *num_blocos_memoria, int indice, int tamanho_processo) {
    if (memoria[indice].tamanho > tamanho_processo) {
        // Deslocar os blocos para abrir espaço para o novo bloco
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

int alocarProcessoFirstFit(BlocoMemoria memoria[], int *num_blocos_memoria, Processo *processo) {
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

void aplicarAleatoriedade(Processo *processo) {
    int chance_aleatoria = rand() % 100;
    if (chance_aleatoria < 5) {
        // Escolher aleatoriamente qual bit alterar (0 para R, 1 para M)
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

void cicloDeClock(BlocoMemoria memoria[], int num_blocos_memoria) {
    printf("%d     |           | Ciclo de clock ocorreu\n", tempo_atual);
    for (int i = 0; i < num_blocos_memoria; i++) {
        if (!memoria[i].livre && memoria[i].processo != NULL) {
            Processo *processo = memoria[i].processo;

            if (processo->R == 1) {
                processo->R = 0;
                atualizarClasse(processo);
                printf("%d     | processo %d| Bit R alterado para 0 pelo ciclo de clock, classe atual %d\n",
                       tempo_atual, processo->id, processo->classe);
            }
        }
    }
}

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

    int ciclos_sem_atividade = 0;

    while (1) {
        // Chegada de processos
        int processos_restantes = 0;
        int atividade_no_ciclo = 0;

        for (int i = 0; i < qtd_processos; i++) {
            if (!fila_processos[i].presente_na_memoria) {
                processos_restantes++;
                if (fila_processos[i].tempo_chegada == tempo_atual) {
                    printf("%d     | processo %d| Processo %d deseja ser alocado\n", tempo_atual, fila_processos[i].id, fila_processos[i].id);
                    int alocado = alocarProcessoFirstFit(memoria, &num_blocos_memoria, &fila_processos[i]);
                    while (!alocado) {
                        printf("%d     | processo %d| Memoria cheia\n", tempo_atual, fila_processos[i].id);
                        printf("%d     | processo %d| Aplicando o NUR\n", tempo_atual, fila_processos[i].id);
                        int indice_removido = -1;
                        Processo *processo_removido = selecionarProcessoParaRemover(memoria, num_blocos_memoria, &indice_removido);
                        if (processo_removido == NULL) {
                            printf("%d     | processo %d| Nao foi possivel alocar o processo %d.\n", tempo_atual, fila_processos[i].id, fila_processos[i].id);
                            break;
                        }
                        removerProcessoDaMemoria(memoria, indice_removido, &num_blocos_memoria);
                        alocado = alocarProcessoFirstFit(memoria, &num_blocos_memoria, &fila_processos[i]);
                    }
                    // Aplicar aleatoriedade imediatamente após a inserção
                    aplicarAleatoriedade(&fila_processos[i]);
                    atividade_no_ciclo = 1;
                }
            }
        }

        // Aplicar ciclo de clock a cada 10 unidades de tempo
        if (tempo_atual % TEMPO_CICLO_CLOCK == 0 && tempo_atual != 0) {
            cicloDeClock(memoria, num_blocos_memoria);
            atividade_no_ciclo = 1;
        }

        // Aplicar aleatoriedade aos processos na memória
        for (int i = 0; i < num_blocos_memoria; i++) {
            if (!memoria[i].livre && memoria[i].processo != NULL) {
                aplicarAleatoriedade(memoria[i].processo);
            }
        }

        // Verificar se houve atividade no ciclo
        if (!atividade_no_ciclo) {
            ciclos_sem_atividade++;
        } else {
            ciclos_sem_atividade = 0;
        }

        tempo_atual++;

        // Verificar se todos os processos já foram inseridos e se não há mais atividade
        if (processos_restantes == 0 && ciclos_sem_atividade >= TEMPO_CICLO_CLOCK) {
            printf("Simulação finalizada após inatividade.\n");
            break;
        }

        // Limite de tempo para evitar loop infinito (caso haja algum erro)
        if (tempo_atual > 1000) {
            printf("Simulacao encerrada devido ao limite de tempo.\n");
            break;
        }
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSOS 100
#define TEMPO_CICLO_CLOCK 10

// Estrutura que representa um processo
typedef struct {
    int id;                     
    int tamanho;                
    int tempo_chegada;          // tempo que o processo foi alocado na memória
    int R;                      
    int M;                      
    int presente_na_memoria;    // indica se o processo ta na memória
    int classe;          
} Processo;

// Estrutura que representa um bloco de memória
typedef struct {
    int inicio;                 // endereço inicial do bloco 
    int tamanho;                // tamanho do bloco
    int livre;                  // variavel q armazena 0 ou 1 para verificar se o bloco está livre ou não 
    Processo *processo;         // ponteiro para o processo alocado no bloco 
} BlocoMemoria;

// variavel para ver o tempo onde está a simulação 
int tempo_atual = 0;

// função para inicializar a memória com um único bloco livre
void inicializaMemoria(BlocoMemoria memoria[], int tamanho_memoria, int *num_blocos_memoria) {
    memoria[0].inicio = 0;                    // Início da memória
    memoria[0].tamanho = tamanho_memoria;     // tamanho total da memória 
    memoria[0].livre = 1;                      // no inicio, todo bloco tá livre 
    memoria[0].processo = NULL;                // nenhum processo alocado 
    *num_blocos_memoria = 1;                   // apenas um bloco de memória inicializado
}

// função para inserir um processo na fila de processos
void inserirProcessoNaFila(Processo fila[], int *qtd_processos, Processo processo) {
    fila[*qtd_processos] = processo;  // adiciona o processo no final da fila
    (*qtd_processos)++;              
}

// função para atualizar a classe (bit R e M)
void atualizarClasse(Processo *processo) {
    processo->classe = (processo->R << 1) | processo->M; // calcula a classe: (R * 2) + M
}

// Função para reorganizar a memória após a remoção de um processo
void compactarMemoria(BlocoMemoria memoria[], int *num_blocos_memoria, int tamanho_memoria) {
    int posicao_atual = 0;                     
    BlocoMemoria memoria_compactada[MAX_PROCESSOS];          // vetor temporário para memória organizada
    int num_blocos_compactados = 0;                           // contador de blocos organizados

 
    for (int i = 0; i < *num_blocos_memoria; i++) {
        // se o bloco não está livre e possui um processo
        if (!memoria[i].livre && memoria[i].processo != NULL) {
            memoria_compactada[num_blocos_compactados] = memoria[i];      // Copia o bloco para o vetor organizado
            memoria_compactada[num_blocos_compactados].inicio = posicao_atual; // atualiza o início do bloco
            posicao_atual += memoria[i].tamanho;                           // avança a posição atual
            num_blocos_compactados++;                                      
        }
    }

    // adiciona um único bloco livre com o espaço restante da memória
    memoria_compactada[num_blocos_compactados].inicio = posicao_atual;
    memoria_compactada[num_blocos_compactados].tamanho = tamanho_memoria - posicao_atual;
    memoria_compactada[num_blocos_compactados].livre = 1;
    memoria_compactada[num_blocos_compactados].processo = NULL;
    num_blocos_compactados++;

    // atualiza a memória principal com os blocos atualizados
    for (int i = 0; i < num_blocos_compactados; i++) {
        memoria[i] = memoria_compactada[i];
    }

    *num_blocos_memoria = num_blocos_compactados; // atualiza o número de blocos de memória
}

// Função para remover um processo da memória e atualizar a memória
void removerProcessoDaMemoria(BlocoMemoria memoria[], int indice, int *num_blocos_memoria, int tamanho_memoria) {
    // Imprime informações sobre a remoção do processo
    printf("%d     | processo %d| Removendo o processo %d da classe mais baixa (Classe %d: R=%d, M=%d)\n",
           tempo_atual, memoria[indice].processo->id, memoria[indice].processo->id,
           memoria[indice].processo->classe, memoria[indice].processo->R, memoria[indice].processo->M);
    
    memoria[indice].livre = 1;                        // marca o bloco como livre
    memoria[indice].processo->presente_na_memoria = 0; // Atualiza a variavel do processo
    memoria[indice].processo = NULL;                 

    // atualiza a memória após remover o processo
    compactarMemoria(memoria, num_blocos_memoria, tamanho_memoria);
}

// função para dividir um bloco de memória se houver espaço suficiente para o processo
void dividirBloco(BlocoMemoria memoria[], int *num_blocos_memoria, int indice, int tamanho_processo) {
    // verifica se o bloco tem espaço extra para dividir
    if (memoria[indice].tamanho > tamanho_processo) {
        // desloca os blocos para abrir espaço para o novo bloco
        for (int i = *num_blocos_memoria; i > indice + 1; i--) {
            memoria[i] = memoria[i - 1];
        }
        (*num_blocos_memoria)++; 

        // configura o novo bloco livre com o espaço restante
        memoria[indice + 1].inicio = memoria[indice].inicio + tamanho_processo;
        memoria[indice + 1].tamanho = memoria[indice].tamanho - tamanho_processo;
        memoria[indice + 1].livre = 1;
        memoria[indice + 1].processo = NULL;

        memoria[indice].tamanho = tamanho_processo; // atualiza o tamanho do bloco original
    }
}

// função para alocar um processo usando a estratégia First Fit
int alocarProcessoFirstFit(BlocoMemoria memoria[], int *num_blocos_memoria, Processo *processo, int tamanho_memoria) {
    // percorre todos os blocos de memória
    for (int i = 0; i < *num_blocos_memoria; i++) {
        // se o bloco está livre e é grande o suficiente para o processo
        if (memoria[i].livre && memoria[i].tamanho >= processo->tamanho) {
            dividirBloco(memoria, num_blocos_memoria, i, processo->tamanho); // divide o bloco se necessário
            memoria[i].livre = 0;                                        // marca o bloco como ocupado
            memoria[i].processo = processo;                              // aloca o processo no bloco
            processo->presente_na_memoria = 1;                          // atualiza a variavel do processo
            atualizarClasse(processo);                                   // atualiza a classe do processo
            printf("%d     | processo %d| Processo %d foi inserido\n", tempo_atual, processo->id, processo->id);
            return 1; // alocação feita
        }
    }
    return 0; // falha na alocação
}

// função para aplicar aleatoriedade e alterar um bit (R ou M) de um processo com 5% de chance
void aplicarAleatoriedade(Processo *processo) {
    int chance_aleatoria = rand() % 100; // Gera um número aleatório entre 0 e 99
    if (chance_aleatoria < 5) { // 5% de chance
        // Escolhe aleatoriamente qual bit alterar (0 para R, 1 para M)
        int bit_escolhido = rand() % 2;
        if (bit_escolhido == 0) {
            // Alterar bit R
            processo->R = processo->R ? 0 : 1; // inverte o bit R
            atualizarClasse(processo);          // atualiza a classe do processo
            printf("%d     | processo %d| Processo %d foi escolhido pela aleatoriedade e modificou seu bit R para %d, indo para classe %d\n",
                   tempo_atual, processo->id, processo->id, processo->R, processo->classe);
        } else {
            // alterar bit M
            processo->M = processo->M ? 0 : 1; // inverte o bit M
            atualizarClasse(processo);          // atualiza a classe do processo
            printf("%d     | processo %d| Processo %d foi escolhido pela aleatoriedade e modificou seu bit M para %d, indo para classe %d\n",
                   tempo_atual, processo->id, processo->id, processo->M, processo->classe);
        }
    }
}

// função para selecionar o processo de menor classe para remover (algoritmo NUR)
Processo* selecionarProcessoParaRemover(BlocoMemoria memoria[], int num_blocos_memoria, int *indice_removido) {
    for (int classe = 0; classe <= 3; classe++) { // percorre as classes 
        for (int i = 0; i < num_blocos_memoria; i++) { // percorre todos os blocos de memória
            if (!memoria[i].livre && memoria[i].processo != NULL) { // se o bloco está ocupado
                if (memoria[i].processo->classe == classe) { // se a classe do processo corresponde
                    *indice_removido = i; // armazena o índice do bloco a ser removido
                    return memoria[i].processo; // retorna o processo a ser removido
                }
            }
        }
    }
    return NULL; // nenhum processo encontrado para remoção
}

// Função para realizar o ciclo de clock, invertendo todos os bits R e M de todos os processos
void cicloDeClock(BlocoMemoria memoria[], int num_blocos_memoria) {
    printf("%d     |           | Ciclo de clock ocorreu, bits R e M foram modificados de todos os processos\n", tempo_atual);
    for (int i = 0; i < num_blocos_memoria; i++) { // percorre todos os blocos de memória
        if (!memoria[i].livre && memoria[i].processo != NULL) { // se o bloco está ocupado
            Processo *processo = memoria[i].processo;
            // inverte o bit R
            processo->R = processo->R ? 0 : 1;
            // inverte o bit M
            processo->M = processo->M ? 0 : 1;
            // atualiza a classe do processo
            atualizarClasse(processo);
        }
    }
    // Exibe mensagem resumida após modificar os bits
    printf("%d     |           | Bits R e M foram invertidos para todos os processos na memória\n", tempo_atual);
}

int main() {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios com a semente baseada no tempo atual

    int tamanho_memoria;
    printf("Insira o tamanho da memoria: ");
    scanf("%d", &tamanho_memoria); // Recebe o tamanho total da memória

    BlocoMemoria memoria[MAX_PROCESSOS]; // Array para armazenar os blocos de memória
    int num_blocos_memoria;
    inicializaMemoria(memoria, tamanho_memoria, &num_blocos_memoria); // Inicializa a memória

    Processo fila_processos[MAX_PROCESSOS]; // Array para armazenar a fila de processos
    int qtd_processos = 0;                  // Contador de processos na fila

    int quantidade_processos;
    printf("Insira a quantidade de processos: ");
    scanf("%d", &quantidade_processos); // Recebe a quantidade total de processos a serem simulados

    // Loop para inserir os processos na fila
    for (int i = 0; i < quantidade_processos; i++) {
        Processo processo;
        processo.id = i + 1; // Atribui um ID único ao processo
        printf("\nProcesso %d:\n", processo.id);
        printf("Tamanho: ");
        scanf("%d", &processo.tamanho);           // Recebe o tamanho do processo
        printf("Tempo que ele devera entrar: ");
        scanf("%d", &processo.tempo_chegada);     // Recebe o tempo de chegada do processo
        processo.R = 1;                           // Inicializa o bit R como 1
        processo.M = 1;                           // Inicializa o bit M como 1
        processo.presente_na_memoria = 0;         // Inicialmente, o processo não está na memória
        atualizarClasse(&processo);               // Atualiza a classe do processo com base nos bits R e M
        inserirProcessoNaFila(fila_processos, &qtd_processos, processo); // Insere o processo na fila
    }

    // Imprime o cabeçalho da tabela de ações
    printf("\nTempo | Processo  | Acoes\n");

    int processos_inseridos = 0;           // Contador de processos que foram inseridos na memória
    int ultimo_processo_inserido = quantidade_processos; // ID do último processo a ser inserido

    // Loop principal da simulação
    while (1) {
        // Verifica a chegada de processos na memória no tempo atual
        for (int i = 0; i < qtd_processos; i++) {
            // Se o processo ainda não está na memória e o tempo de chegada coincide com o tempo atual
            if (!fila_processos[i].presente_na_memoria && fila_processos[i].tempo_chegada == tempo_atual) {
                printf("%d     | processo %d| Processo %d deseja ser alocado\n", tempo_atual, fila_processos[i].id, fila_processos[i].id);
                int alocado = alocarProcessoFirstFit(memoria, &num_blocos_memoria, &fila_processos[i], tamanho_memoria); // Tenta alocar o processo

                // Enquanto não conseguir alocar o processo
                while (!alocado) {
                    printf("%d     | processo %d| Memoria cheia\n", tempo_atual, fila_processos[i].id);
                    printf("%d     | processo %d| Aplicando o NUR\n", tempo_atual, fila_processos[i].id);
                    int indice_removido = -1;
                    // Seleciona o processo a ser removido usando o algoritmo NUR
                    Processo *processo_removido = selecionarProcessoParaRemover(memoria, num_blocos_memoria, &indice_removido);
                    if (processo_removido == NULL) {
                        // Se não for possível remover nenhum processo, aborta a alocação
                        printf("%d     | processo %d| Nao foi possivel alocar o processo %d.\n", tempo_atual, fila_processos[i].id, fila_processos[i].id);
                        break;
                    }
                    removerProcessoDaMemoria(memoria, indice_removido, &num_blocos_memoria, tamanho_memoria); // Remove o processo selecionado
                    alocado = alocarProcessoFirstFit(memoria, &num_blocos_memoria, &fila_processos[i], tamanho_memoria); // Tenta alocar novamente
                }
                // Aplica aleatoriedade imediatamente após a inserção do processo
                aplicarAleatoriedade(&fila_processos[i]);

                processos_inseridos++; // Incrementa o contador de processos inseridos

                // Verifica se todos os processos já foram inseridos
                if (processos_inseridos == ultimo_processo_inserido) {
                    // Verifica se o tempo atual é múltiplo de 10 para aplicar o ciclo de clock
                    if (tempo_atual % TEMPO_CICLO_CLOCK == 0 && tempo_atual != 0) {
                        cicloDeClock(memoria, num_blocos_memoria); // Executa o ciclo de clock
                    }
                    printf("Todos os processos foram inseridos, simulação finalizada\n");
                    return 0; // Finaliza a simulação
                }
            }
        }

        // Aplica o ciclo de clock a cada 10 unidades de tempo
        if (tempo_atual % TEMPO_CICLO_CLOCK == 0 && tempo_atual != 0) {
            cicloDeClock(memoria, num_blocos_memoria); // Executa o ciclo de clock
        }

        // Aplica aleatoriedade aos processos na memória a cada unidade de tempo
        for (int i = 0; i < num_blocos_memoria; i++) {
            if (!memoria[i].livre && memoria[i].processo != NULL) { // Se o bloco está ocupado
                aplicarAleatoriedade(memoria[i].processo); // Aplica a função de aleatoriedade no processo
            }
        }

        tempo_atual++; // Incrementa o tempo atual

        // Limite de tempo para evitar loop infinito (caso haja algum erro na lógica)
        if (tempo_atual > 1000) {
            printf("Simulacao encerrada devido ao limite de tempo.\n");
            break; // Encerra a simulação
        }
    }

    return 0; // Finaliza o programa
}

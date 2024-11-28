#include <iostream>
#include <vector>
using namespace std;

// Função para realizar a alocação de memória usando o algoritmo First Fit
void firstFit(vector<int>& blockSizes, vector<int>& processSizes) {
    int nBlocks = blockSizes.size();
    int nProcesses = processSizes.size();

    // Vetor para armazenar o índice do bloco alocado para cada processo
    vector<int> allocation(nProcesses, -1);

    // Para cada processo, tente encontrar um bloco adequado
    for (int i = 0; i < nProcesses; i++) {
        for (int j = 0; j < nBlocks; j++) {
            if (blockSizes[j] >= processSizes[i]) {
                // Alocar o bloco ao processo
                allocation[i] = j;
                blockSizes[j] -= processSizes[i];
                break; // Sair do loop interno após encontrar o primeiro bloco adequado
            }
        }
    }

    // Exibir a alocação final
    cout << "Processo\tTamanho\tBloco Alocado\n";
    for (int i = 0; i < nProcesses; i++) {
        cout << "P" << i + 1 << "\t\t" << processSizes[i] << "\t\t";
        if (allocation[i] != -1)
            cout << allocation[i] + 1; // Exibindo índices 1-based para clareza
        else
            cout << "Não Alocado";
        cout << endl;
    }
}

int main() {
    // Tamanhos dos blocos de memória disponíveis
    vector<int> blockSizes = {100, 500, 200, 300, 600};

    // Tamanhos dos processos a serem alocados
    vector<int> processSizes = {212, 417, 112, 426};

    // Executa o algoritmo First Fit
    firstFit(blockSizes, processSizes);

    return 0;
}

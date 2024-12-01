#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

// Estrutura do Processo
struct Processo {
    int id;
    int tamanho;
    bool R;
    bool M;
    int tempo_chegada;
    int tempoUltimaAtualizacaoR;
    int tempoUltimaAtualizacaoM;
};

// Estrutura do Bloco de Memória
struct BlocoMemoria {
    int inicio;
    int tamanho;
    bool livre;
    Processo* processo; // Ponteiro para o processo alocado (se houver)
};

// Função para comparar blocos de memória por endereço inicial
bool compararBlocos(const BlocoMemoria& a, const BlocoMemoria& b) {
    return a.inicio < b.inicio;
}

// Classe que simula a memória e gerencia os processos
class Memoria {
private:
    int tamanhoTotal;
    list<BlocoMemoria> blocos;
    vector<Processo*> processos_na_memoria;
    vector<Processo> processos_definidos; // Todos os processos definidos
    queue<Processo*> fila_memoria_virtual; // Processos aguardando para entrar na memória
    int tempo;

public:
    Memoria(int tamanho) : tamanhoTotal(tamanho), tempo(0) {
        BlocoMemoria blocoInicial = {0, tamanhoTotal, true, nullptr};
        blocos.push_back(blocoInicial);
        srand(time(0)); // Inicializa a semente para números aleatórios
    }

    void adicionarProcesso(Processo p) {
        processos_definidos.push_back(p);
        fila_memoria_virtual.push(&processos_definidos.back()); // Adiciona o processo à fila de memória virtual
    }

    // Alocação First-Fit
    bool alocarProcesso(Processo* processo) {
        for (auto it = blocos.begin(); it != blocos.end(); ++it) {
            if (it->livre && it->tamanho >= processo->tamanho) {
                // Dividir o bloco se houver espaço sobrando
                if (it->tamanho > processo->tamanho) {
                    BlocoMemoria novo_bloco = {
                        it->inicio + processo->tamanho,
                        it->tamanho - processo->tamanho,
                        true,
                        nullptr};
                    it->tamanho = processo->tamanho;
                    blocos.insert(next(it), novo_bloco);
                }
                it->livre = false;
                it->processo = processo;
                processos_na_memoria.push_back(processo);
                return true;
            }
        }
        // Não há espaço disponível
        return false;
    }

    void removerProcesso(Processo* processo) {
        for (auto it = blocos.begin(); it != blocos.end(); ++it) {
            if (!it->livre && it->processo == processo) {
                it->livre = true;
                it->processo = nullptr;
                // Remove o processo da lista de processos na memória
                processos_na_memoria.erase(remove(processos_na_memoria.begin(), processos_na_memoria.end(), processo), processos_na_memoria.end());
                // Mesclar blocos livres adjacentes
                fundirBlocosLivres();
                break;
            }
        }
    }

    void fundirBlocosLivres() {
        for (auto it = blocos.begin(); it != blocos.end(); ) {
            auto next_it = next(it);
            if (next_it != blocos.end() && it->livre && next_it->livre) {
                it->tamanho += next_it->tamanho;
                blocos.erase(next_it);
            } else {
                ++it;
            }
        }
    }

    // Aplicar o algoritmo NUR para liberar espaço
    void aplicarNUR(int tamanhoNecessario) {
        // Classificar processos em quatro classes com base nos bits R e M
        vector<Processo*> classe0, classe1, classe2, classe3;
        for (Processo* proc : processos_na_memoria) {
            if (!proc->R && !proc->M)
                classe0.push_back(proc);
            else if (!proc->R && proc->M)
                classe1.push_back(proc);
            else if (proc->R && !proc->M)
                classe2.push_back(proc);
            else
                classe3.push_back(proc);
        }

        // Tentar remover processos da classe mais baixa primeiro
        vector<Processo*> *classes[] = {&classe0, &classe1, &classe2, &classe3};

        for (auto cls : classes) {
            while (!cls->empty()) {
                Processo* proc_para_remover = cls->front();
                cls->erase(cls->begin());
                removerProcesso(proc_para_remover);
                cout << "Processo P" << proc_para_remover->id << " removido da memória (Classe " << obterClasse(proc_para_remover) << ") para liberar espaço.\n";
                if (memoriaLivre() >= tamanhoNecessario)
                    return;
            }
        }
        cout << "Não foi possível liberar memória suficiente para alocar o processo.\n";
    }

    int obterClasse(Processo* p) {
        if (!p->R && !p->M) return 0;
        if (!p->R && p->M) return 1;
        if (p->R && !p->M) return 2;
        if (p->R && p->M) return 3;
        return -1;
    }

    int memoriaLivre() {
        int memoria_livre = 0;
        for (auto& bloco : blocos) {
            if (bloco.livre)
                memoria_livre += bloco.tamanho;
        }
        return memoria_livre;
    }

    void simularUnidadeDeTempo() {
        // Verificar processos que chegam neste tempo
        while (!fila_memoria_virtual.empty() && fila_memoria_virtual.front()->tempo_chegada == tempo) {
            Processo* p = fila_memoria_virtual.front();
            fila_memoria_virtual.pop();
            cout << "Tempo " << tempo << ": Processo P" << p->id << " chegou e tenta ser alocado.\n";
            bool alocado = alocarProcesso(p);
            if (!alocado) {
                cout << "Não há espaço disponível. Aplicando o algoritmo NUR.\n";
                aplicarNUR(p->tamanho);
                alocado = alocarProcesso(p);
                if (alocado)
                    cout << "Processo P" << p->id << " alocado após aplicar o NUR.\n";
                else
                    cout << "Falha ao alocar o processo P" << p->id << " mesmo após aplicar o NUR.\n";
            } else {
                cout << "Processo P" << p->id << " alocado com sucesso.\n";
            }
        }

        // Atualizar aleatoriamente os bits R e M dos processos na memória
        for (Processo* p : processos_na_memoria) {
            // Chance de ser referenciado
            if (rand() % 2 == 0) {
                p->R = true;
                p->tempoUltimaAtualizacaoR = tempo;
            }
            // Chance de ser modificado
            if (rand() % 2 == 0) {
                p->M = true;
                p->tempoUltimaAtualizacaoM = tempo;
            }
        }

        // A cada 10 unidades de tempo, atualizar os bits R e M
        if ((tempo > 0) && (tempo % 10 == 0)) {
            cout << "Tempo " << tempo << ": Ciclo de clock ocorre. Atualizando bits R e M.\n";
            for (Processo* p : processos_na_memoria) {
                if (p->R == 1) {
                    p->R = 0;
                }
                if (p->M == 1 && (tempo - p->tempoUltimaAtualizacaoM) >= 10) {
                    p->M = 0;
                }
            }
        }

        tempo++;
    }

    void simular(int tempo_maximo) {
        while (tempo <= tempo_maximo || !fila_memoria_virtual.empty()) {
            simularUnidadeDeTempo();
            // Opcional: exibir o estado da memória a cada unidade de tempo
            // exibirMemoria();
        }
    }

    void exibirMemoria() {
        cout << "\nEstado da memória no tempo " << tempo << ":\n";
        for (auto& bloco : blocos) {
            cout << "Endereço " << bloco.inicio << " - ";
            if (bloco.livre)
                cout << "Livre (" << bloco.tamanho << " unidades)\n";
            else {
                Processo* p = bloco.processo;
                cout << "P" << p->id << " (" << bloco.tamanho << " unidades), R" << p->R << "M" << p->M << "\n";
            }
        }
        cout << "Memória livre total: " << memoriaLivre() << " unidades\n";
    }

    void exibirProcessosNaMemoria() {
        cout << "\nProcessos na memória:\n";
        for (Processo* p : processos_na_memoria) {
            cout << "P" << p->id << ": Tamanho=" << p->tamanho << ", R=" << p->R << ", M=" << p->M << "\n";
        }
    }
};

int main() {
    int tamanho_memoria_total;
    cout << "Informe o tamanho total da memória: ";
    cin >> tamanho_memoria_total;
    Memoria memoria(tamanho_memoria_total);

    int num_processos;
    cout << "Informe o número de processos: ";
    cin >> num_processos;

    for (int i = 0; i < num_processos; ++i) {
        Processo p;
        p.id = i+1;
        cout << "Processo P" << p.id << ":\n";
        cout << "Informe o tamanho: ";
        cin >> p.tamanho;
        cout << "Informe o tempo de chegada: ";
        cin >> p.tempo_chegada;
        p.R = true;
        p.M = true;
        p.tempoUltimaAtualizacaoR = 0;
        p.tempoUltimaAtualizacaoM = 0;
        memoria.adicionarProcesso(p);
    }

    int tempo_maximo_simulacao;
    cout << "Informe o tempo máximo de simulação: ";
    cin >> tempo_maximo_simulacao;

    memoria.simular(tempo_maximo_simulacao);

    cout << "Simulação concluída.\n";
    memoria.exibirMemoria();

    return 0;
}

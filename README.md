# Algoritmos de Substituição de Página - Técnica NUR

Este projeto é uma implementação da técnica de substituição de página **"Não Usada Recentemente (NUR)"** e da alocação de memória com a técnica **First-Fit**. Ele foi desenvolvido para a disciplina **Sistemas Operacionais** na Universidade Federal de Alfenas (UNIFAL-MG).

## Descrição
A implementação realiza a simulação de gerenciamento de memória dinâmica utilizando as seguintes regras:
- **Bits R e M**: 
  - Todos os processos têm os bits `R` e `M` inicializados como `1`.
  - A cada **10 unidades de tempo**, ocorre um ciclo de clock:
    - O bit `R` de cada processo é atualizado para `0`.
    - O bit `M` de cada processo é atualizado para `0` caso tenham passado **10 unidades de tempo desde sua última modificação**.
- **Alocação de Memória**: 
  - A inserção de novos processos utiliza a técnica **First-Fit** (outra técnica pode ser justificada no documento explicativo).
  - O tamanho da memória e dos processos é configurado em tempo de execução.

### Exemplos de Atualização dos Bits:
1. **Instante 0**: Processo `P1` entra na memória com bits `R=1, M=1`.
2. **Instante 10**: Ciclo de clock:
   - `P1`: `R=0, M=0` (clock e 10 unidades desde a última modificação).
   - Outros processos seguem as mesmas regras.
3. **Instante 13**: `P2`: `M=0` (10 unidades desde a última modificação).

## Tecnologias Utilizadas
- Linguagem de Programação: **C++** 
- Bibliotecas padrão da linguagem.

## Como Executar
1. 

## Decisões de Implementação
- **Entrada de Dados**:
  - O tamanho da memória e dos processos é fornecido pelo usuário em tempo de execução.
- **Alocação**:
  - A técnica de **First-Fit** foi escolhida por sua simplicidade e eficiência para casos básicos de gerenciamento de memória.
- **Simulação de Clock**:
  - A cada 10 unidades de tempo, ocorre uma verificação global para atualizar os bits `R` e `M`.

## Documento Explicativo
Um documento explicativo está incluído, descrevendo detalhadamente o funcionamento da implementação e as decisões tomadas.

## Observações
- Este projeto segue as regras da disciplina **Sistemas Operacionais** da UNIFAL-MG.

--

**Universidade Federal de Alfenas (UNIFAL-MG)**  
**Disciplina**: Sistemas Operacionais  
**Docente**: Fellipe Guilherme Rey de Souza
**Trabalho**: Algoritmos de Substituição de Página - Técnica NUR  
**Desenvolvedores**: Maria Luiza Alves Belarmino / Nycole Paulino Santos

--
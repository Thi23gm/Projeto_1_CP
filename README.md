# K-Means Paralelizado com OpenMP

- Thiago Gomes 
- Matheus Eduardo

## Descrição

Este projeto implementa o algoritmo K-Means em C, com paralelização utilizando OpenMP. O objetivo é agrupar um grande conjunto de dados em k clusters, acelerando o processo através do uso de múltiplos threads.

O algoritmo foi testado com uma base de **1.000.000** de registros e **4** features, para garantir que a execução sequencial demandasse no mínimo 10 segundos, conforme os requisitos.

## Requisitos

- Compilador C compatível com C99 ou superior.
- OpenMP instalado (para suporte à paralelização).
- Dataset real ou sintético no formato adequado.

## Arquivos

- `main.c`: Código-fonte principal com a implementação do K-Means paralelizado.
- `generate_data.c`: Código-fonte para gerar um dataset sintético.
- `data.txt`: Arquivo de dados contendo o dataset utilizado.
- `readme.txt`: Este arquivo com instruções e resultados.

## Compilação

### Compilando o Gerador de Dados

Para gerar um dataset sintético com 1.000.000 de registros e 4 features:

```bash
gcc -o generate_data generate_data.c
```

### Gerando o Dataset

```bash
./generate_data
```

**Nota**: Certifique-se de que o arquivo `data.txt` foi gerado com sucesso.

### Compilando o Programa Principal

#### Versão Sequencial

Para compilar a versão sequencial (sem paralelismo):

```bash
gcc -o kmeans main.c -std=c99 -lm
```

#### Versão Paralela

Para compilar a versão paralela com suporte a OpenMP:

```bash
gcc -o kmeans_parallel main.c -std=c99 -fopenmp -lm
```

## Execução

O programa requer três argumentos de linha de comando:

1. `datafile`: Nome do arquivo contendo o dataset (por exemplo, `data.txt`).
2. `k`: Número de clusters desejados.
3. `num_threads`: Número de threads para execução paralela.

### Exemplos

#### Executando a Versão Sequencial

```bash
./kmeans data.txt 5 1
```

#### Executando a Versão Paralela com 2 Threads

```bash
./kmeans_parallel data.txt 5 2
```

#### Executando a Versão Paralela com 4 Threads

```bash
./kmeans_parallel data.txt 5 4
```

#### Executando a Versão Paralela com 8 Threads

```bash
./kmeans_parallel data.txt 5 8
```

## Resultados de Desempenho

Os testes foram realizados no servidor **parcode** com uma base de **1.000.000** de registros e **4** features. Os tempos de execução foram medidos para diferentes números de threads.

### Tempos de Execução

| Número de Threads | Tempo de Execução (segundos) | Speedup |
|-------------------|------------------------------|---------|
| 1 (Sequencial)    | 147.362434                   | 1.00    |
| 1 (Paralelo)      | 147.362434                   | 1.00    |
| 2                 | 84.037962                    | 1.75    |
| 4                 | 40.977002                    | 3.60    |
| 8                 | 23.769238                    | 6.19    |

**Observação**: O speedup é calculado como o tempo sequencial dividido pelo tempo paralelo.

### Gráfico de Speedup

Para visualizar melhor o ganho de desempenho, segue o gráfico do speedup em função do número de threads:

```
Speedup
^
|                               *
|                        *
|                 *
|          *
|    *
|________________________________> Número de Threads
     1     2     4     8
```

### Análise dos Resultados

- **Speedup Proporcional**: Observa-se que o speedup aumenta com o número de threads, aproximando-se de um speedup linear até 8 threads.
- **Eficiência Paralela**: Com 8 threads, a eficiência (speedup / número de threads) é de aproximadamente 77%, o que indica uma boa escalabilidade do algoritmo paralelizado.
- **Overhead de Paralelização**: A diferença entre o speedup ideal e o obtido deve-se ao overhead inerente à criação e gerenciamento de threads, bem como à sincronização necessária entre elas.

## Explicação sobre a Aplicação

O algoritmo K-Means é um método de aprendizado não supervisionado usado para agrupar dados em k clusters com base em características similares. Ele funciona através dos seguintes passos:

1. **Inicialização**: Escolha inicial dos k centroides.
2. **Atribuição**: Cada ponto de dados é atribuído ao cluster com o centróide mais próximo.
3. **Atualização**: Os centroides são recalculados como a média dos pontos atribuídos a cada cluster.
4. **Iteração**: Os passos de atribuição e atualização são repetidos até que os centroides não mudem significativamente.

### Paralelização com OpenMP

A paralelização foi implementada nas seguintes partes críticas do algoritmo:

- **Atribuição das Observações aos Clusters**: As observações são distribuídas entre os threads para calcular a distância aos centroides e determinar o cluster mais próximo.
- **Atualização dos Centroides**: Cada thread calcula somas parciais das features para os clusters locais, que são combinadas posteriormente para atualizar os centroides globais.

### Comentários sobre o Código

- As mudanças para a paralelização estão comentadas no código-fonte `main.c`.
- Foi utilizado o OpenMP para facilitar a distribuição do trabalho entre os threads.
- Cuidados foram tomados para evitar condições de corrida e garantir a corretude dos resultados.

## Instruções Adicionais

- **Dataset Personalizado**: Caso deseje utilizar um dataset próprio, certifique-se de que ele está no formato adequado e atualize o nome do arquivo nos comandos de execução.
- **Número de Clusters (`k`)**: O valor de `k` pode ser ajustado conforme a necessidade, alterando o segundo argumento na linha de comando.
- **Número de Threads**: Para obter o melhor desempenho, ajuste o número de threads de acordo com o número de núcleos disponíveis no seu sistema.

## Conclusão

Este projeto demonstra como a paralelização pode ser utilizada para acelerar algoritmos computacionalmente intensivos como o K-Means. Utilizando OpenMP, foi possível reduzir significativamente o tempo de execução, especialmente em sistemas com múltiplos núcleos.

---

**Observação**: Certifique-se de que o OpenMP está instalado e configurado corretamente no seu ambiente. Em caso de dúvidas ou problemas na compilação ou execução, consulte a documentação do seu compilador ou procure por soluções específicas para o seu sistema operacional.

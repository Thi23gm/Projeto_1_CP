// Tempos de execução no servidor parcode (Base 1.000.000 de registros e 4 features):
// Tempo da versão sequencial: 147.362434 segundos
// Tempo da versão paralela (1 thread): 147.362434 segundos
// Tempo da versão paralela (2 threads): 84.037962 segundos
// Tempo da versão paralela (4 threads): 40.977002 segundos
// Tempo da versão paralela (8 threads): 23.769238 segundos
// Speedup com 8 threads: 6.19

#include <omp.h>          // Biblioteca OpenMP para paralelismo
#include <float.h>        // DBL_MAX, DBL_MIN
#include <math.h>         // sqrt
#include <stdio.h>        // printf, fprintf
#include <stdlib.h>       // rand, srand, malloc, free, atoi, atof
#include <string.h>       // memset, strtok
#include <time.h>         // time

typedef struct observation
{
    double* features; /**< array de features */
    int num_features; /**< número de features */
    int group;        /**< o número do grupo ao qual esta observação pertence */
} observation;

typedef struct cluster
{
    double* features; /**< array de centroides das features */
    int num_features; /**< número de features */
    size_t count;     /**< número de observações neste cluster */
} cluster;

/*!
 * Função que calcula o índice do centróide mais próximo de uma observação.
 * Função sequencial.
 */
int calculateNearest(observation* o, cluster clusters[], int k)
{
    double minD = DBL_MAX;
    int index = -1;
    for (int i = 0; i < k; i++)
    {
        double dist = 0.0;
        for (int f = 0; f < o->num_features; f++)
        {
            double diff = clusters[i].features[f] - o->features[f];
            dist += diff * diff;
        }
        if (dist < minD)
        {
            minD = dist;
            index = i;
        }
    }
    return index;
}

cluster* kMeans(observation observations[], size_t size, int num_features, int k)
{
    cluster* clusters = NULL;
    if (k <= 1)
    {
        clusters = (cluster*)malloc(sizeof(cluster));
        clusters[0].features = (double*)calloc(num_features, sizeof(double));
        clusters[0].num_features = num_features;
        clusters[0].count = size;
        for (size_t i = 0; i < size; i++)
        {
            for (int f = 0; f < num_features; f++)
            {
                clusters[0].features[f] += observations[i].features[f];
            }
            observations[i].group = 0;
        }
        for (int f = 0; f < num_features; f++)
        {
            clusters[0].features[f] /= size;
        }
    }
    else if (k < size)
    {
        clusters = (cluster*)malloc(sizeof(cluster) * k);
        for (int i = 0; i < k; i++)
        {
            clusters[i].features = (double*)calloc(num_features, sizeof(double));
            clusters[i].num_features = num_features;
            clusters[i].count = 0;
        }

        // Inicialização das observações com grupos aleatórios
        #ifdef _OPENMP
        #pragma omp parallel
        {
            unsigned int seed = time(NULL) ^ omp_get_thread_num();
            #pragma omp for
            for (size_t j = 0; j < size; j++)
            {
                observations[j].group = rand_r(&seed) % k;
            }
        }
        #else
        // Versão sequencial
        srand(time(NULL));
        for (size_t j = 0; j < size; j++)
        {
            observations[j].group = rand() % k;
        }
        #endif


        size_t changed;
        int maxIterations = 300; // Limite máximo de iterações
        int iteration = 0;

        do
        {
            // Resetar clusters
            for (int i = 0; i < k; i++)
            {
                memset(clusters[i].features, 0, num_features * sizeof(double));
                clusters[i].count = 0;
            }

            // Atualização dos clusters
            // Paralelização deste loop para somar as features
            #ifdef _OPENMP
            #pragma omp parallel
            {
                int tid = omp_get_thread_num();
                int nthreads = omp_get_num_threads();
                size_t chunk_size = (size + nthreads - 1) / nthreads;
                size_t start = tid * chunk_size;
                size_t end = (start + chunk_size) > size ? size : (start + chunk_size);

                cluster* local_clusters = (cluster*)malloc(sizeof(cluster) * k);
                for (int i = 0; i < k; i++)
                {
                    local_clusters[i].features = (double*)calloc(num_features, sizeof(double));
                    local_clusters[i].num_features = num_features;
                    local_clusters[i].count = 0;
                }

                for (size_t j = start; j < end; j++)
                {
                    int t = observations[j].group;
                    for (int f = 0; f < num_features; f++)
                    {
                        local_clusters[t].features[f] += observations[j].features[f];
                    }
                    local_clusters[t].count++;
                }

                // Combinar resultados locais
                #pragma omp critical
                {
                    for (int i = 0; i < k; i++)
                    {
                        for (int f = 0; f < num_features; f++)
                        {
                            clusters[i].features[f] += local_clusters[i].features[f];
                        }
                        clusters[i].count += local_clusters[i].count;
                    }
                }

                for (int i = 0; i < k; i++)
                {
                    free(local_clusters[i].features);
                }
                free(local_clusters);
            }
            #else
            for (size_t j = 0; j < size; j++)
            {
                int t = observations[j].group;
                for (int f = 0; f < num_features; f++)
                {
                    clusters[t].features[f] += observations[j].features[f];
                }
                clusters[t].count++;
            }
            #endif

            // Atualizar os centróides
            for (int i = 0; i < k; i++)
            {
                if (clusters[i].count > 0)
                {
                    for (int f = 0; f < num_features; f++)
                    {
                        clusters[i].features[f] /= clusters[i].count;
                    }
                }
                else
                {
                    // Re-inicializar o cluster vazio para uma posição aleatória
                    for (int f = 0; f < num_features; f++)
                    {
                        clusters[i].features[f] = ((double)rand() / RAND_MAX);
                    }
                }
            }

            // Reatribuição das observações aos clusters mais próximos
            changed = 0;
            #ifdef _OPENMP
            #pragma omp parallel for reduction(+:changed)
            #endif
            for (size_t j = 0; j < size; j++)
            {
                int new_group = calculateNearest(&observations[j], clusters, k);
                if (new_group != observations[j].group)
                {
                    changed++;
                    observations[j].group = new_group;
                }
            }

            iteration++;
            // Condição de parada
        } while (changed > 0 && iteration < maxIterations);
    }
    else
    {
        // Cada observação é um cluster
        clusters = (cluster*)malloc(sizeof(cluster) * k);
        for (int i = 0; i < k; i++)
        {
            clusters[i].features = (double*)calloc(num_features, sizeof(double));
            clusters[i].num_features = num_features;
            clusters[i].count = 1;
            for (int f = 0; f < num_features; f++)
            {
                clusters[i].features[f] = observations[i].features[f];
            }
            observations[i].group = i;
        }
    }
    return clusters;
}

/*!
 * Função para ler os dados de um arquivo e executar o K-Means.
 */
void test(const char* filename, int k)
{
    // Abrir o arquivo
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s.\n", filename);
        exit(EXIT_FAILURE);
    }

    size_t size = 0;
    int num_features = 0;
    char line[10240];

    // Determinar o número de observações e features
    if (fgets(line, sizeof(line), fp))
    {
        // Contar o número de features
        char* token = strtok(line, ", \t\n");
        while (token)
        {
            num_features++;
            token = strtok(NULL, ", \t\n");
        }
        size++;
    }
    else
    {
        fprintf(stderr, "Arquivo vazio.\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), fp))
    {
        size++;
    }

    rewind(fp);

    // Alocar vetor de observações
    observation* observations = (observation*)malloc(sizeof(observation) * size);
    if (observations == NULL)
    {
        fprintf(stderr, "Erro na alocação de observations.\n");
        exit(EXIT_FAILURE);
    }

    // Ler os dados e preencher as observações
    size_t index = 0;
    while (fgets(line, sizeof(line), fp))
    {
        observations[index].features = (double*)malloc(sizeof(double) * num_features);
        observations[index].num_features = num_features;
        observations[index].group = -1;

        char* token = strtok(line, ", \t\n");
        int feature_index = 0;
        while (token && feature_index < num_features)
        {
            observations[index].features[feature_index] = atof(token);
            feature_index++;
            token = strtok(NULL, ", \t\n");
        }
        if (feature_index != num_features)
        {
            fprintf(stderr, "Inconsistência no número de features na linha %zu.\n", index + 1);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        index++;
    }

    fclose(fp);

    // Executar o algoritmo K-Means
    cluster* clusters = kMeans(observations, size, num_features, k);

    // Liberar memória
    for (size_t i = 0; i < size; i++)
    {
        free(observations[i].features);
    }
    free(observations);
    for (int i = 0; i < k; i++)
    {
        free(clusters[i].features);
    }
    free(clusters);
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Uso: %s datafile k num_threads\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* datafile = argv[1];
    int k = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    // Configurar o número de threads para a versão paralela
    #ifdef _OPENMP
    omp_set_num_threads(num_threads);
    #endif

    // Medir o tempo de execução
    double start = omp_get_wtime();
    test(datafile, k);
    double end = omp_get_wtime();
    printf("Tempo de execução: %.6f segundos\n", end - start);

    return 0;
}

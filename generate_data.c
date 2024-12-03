#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int num_observations = 1000000; // Número de observações
    int num_features = 4;         // Número de features por observação

    FILE *fp = fopen("data.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Erro ao criar o arquivo data.txt.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    for (int i = 0; i < num_observations; i++) {
        for (int j = 0; j < num_features; j++) {
            double value = ((double)rand() / RAND_MAX) * 10.0;
            fprintf(fp, "%.4f", value);
            if (j < num_features - 1) {
                fprintf(fp, ",");
            }
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Dataset sintético data.txt criado com sucesso.\n");
    return 0;
}

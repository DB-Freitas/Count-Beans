#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "../utils/imagelib.h"

// Mínimo de dois números
int MIN(int x, int y){
    return (x < y) ? x : y;
}

// Transformação da distância
void distanceTransform(image In) {
    int nr = In->nr;
    int nc = In->nc;
    int *px = In->px;

    // Cria uma matriz de distância com valores máximos
    int *dist = (int *)malloc(nr * nc * sizeof(int));
    for (int i = 0; i < nr * nc; i++) {
        dist[i] = (px[i] == 0) ? 0 : INT_MAX;
    }

    // Passagem direta
    for (int i = 0; i < nr; i++) {
        for (int j = 0; j < nc; j++) {
            if (px[i * nc + j] != 0) {
                if (i > 0)
                    dist[i * nc + j] = MIN(dist[i * nc + j], dist[(i - 1) * nc + j] + 1);
                if (j > 0)
                    dist[i * nc + j] = MIN(dist[i * nc + j], dist[i * nc + (j - 1)] + 1);
            } else {
                dist[i * nc + j] = 0;
            }
        }
    }

    // Passagem reversa
    for (int i = nr - 1; i >= 0; i--) {
        for (int j = nc - 1; j >= 0; j--) {
            if (px[i * nc + j] != 0) {
                if (i < nr - 1)
                    dist[i * nc + j] = MIN(dist[i * nc + j], dist[(i + 1) * nc + j] + 1);
                if (j < nc - 1)
                    dist[i * nc + j] = MIN(dist[i * nc + j], dist[i * nc + (j + 1)] + 1);
            }
        }
    }

    int maxDistance = 0;
    for (int i = 0; i < nr * nc; i++) {
        if (dist[i] > maxDistance) {
            maxDistance = dist[i];
        }
    }

    // Copia a matriz de distâncias para a imagem
    for (int i = 0; i < nr * nc; i++) {
        px[i] = (dist[i] >= maxDistance -10) ? 255 : 0;
    }

    free(dist);
}

int isUnique(int arr[], int index) {
    for (int i = 0; i < index; i++) {
        if (arr[i] == arr[index]) {
            return 0;
        }
    }
    return 1;
}

// Função que conta quantos números diferentes existem em um vetor
int countUniqueNumbers(int arr[], int tamanho) {
    int count = 0;
    
    for (int i = 0; i < tamanho; i++) {
        if (isUnique(arr, i)) {
            count++;
        }
    }
    
    return count;
}

int find(int parent[], int i)
{
    while (parent[i] != i)
        i = parent[i];
    return i;
}

void Union(int parent[], int i, int j)
{
    int x = find(parent, i);
    int y = find(parent, j);
    parent[y] = x;
}

void label(image In) {
    int nr = In->nr;
    int nc = In->nc;
    int *px = In->px;
    int parent[nr * nc];
    int labelCount = 0;

    // Inicializa o array parent
    for (int i = 0; i < nr * nc; i++)
        parent[i] = i;

    // Primeira passagem
    for (int i = 0; i < nr; i++) {
        for (int j = 0; j < nc; j++) {
            if (px[i * nc + j] != 0) {
                int up = (i > 0) ? px[(i - 1) * nc + j] : 0;
                int left = (j > 0) ? px[i * nc + (j - 1)] : 0;

                if (up == 0 && left == 0) {
                    px[i * nc + j] = ++labelCount;
                } else if (up != 0 && left == 0) {
                    px[i * nc + j] = up;
                } else if (up == 0 && left != 0) {
                    px[i * nc + j] = left;
                } else {
                    int rootUp = find(parent, up);
                    int rootLeft = find(parent, left);
                    int minRoot = MIN(rootUp, rootLeft);
                    px[i * nc + j] = minRoot;
                    if (rootUp != rootLeft)
                        Union(parent, rootUp, rootLeft);
                }
            }
        }
    }

    // Segunda passagem - Atualiza todos os rótulos para os seus rótulos raiz
    for (int i = 0; i < nr * nc; i++)
        px[i] = find(parent, px[i]);

    In->ml = countUniqueNumbers(px, nr * nc);
}

image limiarization(image In)
{
    image Out = img_clone(In);
    for (int i = 0; i < In->nr * In->nc; i++) {
        Out->px[i] = (In->px[i] > 87) ? In->ml : 0;
    }

    return Out;
}

void msg(char *s)
{
    printf("\nIntensidade");
    printf("\n-------------------------------");
    printf("\nUso:  %s  nome-imagem[.pgm] \n\n", s);
    printf("    nome-imagem[.pgm] é o nome do arquivo da imagem \n");
    exit(1);
}

//main
int main(int argc, char *argv[]) {
    char nameIn[100], nameOut[100], cmd[110];
    image In, Out;
    if (argc < 2)
        msg(argv[0]);

    img_name(argv[1], nameIn, nameOut, GRAY, GRAY);

    In = img_get(nameIn, GRAY);

    // Limiarização
    Out = limiarization(In);

    // Transformação de distância
    distanceTransform(Out);

    label(Out);

    printf("#componentes= %d\n", Out->ml -1);

    img_put(Out, nameOut, GRAY);

    sprintf(cmd, "%s %s &", VIEW, nameOut);
    system(cmd);
    img_free(In);
    img_free(Out);
    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIDTH 640
#define HEIGHT 480
#define ORTOGONAL 0
#define PERSPECTIVA 1

typedef struct _tObj{
    int nPontos;
    int nArestas;
    float **pontos;
    int **arestas;
    float **modelMatrix;
} tObjeto3d;

typedef struct _tCam{
    float pos[3];
    float centro[3];
    float cima[3];
    float **viewMatrix;
} tCamera;

typedef struct _tProj{
    int tipo;
    float left;
    float right;
    float top;
    float bottom;
    float near;
    float far;
    float **projectionMatrix;
} tProj;


void criaIdentidade4d(float **novaMatriz){
    int i,j;

    for(i=0; i<4; i++){
        for(j=0; j<4; j++)
            if(i==j) novaMatriz[i][j] = 1.0;
            else novaMatriz[i][j] = 0.0;
    }
}

void imprimeMatriz(float **matriz){
    int i, j;
    fflush(stdout);
    for(i=0; i<4; i++){
        for(j=0; j<4; j++){
            printf("%8.4f ", matriz[i][j]);
        }
        printf("\n");
    }
}

float *multMatrizPonto4d(float **matriz, float *ponto){
    float *resultado;
    int i, j;

    resultado = (float *) malloc(4 * sizeof(float));

    for(i=0; i<4; i++){
        resultado[i] = 0.0;
        for(j=0; j<4; j++){
            resultado[i] += matriz[i][j] * ponto[j];
        }
    }
    return resultado;
}

void MultMatriz4d(float **matrizA, float **matrizB){
    float matrizC[4][4], soma;
    int k, i, j;

    for(i=0; i<4; i++)
        for(j=0; j<4; j++){
            soma = 0.0;
            for(k=0; k<4; k++)
                soma += matrizA[i][k] * matrizB[k][j];
            matrizC[i][j] = soma;
        }

    for(i=0; i<4; i++)
        for(j=0; j<4; j++)
            matrizB[i][j] = matrizC[i][j];
}

tProj *criaProjecao(int tipo, float left, float right, float top, float bottom, float near, float far){
    int i, j;
    tProj *novaProjecao;

    novaProjecao = (tProj *) malloc(sizeof(tProj));

    novaProjecao->projectionMatrix = (float **) malloc(sizeof(float *));

    for(i=0; i<4; i++){
        novaProjecao->projectionMatrix[i] = (float *) malloc(sizeof(float));
        for(j=0; j<4; j++)
            novaProjecao->projectionMatrix[i][j] = 0.0;
    }
    novaProjecao->projectionMatrix[0][0] = 2.0 / 16.0;
    novaProjecao->projectionMatrix[1][1] = 2.0 / 12.0;
    novaProjecao->projectionMatrix[2][2] = -2.0 / -19.0;
    novaProjecao->projectionMatrix[3][3] = 1.0;
    novaProjecao->projectionMatrix[2][3] = 1.0;
    return novaProjecao;
}

tCamera *criaCamera(){
    int i, j;
    tCamera *novacamera;

    novacamera = (tCamera *) malloc(sizeof(tCamera));

    novacamera->viewMatrix = (float **) malloc(sizeof(float *));

    for(i=0; i<4; i++){
        novacamera->viewMatrix[i] = (float *) malloc(sizeof(float));
    }

    novacamera->pos[0] = 0.0;
    novacamera->pos[0] = 0.0;
    novacamera->pos[0] = 1.0;

    novacamera->centro[0] = 0.0;
    novacamera->centro[0] = 0.0;
    novacamera->centro[0] = 0.0;

    novacamera->cima[0] = 0.0;
    novacamera->cima[0] = 1.0;
    novacamera->cima[0] = 0.0;

    for(i=0; i<4; i++){
        for(j=0; j<4; j++)
            if(i==j) novacamera->viewMatrix[i][j] = 1.0;
            else if(i==2 && j==3) novacamera->viewMatrix[i][j] = -1.0;
            else novacamera->viewMatrix[i][j] = 0.0;
    }

    return novacamera;
}

void desenhaArestaViewport(SDL_Renderer *renderer, float *ponto1, float *ponto2){
    int p1x, p1y, p2x, p2y;

    p1x = ((ponto1[0])+1) * WIDTH/2;
    p1y = (1-ponto1[1]) * HEIGHT/2;
    p2x = ((ponto2[0])+1) * WIDTH/2;
    p2y = (1-ponto2[1]) * HEIGHT/2;
    printf("Desenhando (%d, %d) -- (%d, %d)\n", p1x, p1y, p2x, p2y);
    SDL_RenderDrawLine(renderer, p1x, p1y, p2x, p2y);

    return;
}

tObjeto3d *carregaObjeto(char *nomeArquivo){
    int i, j;
    FILE *arquivoObj;
    tObjeto3d *novoObjeto;

    arquivoObj = fopen(nomeArquivo, "r");
    if(arquivoObj == NULL) {
        printf("Erro ao abrir arquivo\n");
        return NULL;
    }

    novoObjeto = (tObjeto3d *) malloc(sizeof(tObjeto3d));

    fscanf(arquivoObj,"%d", &(novoObjeto->nPontos));

    novoObjeto->pontos = (float **) malloc(novoObjeto->nPontos * sizeof(float*));

    for(i=0; i<novoObjeto->nPontos; i++){
        novoObjeto->pontos[i]=(float *) malloc(3 * sizeof(float));
        fscanf(arquivoObj, "%f%f%f", &(novoObjeto->pontos[i][0]), &(novoObjeto->pontos[i][1]), &(novoObjeto->pontos[i][2]));
    }

    fscanf(arquivoObj, "%d", &(novoObjeto->nArestas));
    novoObjeto->arestas = (int **) malloc(novoObjeto->nArestas * sizeof(int*));
    for(i=0; i<novoObjeto->nArestas; i++){
        novoObjeto->arestas[i]=(int *) malloc(2 * sizeof(int));
        fscanf(arquivoObj, "%d%d", &(novoObjeto->arestas[i][0]), &(novoObjeto->arestas[i][1]));
    }

    novoObjeto->modelMatrix = (float **) malloc(sizeof(float *));
    for(i=0; i<4; i++){
        novoObjeto->modelMatrix[i] = (float *) malloc(sizeof(float));
        for(j=0; j<4; j++)
            if(i==j) novoObjeto->modelMatrix[i][j] = 1.0;
            else novoObjeto->modelMatrix[i][j] = 0.0;
    }
    return novoObjeto;
}

void imprimeObjeto(tObjeto3d *objeto){
    int i;

    if(objeto == NULL){
        printf("ERRO: Objeto nulo!\n");
        return;
    }

    printf("Pontos\n");

    for(i = 0; i<objeto->nPontos; i++)
        printf(" [%d] - (%8.4f, %8.4f, %8.4f)\n", i, objeto->pontos[i][0], objeto->pontos[i][1], objeto->pontos[i][2]);

    printf("Arestas\n");

    for(i = 0; i<objeto->nArestas; i++)
        printf(" [%d] - (%3d, %3d)\n", i, objeto->arestas[i][0], objeto->arestas[i][1]);

    printf("modelMatrix\n");
    imprimeMatriz(objeto->modelMatrix);
}

void desenhaObjeto(SDL_Renderer *renderer, float **matriz, tObjeto3d *objeto){
    int i;
    float *ponto1, *ponto2;
    for(i=0; i < objeto->nArestas; i++ ){
        ponto1 = multMatrizPonto4d(matriz, objeto->pontos[objeto->arestas[i][0]]);
        ponto2 = multMatrizPonto4d(matriz, objeto->pontos[objeto->arestas[i][1]]);
        desenhaArestaViewport(renderer, ponto1, ponto2);
        free(ponto1);
        free(ponto2);
    }
}

int rotateObj(float **modelMatrix, float ang, int x, int y, int z){

    ang = ang*3.141592/180; //rad
    float sinAng = sin(ang);
    float cosAng = cos(ang);

    modelMatrix[0][0] = (1-cosAng)*x*x + cosAng;
    modelMatrix[0][1] = (1-cosAng)*x*y - sinAng*z;
    modelMatrix[0][2] = (1-cosAng)*x*z + sinAng*y;

    modelMatrix[1][0] = (1-cosAng)*x*y + sinAng*z;
    modelMatrix[1][2] = (1-cosAng)*y*y + cosAng;
    modelMatrix[1][3] = (1-cosAng)*y*z - sinAng*x;

    modelMatrix[2][3] = (1-cosAng)*x*z - sinAng*y;
    modelMatrix[2][3] = (1-cosAng)*y*z + sinAng*x;
    modelMatrix[2][3] = (1-cosAng)*z*z + cosAng;

    printf("rotacionando");
    return 0;    
}


int main(int arc, char *argv[]){
    SDL_Window *window;
    SDL_Event windowEvent;
    SDL_Renderer *renderer;
    tObjeto3d *objeto1;
    tCamera *camera1;
    tProj *projecao1;
    float **matrizComposta;
    int i, quit = 0;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Erro ao inicializar SDL! SDL Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow("Hello SDL World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL){
        printf("Erro ao criar janela! SDL Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    char file[20] = "cubo.dcg";
    // char file[20] = "quadrado.dcg";
    objeto1 = carregaObjeto(file);
    imprimeObjeto(objeto1);

    camera1 = criaCamera();
    projecao1 = criaProjecao(0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    matrizComposta = (float **) malloc(sizeof(float *));
    for(i=0; i<4; i++)
        matrizComposta[i] = (float *) malloc(sizeof(float));

    float girar = 0;
    while(!quit){
        if(girar == 180)
            girar = -180;
        else
            girar += 1;

        SDL_Delay(10);
        SDL_PollEvent(&windowEvent);
        switch (windowEvent.type){
            case SDL_QUIT:
                quit = 1;
                break;
            // TODO input handling code goes here
        }

        // clear window

        SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
        SDL_RenderClear(renderer);

        // TODO rendering code goes here
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        printf("Cria identidade...\n");
        criaIdentidade4d(matrizComposta);
        imprimeMatriz(matrizComposta);

        rotateObj(objeto1->modelMatrix,girar, 0, 1, 0);
        printf("Multiplicando matrizes Model X Id...\n");
        MultMatriz4d(objeto1->modelMatrix , matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Multiplicando matrizes View X Model...\n");
        MultMatriz4d(camera1->viewMatrix , matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Multiplicando matrizes Projecao X View X Model...\n");
        MultMatriz4d(projecao1->projectionMatrix , matrizComposta);
        imprimeMatriz(matrizComposta);

        printf("Desenhando objeto...\n");
        
        desenhaObjeto(renderer, matrizComposta, objeto1);

        // render window

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>

// #define _DEBUG
// #define WIDTH 640
// #define HEIGHT 480
#define WIDTH 1920
#define HEIGHT 1080
#define ORTOGONAL 0
#define PERSPECTIVA 1
#define MATRIXLENGTH 4

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
        resultado[i] = 0.0f;
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

    novaProjecao->projectionMatrix = (float **) malloc(MATRIXLENGTH * sizeof(float *));
    
    for(i=0; i<4; i++){
        novaProjecao->projectionMatrix[i] = (float *) malloc(MATRIXLENGTH * sizeof(float));
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

    novacamera->viewMatrix = (float **) malloc(MATRIXLENGTH * sizeof(float *));

    for(i=0; i<4; i++){
        novacamera->viewMatrix[i] = (float *) malloc(MATRIXLENGTH * sizeof(float));
    }

    novacamera->pos[0] = 0.0;
    novacamera->pos[1] = 0.0;
    novacamera->pos[2] = 1.0;

    novacamera->centro[0] = 0.0;
    novacamera->centro[1] = 0.0;
    novacamera->centro[2] = 0.0;

    novacamera->cima[0] = 0.0;
    novacamera->cima[1] = 1.0;
    novacamera->cima[2] = 0.0;

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
    #ifdef _DEBUG
        printf("Desenhando (%d, %d) -- (%d, %d)\n", p1x, p1y, p2x, p2y);
    #endif
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
        novoObjeto->pontos[i]=(float *) malloc(4 * sizeof(float));
        fscanf(arquivoObj, "%f%f%f", &(novoObjeto->pontos[i][0]), &(novoObjeto->pontos[i][1]), &(novoObjeto->pontos[i][2]));
        novoObjeto->pontos[i][3] = 1;
    }

    fscanf(arquivoObj, "%d", &(novoObjeto->nArestas));
    novoObjeto->arestas = (int **) malloc(novoObjeto->nArestas * sizeof(int*));
    for(i=0; i<novoObjeto->nArestas; i++){
        novoObjeto->arestas[i]=(int *) malloc(2 * sizeof(int));
        fscanf(arquivoObj, "%d%d", &(novoObjeto->arestas[i][0]), &(novoObjeto->arestas[i][1]));
    }

    novoObjeto->modelMatrix = (float **) malloc(MATRIXLENGTH * sizeof(float *));
    for(i=0; i<4; i++){
        novoObjeto->modelMatrix[i] = (float *) malloc(MATRIXLENGTH * sizeof(float));
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

void desenhaEixo(SDL_Renderer *renderer, float **matriz, tObjeto3d *objeto){
    int i;
    float *ponto1, *ponto2;
    // SDL_RenderDrawPoint( renderer, 0, 0 );

    for(i=0; i < objeto->nArestas; i++ ){
        if(i==0)
            SDL_SetRenderDrawColor( renderer, 0, 255, 94, 255 );
        else if(i==1)
            SDL_SetRenderDrawColor( renderer, 0, 136, 255, 255 );
        else
            SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );

        ponto1 = multMatrizPonto4d(matriz, objeto->pontos[objeto->arestas[i][0]]);
        ponto2 = multMatrizPonto4d(matriz, objeto->pontos[objeto->arestas[i][1]]);
        desenhaArestaViewport(renderer, ponto1, ponto2);
        free(ponto1);
        free(ponto2);
    }
}

void rotate(float **modelMatrix, float ang, int x, int y, int z){

    ang = ang* M_PI/180; //rad
    float sinAng = sin(ang);
    float cosAng = cos(ang);
    float **matrix;

    matrix = (float**) malloc(MATRIXLENGTH * sizeof(float*));
    for (int i = 0; i < MATRIXLENGTH; i++)
        matrix[i] = (float*) malloc(MATRIXLENGTH * sizeof(float));
    
    criaIdentidade4d(matrix);

    matrix[0][0] = (1-cosAng)*x*x + cosAng;
    matrix[0][1] = (1-cosAng)*x*y - sinAng*z;
    matrix[0][2] = (1-cosAng)*x*z + sinAng*y;

    matrix[1][0] = (1-cosAng)*x*y + sinAng*z;
    matrix[1][1] = (1-cosAng)*y*y + cosAng;
    matrix[1][2] = (1-cosAng)*y*z - sinAng*x;

    matrix[2][0] = (1-cosAng)*x*z - sinAng*y;
    matrix[2][1] = (1-cosAng)*y*z + sinAng*x;
    matrix[2][2] = (1-cosAng)*z*z + cosAng;

    MultMatriz4d(matrix, modelMatrix);   
}

void translate(float **modelMatrix, float x, float y, float z){
    float **matrix;

    matrix = (float **) malloc(MATRIXLENGTH * sizeof(float *));
    for (int i = 0; i < MATRIXLENGTH; i++)
        matrix[i] = (float *) malloc(MATRIXLENGTH * sizeof(float));
    
    criaIdentidade4d(matrix);
    
    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;

    MultMatriz4d(matrix, modelMatrix);
}

void alteraEscala(float **modelMatrix, float x, float y, float z){

    float **matrix;
    matrix = (float**) malloc(MATRIXLENGTH * sizeof(float*));
    for (int i = 0; i < MATRIXLENGTH; i++)
        matrix[i] = (float*) malloc(MATRIXLENGTH * sizeof(float));
    
    criaIdentidade4d(matrix);

    matrix[0][0] = x;
    matrix[1][1] = y;
    matrix[2][2] = z;

    MultMatriz4d(matrix, modelMatrix);
}

void iniciaSDL(SDL_Event windowEvent, SDL_Renderer *renderer, int quit){
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
}

tProj *criarFrustumProjection(float fovY, float aspectRatio,float front, float back){
    int i,j;
    tProj *novaProjecao;
    novaProjecao = (tProj *) malloc(sizeof(tProj));
    const float DEG2RAD = M_PI / 180;

    float tangent = tan(fovY/2 * DEG2RAD);
    float height = front*tangent;
    float width = height*aspectRatio;

    

    novaProjecao->projectionMatrix = (float **) malloc(MATRIXLENGTH * sizeof(float *));
    for(i=0; i<MATRIXLENGTH; i++){
        novaProjecao->projectionMatrix[i] = (float *) malloc(MATRIXLENGTH * sizeof(float));
        for(j=0; j<MATRIXLENGTH; j++)
            novaProjecao->projectionMatrix[i][j] = 0.0;
    }
    novaProjecao->projectionMatrix[0][0] = 2.0 / 16.0;
    novaProjecao->projectionMatrix[1][1] = 2.0 / 12.0;
    novaProjecao->projectionMatrix[2][2] = -2.0 / -19.0;
    novaProjecao->projectionMatrix[3][3] = 1.0;
    novaProjecao->projectionMatrix[2][3] = 1.0;

    
    const float left = -width; 
    const float right = width;
    novaProjecao->left = -left;
    novaProjecao->right = right;

    const float bottom = -height;
    const float top = height;
    novaProjecao->bottom = bottom;
    novaProjecao->top = top;

    const float near = front;
    const float far = back;
    novaProjecao->near = near;
    novaProjecao->far = far;

    novaProjecao->projectionMatrix[0][0] = (2*near)/(right-left);
    novaProjecao->projectionMatrix[1][1] = (2*near)/(top-back);
    novaProjecao->projectionMatrix[2][2] = -(far+near)/(far-near);
    novaProjecao->projectionMatrix[3][3] = 0;

    novaProjecao->projectionMatrix[0][2] = (right+left)/(right-left);
    novaProjecao->projectionMatrix[1][2] = (top+bottom)/(top-bottom);
    novaProjecao->projectionMatrix[3][2] = -1;
    novaProjecao->projectionMatrix[2][3] = -2*(far*near)/(far-near);

    return novaProjecao;
}

int main(int arc, char *argv[]){
    SDL_Window *window;
    SDL_Event windowEvent;
    SDL_Renderer *renderer;
    tObjeto3d *objeto1;
    tObjeto3d *objeto2;
    tObjeto3d *eixo;
    tCamera *camera1;
    tProj *projecao1;
    float **matrizComposta;
    int i, quit = 0;


    if(SDL_Init(SDL_INIT_VIDEO) < 0){
    printf("Erro ao inicializar SDL! SDL Error: %s\n", SDL_GetError());
    EXIT_FAILURE;
    }

    window = SDL_CreateWindow("Hello SDL World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL){
        printf("Erro ao criar janela! SDL Error: %s\n", SDL_GetError());
        EXIT_FAILURE;
    }

    void criaTela(SDL_Window *window, SDL_Renderer *renderer);
    renderer = SDL_CreateRenderer(window, -1, 0);

    char file[40] = "./objetos/cubo.dcg";
    objeto1 = carregaObjeto(file);
    imprimeObjeto(objeto1);

    camera1 = criaCamera();
    projecao1 = criaProjecao(0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    projecao1 = criarFrustumProjection(1, 1, -2,100);

    matrizComposta = (float **) malloc(MATRIXLENGTH * sizeof(float *));
    for(i=0; i<4; i++)
        matrizComposta[i] = (float *) malloc(MATRIXLENGTH * sizeof(float));

    float ang = 0.4;
    // float translate = 0.5;
    // translate(objeto1->modelMatrix, 0, 0, -12);
    // alteraEscala(objeto1->modelMatrix, 2, 2, 3);


    while(!quit){

        iniciaSDL(windowEvent, renderer, quit);

        criaIdentidade4d(matrizComposta);
        #ifdef _DEBUG
            printf("Cria identidade...\n");
            imprimeMatriz(matrizComposta);
        #endif
        
        MultMatriz4d(objeto1->modelMatrix, matrizComposta);
        #ifdef _DEBUG
            printf("Multiplicando matrizes Model X Id...\n");
            imprimeMatriz(matrizComposta);
        #endif
        
        
        rotate(camera1->viewMatrix, 1, 0, 1, 0);
        MultMatriz4d(camera1->viewMatrix , matrizComposta);
        #ifdef _DEBUG
            printf("Multiplicando matrizes View X Model...\n");
            imprimeMatriz(matrizComposta);
        #endif
        
        MultMatriz4d(projecao1->projectionMatrix , matrizComposta);
        #ifdef _DEBUG
            printf("Multiplicando matrizes Projecao X View X Model...\n");
            imprimeMatriz(matrizComposta);
        #endif

        #ifdef _DEBUG
            printf("Desenhando objeto...\n");
        #endif
        
        desenhaObjeto(renderer, matrizComposta, objeto1);
        // desenhaObjeto(renderer, matrizComposta, objeto2);
        // desenhaEixo(renderer, matrizComposta, eixo);

        // render window
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
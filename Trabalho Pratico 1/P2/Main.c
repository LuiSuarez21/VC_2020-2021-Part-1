#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"

/* GRUPO 15
* Alunos: Luis Esteves - 16960
*         João Morais  - 17214  
*         Sergio Ribeiro - 18858  
* IPCA
* Disciplina: Visão de computadores, LESI-PL, 2º Ano;
* Trabalho Prático nº1 | Problema 2 (P2);
* Neste problema é pretendido calcular o centro de massa e a area de algumas celulas de um olho. Para alem disso, realizar processos morfologicos de maneira a
* extrair essas celulas da imagem original, tornar a imagem em formato binario e demarcar o centro de massa de cada celula;
*/

//Pagina Main.c
int main()
{
    //Criação das varíaveis e dos objectos que nos vão ajudar neste trabalho;
    IVC *image, *image2, *image3, *image4, *image5, *image6, *image7;
    int i, nblobs;
    OVC *blobs;

    //Leitura da imagem (img2.ppm).
    //Criação de objectos com as características da imagem inicial. 
    //Estas novas "imagens" vão ajudar a manipular a imagem inicial.
    //img2.ppm -> imagem inicial
    image = vc_read_image("img2.ppm");
    image2 = vc_image_new(image->width, image->height, 1, 255);
    image3 = vc_image_new(image2->width, image2->height, 1, 255);
    image4 = vc_image_new(image3->width, image3->height, 1, 255);
    image5 = vc_image_new(image4->width, image4->height, 1, 255);
    image6 = vc_image_new(image5->width, image5->height, 1, 255);

    //Verifica se a imagem existe;
    if (image == NULL)
    {
        printf("Erro: vc_read_image()\nFicheiro não encontrado!\n");
        getchar();
        return 0;
    }   

    //Primeiro vamos obter a componente azul da imagem original (Blue de RGB);
    //O azul pois é a cor de cada celula do olho. Queremos ignorar tudo o resto;
    vc_rgb_get_blue_gray(image);       
    //Pegamos no resultado funcao anterior e metemos todos os canais RGB com os valores obtidos anteriormente;
    vc_rgb_to_gray(image, image2);   
    //Aqui convertemos a image2 numa imagem em binario;
    //O threshold utilizado é 100, pois é o melhor valor que nos possibilita ter o minimo de perda e maior exatidao ao obter as celulas do olho;   
    vc_gray_to_bin(image2, image3);     
    //Funcao para preencher falhas da image3;
    vc_bin_close(image3, image4, 7); 
    //Funcao para remover alguns excessos da image4;
    vc_bin_open(image4, image5, 5);  

    //Agora vamos identificar o blob, que neste caso seram cada celula do olho;
    //Esta operacao é importante, não só pela identificacao do blob, mas pelo os calculos que podemos fazer com esta identificacao;
    blobs = vc_bin_blob_label(image5, image6, &nblobs); 
    //Pegamos na imagem ja etiquetada e iremos entao calcular o seu centro de massa, perimetro e area;
    //Aqui tambem identificamos cada centro de massa com um ponto preto (na image5);
    vc_bin_blob_calc(image6, blobs, image5, &nblobs);
    if (blobs != NULL)
    {
        //Impressao dos dados obtidos em cima;
        //Numero de blobs identificados na imagem;
        printf("\nLabels: %d", nblobs);
        for (i = 0; i < nblobs; i++)
        {
            printf("\n\n- Label %d", blobs[i].label);
            printf("\n- Mass center (x,y): %d;%d", blobs[i].xc, blobs[i].yc);
            printf("\n- Area: %d", blobs[i].area);
            
        }
        free(blobs);
    }
   
    //Imprimimos o resultado final;
    vc_write_image("Resultado.pgm", image5);
    //Libertamos o espaço da memoria utilizado nas imagens auxiliares nestes processos todos no Main.c
    vc_image_free(image);
    vc_image_free(image2);
    vc_image_free(image3);
    vc_image_free(image4);
    vc_image_free(image5);
    vc_image_free(image6);
    printf("\nPress any key to exit...\n");
    getchar();
}

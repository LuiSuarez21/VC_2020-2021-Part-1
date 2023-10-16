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
* Trabalho Prático nº1 | Problema 1 (P1);
* Neste problema é pretendido calcular o centro de massa o perimetro e a area de um cerebro. Para alem disso, realizar processos morfologicos de maneira a
* extrair do cranio (imagem original) apenas a parte que nós queremos, que é o cerebro.
*/

//Pagina Main.c
int main()
{
    //Criação das varíaveis e dos objectos que nos vão ajudar neste trabalho;
    IVC *image, *image2, *image3, *image4, *image5, *image6, *image7;
    int i, nblobs;
    OVC *blobs;

    //Leitura da imagem, do crânio.
    //Criação de objectos com as características da imagem inicial. 
    //Estas novas "imagens" vão ajudar a manipular a imagem inicial.
    // img1.pgm -> imagem inicial
    // 
    image = vc_read_image("img1.pgm");
    image2 = vc_image_new(image->width, image->height, 1, 255);
    image3 = vc_image_new(image2->width, image2->height, 1, 255);
    image4 = vc_image_new(image3->width, image3->height, 1, 255);
    image5 = vc_image_new(image4->width, image4->height, 1, 255);
    image6 = vc_image_new(image5->width, image5->height, 1, 255);
    image7 = vc_image_new(image4->width, image4->height, 1, 255);

    //Verifica se a imagem existe;
    if (image == NULL)
    {
        printf("Erro: vc_read_image()\nFicheiro não encontrado!\n");
        getchar();
        return 0;
    }

    //Primeiro passa a imagem de tons de cinzento para uma imagem em binario;
    //Utiliza-se um threshold especial para o problema (ver descricao da funcao em Func.c);
    vc_gray_to_bin(image, image2);
    //Realiza-se a operação morfologica binaria open para retirar possiveis excessos e para tirar algum ruido na imagem;
    vc_bin_open(image2, image3, 7);
    //Realiza-se a operação morfologica binaria dilate para preencher a imagem que obtivemos da operação anterior;
    vc_bin_dilate(image3, image4, 5);

    //Agora vamos identificar o blob, que neste caso será apenas o cerebro;
    //Esta operacao é importante, não pela identificacao do blob, mas pelo os calculos que podemos fazer com esta identificacao;
    blobs = vc_bin_blob_label(image4, image5, &nblobs); 
    //Pegamos na imagem ja etiquetada e iremos entao calcular o seu centro de massa, perimetro e area;
    vc_bin_blob_calc(image5, blobs, &nblobs);
    if (blobs != NULL)
    {
        //Impressao dos dados obtidos em cima;
        //Numero de blobs identificados na imagem;
        printf("\nLabels: %d", nblobs);
        for (i = 0; i < nblobs; i++)
        {
            printf("\n\n- Label %d", blobs[i].label);
            printf("\n- Mass center (x,y): %d;%d", blobs[i].xc, blobs[i].yc);
            printf("\n- Perimeter: %d", blobs[i].perimeter);
            printf("\n- Area: %d", blobs[i].area);
            
        }
        free(blobs);
    }
    //Finalmente, damos um toque final na image4 (imagem que não foi etiquetada) e fechamos a imagem de vez com a operacao close;
    vc_bin_close(image4, image6, 5);
    //Com a image5, fazemos a subreposicao de duas imagens (entre a img1 e image6) de maneira a obtermos so o cerebro;
    vc_convert(image,image6,image7);
    //Imprimimos o resultado final;
    vc_write_image("Resultado.pgm", image7);
    //Libertamos o espaço da memoria utilizado nas imagens auxiliares nestes processos todos no Main.c
    vc_image_free(image);
    vc_image_free(image2);
    vc_image_free(image3);
    vc_image_free(image4);
    vc_image_free(image5);
    vc_image_free(image6);
    vc_image_free(image7);
    printf("\nPress any key to exit...\n");
    getchar();
}

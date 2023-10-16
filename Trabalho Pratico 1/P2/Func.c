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

//Func.C -> Pagina onde se situam todas as funcoes utilizadas no trabalho;

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

#pragma region Codigo Base
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)))
			;
		if (c != '#')
			break;
		do
			c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF)
			break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#')
			ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0)
		{
			channels = 1;
			levels = 1;
		} // Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0)
			channels = 1; // Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0)
			channels = 3; // Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL)
		return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

#pragma endregion

#pragma region Codigo desenvolvido fora das aulas

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//          FUNÇÕES: BINÁRIZAÇÃO DA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Funcao que ira calcular a Grayscale da imagem a partir da componente Blue da imagem original (Blue de RGB);
int vc_rgb_get_blue_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica erros
	if (srcdst == NULL)
	{
		printf("Erro: vc_rgb_get_blue_gray()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL)
	{
		printf("Erro: vc_rgb_get_blue_gray()\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if (srcdst->channels != 3)
	{
		printf("Erro: vc_rgb_get_blue_gray()\n-Imagem com formato errado!\n");
		getchar();
		return 0;
	}

	//Pega no valor azul e iguala todos os outros a esse valor
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = data[pos + 2];	   //Red
			data[pos + 1] = data[pos + 2]; //Green
		}
	}
	return 1;
}

//Calcular RGB para Gray (tons de cinzento);
int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float r, g, b;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Erro: vc_rgb_to_gray()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Erro: vc_rgb_to_gray()\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height))
	{
		printf("Erro: vc_rgb_to_gray()\n-Dimensoes erradas!\n");
		getchar();
		return 0;
	}

	if ((src->channels != 3) || (dst->channels != 1))
	{
		printf("Erro: vc_rgb_to_gray()\n-Formato errado!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			r = (float)datasrc[pos_src];
			g = (float)datasrc[pos_src + 1];
			b = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((r * 0.299) + (g * 0.587) + (b * 0.114));

			//versao gimp
			//datadst[pos_dst] = (unsigned char) ((rf+ gf + bf)/ 3.0);
		}
	}

	return 1;
}

//Trata-se de uma funcao que transforma a imagem src que esta em gray para uma imagem em binario porem alteramos o manualmente o valor de theshold de maneira a podermos filtrar a imagem; 
//De resto, a função é simples. Lê a imagem src, verfica se não há erros e depois precorre tudo até encontrar pixeis que estejam entre o threshold e maximo valor possivel;
//Ao encontrar pixeis com entre estes valores, da-lhes o valor 255 (ficam brancos). Tudo o resto fica preto.
int vc_gray_to_bin(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;
	int x, y;
	long int pos;
	int threshold = 0;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Erro: vc_gray_to_bin()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Erro: vc_gray_to_bin()\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if ((src->channels != 1))
	{
		printf("Erro: vc_gray_to_bin()\n-Imagem com formato errado!\n");
		getchar();
		return 0;
	}

	threshold = 100;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			pos = y * bytesperline + x * channels; //Posicao dos pixeis

			if (datasrc[pos] > threshold) //Cada posicao com intensidade acima da média fica a branco
			{
				datadst[pos] = 255;
			}
			//Abaixo da media fica a preto
			else
			{
				datadst[pos] = 0;
			}
		}
	}
	return 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//          FUNÇÕES: OPERADORES MORFOLOGICOS EM IMAGENS BINÁRIAS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// - Operadores Morfologicos (Binarios): Dilatacao
// A dilatacao consiste em adicionar pixéis aos limites de uma regiao segmentada, aumentando assim a sua area e preenchendo algumas zonas no seu interior; 
int vc_bin_dilate(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; 
	float mean, stdeviation, sum, total;
	long int pos, posk;
	unsigned char threshold;
	int max, min;
	int aux = 0;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Erro: vc_bin_dilate()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Erro: vc_bin_dilate():\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Erro: vc_bin_dilate()\n-Erro nas dimensoes!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Erro: vc_bin_dilate()\n-Imagem com formato errado!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) //Dois ciclos para navegar em cada pixel da imagem
		{
			pos = y * bytesperline + x * channels; //Posicao do pixel central

			max = datasrc[pos];
			min = datasrc[pos];
			aux = 0;

			// NxM Vizinhos
			//Dois ciclos para aceder a informacao a volta do pixel central (do Kernel/Margem)
			for (ky = -offset; ky <= offset; ky++) 
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) 
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posicao do kernel 

						if (datasrc[posk] == 255)
							aux = 255; 
					}
				}
			}
			datadst[pos] = aux;
		}
	}

	return 1;
}

// - Operadores Morfologicos (Binarios): Erosao
//  A erosao consiste em remover pixéis aos limites de uma regiao segmentada, diminuindo assim a sua area e eliminando também regiões cuja dimensao seja inferior;
// Resumidamente, trata-se de uma dilatacao, só que ao encontrar um pixer segmentado, em vez de o tornar branco (adicionar à imagem) torna-o preto.
int vc_bin_erode(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; 
	long int pos, posk;
	unsigned char threshold;
	int aux = 0;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Erro: vc_bin_erode()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Erro: vc_bin_erode()\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Erro: vc_bin_erode()\n-Erro nas dimensoes!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Erro: vc_bin_erode()\n-Imagem com formato errado!\n");
		getchar();
		return 0;
	}

	//Dois ciclos para navegar em cada pixel da imagem
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) 
		{
			pos = y * bytesperline + x * channels; 
			aux = 255;
			
			// NxM Vizinhos
			//Dois ciclos para aceder a informacao a volta do pixel central (do Kernel/Margem)
			for (ky = -offset; ky <= offset; ky++) 
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posicao do kernel

						if (datasrc[posk] == 0)
							aux = 0; //Se no kernel houver um pixel segmentado, o limite é removido;
					}
				}
			}
			datadst[pos] = aux;
		}
	}

	return 1;
}

//Operadores Morfologicos (Binarios): Open
//E obtida por uma erosao, seguida de uma dilatacao.Utilizada para remover pequenas regiões de primeiro plano.
int vc_bin_open(IVC *src, IVC *dst, int kernel)
{
	int open = 1;
	IVC *aux = vc_image_new(src->width, src->height, 1, src->levels);
	
	open &= vc_bin_erode(src, aux, kernel);
	open &= vc_bin_dilate(aux, dst, kernel);

	vc_image_free(aux);
	return open;
}

//Operadores Morfologicos (Binarios): Close
//E obtida por uma dilatacao, seguida de uma erosao. Utilizada para preencher falhas dentro de regiões de primeiro plano.
int vc_bin_close(IVC *src, IVC *dst, int kernel)
{
	int close = 1;
	IVC *aux = vc_image_new(src->width, src->height, 1, src->levels);

	close &= vc_bin_dilate(src, aux, kernel);
	close &= vc_bin_erode(aux, dst, kernel);

	vc_image_free(aux);
	return close;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//          FUNÇÕES: BLOBS
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Etiquetagem de blobs
//Esta funcao ira tratar de etiquetar a imagem, ou seja, de identificar quantos blobs existem e identifica os pixeis pertenceste a ele; 
OVC *vc_bin_blob_label(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = {0};
	int labelarea[256] = {0};
	int label = 1; 
	int num, tmplabel;
	OVC *blobs; 

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Erro: vc_bin_blob_label()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Erro: vc_bin_blob_label()\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Erro: vc_bin_blob_label()\n-Erro nas dimensoes!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Erro: vc_bin_blob_label()\n-Imagem com formato errado!\n");
		getchar();
		return 0;
	}

	// Copia dados da imagem binaria para uma imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Serao atribudas etiquetas no intervalo 1 a 254;
	//Background = 0, Foreground = 255
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0)
			datadst[i] = 255;
	}

	// Limpa os excessos da imagem binria
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			
			posA = (y - 1) * bytesperline + (x - 1) * channels; // Este vai ser o nosso A;
			posB = (y - 1) * bytesperline + x * channels;		// Este vai ser o nosso B;
			posC = (y - 1) * bytesperline + (x + 1) * channels; // Este vai ser o nosso C;
			posD = y * bytesperline + (x - 1) * channels;		// Este vai ser o nosso D;
			posX = y * bytesperline + x * channels;				// Este vai ser o nosso X;

			// Se a etiquetagem ja existe atribui-se X a etiqueta;
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				
				else
				{
					//Caso nao exista cria uma nova etiqueta;
					num = 255; 
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					if ((datadst[posB] != 0) && (datadst[posB] != 255) && (datadst[posB] < num)) num = labeltable[datadst[posB]];
					if ((datadst[posC] != 0) && (datadst[posC] != 255) && (datadst[posC] < num)) num = labeltable[datadst[posC]];
					if ((datadst[posD] != 0) && (datadst[posD] != 255) && (datadst[posD] < num)) num = labeltable[datadst[posD]];

					//Atribui etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if ((datadst[posA] != 0))
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel) labeltable[a] = num;
							}
						}
					}
					if ((datadst[posB] != 0))
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel) labeltable[a] = num;
							}
						}
					}
					if ((datadst[posC] != 0))
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel) labeltable[a] = num;
							}
						}
					}
					if ((datadst[posD] != 0))
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel) labeltable[a] = num;
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // Este que e o nosso X;
			if (datadst[posX] != 0)datadst[posX] = labeltable[datadst[posX]];
		}
	}

	// Contagem do numero de blobs
	// Eliminar etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	//Conta etiquetas e organiza a tabela de etiquetas, para que nao exista 0 entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; 
			(*nlabels)++;						
		}
	}


	if (*nlabels == 0)
		return NULL;

	// Cria lista de blobs e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++)
			blobs[a].label = labeltable[a];
	}
	else
		return NULL;

	return blobs;
}

// Calculo da area, perimetro e centro de massa dos blobs obtidos na funcao vc_bin_blob_label;
//Para alem disso, ao descobrirmos nesta funcao o centro de massa de cada blob, indicamos na imagem com um pixel preto o centro de cada blob;
int vc_bin_blob_calc(IVC *src, OVC *blobs, IVC *srcdst, int *nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	unsigned char *datascrdst = (unsigned char *)srcdst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Erro: vc_bin_blob_calc()\n-Imagem vazia!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Erro: vc_bin_blob_calc()\n-Dimensoes ou informacoes erradas!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Erro:  vc_bin_blob_calc()\n-Imagem com formato errado!\n");
		getchar();
		return 0;
	}

	for (i = 0; i < *nblobs; i++)
	{
		//Inicializacao das variaveis;
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;
		sumx = 0;
		sumy = 0;
		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Actualizacao da informacao relativa a area do blob;
					blobs[i].area++;

					// Actualizacao da informacao relativa ao centro de Gravidade do blob;
					sumx += x;
					sumy += y;

					// Actualizacao das informacoes da Bounding Box ("caixa" que determina o tamanho do blob);
					if (xmin > x)
						xmin = x;
					if (ymin > y)
						ymin = y;
					if (xmax < x)
						xmax = x;
					if (ymax < y)
						ymax = y;
					// Perimetro do blob;
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}
		
		// Bounding Box final, obtida depois de todos os calculos feitos e do programa saber ja as dimensoes de cada blob;
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade do blob;
		//Aqui, ao saber qual é o centro do blob, torna-se esse centro na cor oposta do blob, de maneira a marcar então na imagem o centro de cada celula.
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
		pos =  blobs[i].yc * bytesperline + blobs[i].xc * channels;
		if (datascrdst[pos] == 255) datascrdst[pos] = 0;

	}

	return 1;
}


#pragma endregion

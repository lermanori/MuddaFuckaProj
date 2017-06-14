#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int BOOL;
#define FALSE 0
#define TRUE 1

typedef struct _grayImage {
	unsigned short rows, cols;
	unsigned char** pixels;
}grayImage;

typedef unsigned short imgPos[2];

typedef struct _treeNode {
	imgPos position;
	struct _treeNodeListCell *next_possible_positions;//list of Locations
}treeNode;

typedef struct _treeNodeListCell {
	treeNode* node;
	struct _treeNodeListCell* next;
}treeNodeListCell;

typedef struct _segment {
	treeNode* root;
}Segment;

void checkMemoryAllocation(void* ptr);
grayImage* readPGM(char* fname);
Segment findSingleSegment(grayImage* img, imgPos start, unsigned char threshold);
int** createSignMat(grayImage* img, imgPos start, unsigned char threshold);
void updateSignMatWithValues(int** signMat, unsigned char maxGrayValue, unsigned char minGrayValue, grayImage* img);
int countNeighbors(imgPos start, grayImage* img, int** signMatrix);
imgPos* NextPossibleNeighbors(imgPos start, grayImage* img, int** signMatrix);

void main()
{
	int i, j;
	grayImage* res;
	Segment test;
	res = readPGM("testPGM.pgm");
	imgPos start;
	start[0] = 0;
	start[1] = 1;

	test = findSingleSegment(res, start, 15);

	for (i = 0; i < res->rows; i++)
	{
		for (j = 0; j < res->cols; j++)
		{
			printf("%c ", res->pixels[i][j]);
		}
		printf("\n\n\n\n\n");
	}

	system("pause");
}

//grayImage
grayImage* readPGM(char* fname)
{
	//variables
	int i, j, k;
	int temp = 0;
	char** mat;
	FILE* fp;
	char line[257];
	char line2[257];
	grayImage* res;
	BOOL end = FALSE;
	char insertValue;
	res = (grayImage*)malloc(sizeof(grayImage)); //allocate grayImage 
	fp = fopen(fname, "r");

	fgets(line, 256, fp); //read and ignore from the first line 'P2'
	fscanf(fp, "%hu %hu", &res->cols, &res->rows);//may need to change the order of rows and cols
	fgets(line, 256, fp);
	fgets(line, 256, fp); //read and ignore from the first line 'max gray value'

						  //allocate the pixels matrix
	mat = (char**)malloc((res->rows) * sizeof(char*));
	for (k = 0; k < res->cols; k++)
		mat[k] = (char*)malloc((res->cols) * sizeof(char));

	//filling the matrix with values from the file

	for (i = 0; i < res->rows; i++)
	{
		for (j = 0; j < res->cols; j++)
		{
			fscanf(fp, "%d", &temp);
			insertValue = temp;
			mat[i][j] = insertValue;
		}
		fgets(line, 256, fp);
	}
	res->pixels = mat;

	return res;
}

Segment findSingleSegment(grayImage* img, imgPos start, unsigned char threshold)
{
	//Segment res;
	int** signMat;
	unsigned char grayValue, maxGrayValue, minGrayValue;
	int i, j;
	int neighbors;
	imgPos* newStartPtr;
	signMat = createSignMat(img, start, threshold);
	grayValue = (char)img->pixels[start[0]][start[1]];
	maxGrayValue = grayValue + threshold;
	minGrayValue = grayValue - threshold;
	updateSignMatWithValues(signMat, maxGrayValue, minGrayValue, img);
	
	for (i = 0; i < img->rows; i++)
	{
		for (j = 0; j < img->cols; j++)
		{
			printf("%d ", signMat[i][j]);
		}
		printf("\n");
	}

	neighbors = countNeighbors(start, img, signMat);
	newStartPtr = NextPossibleNeighbors(start, img, signMat);
	printf("neighbors are: %d\n", neighbors);
	if (newStartPtr != NULL)
	printf("next possible neigbour is: [%hu,%hu]", (*newStartPtr)[0], (*newStartPtr)[1]);

	return;
	//return res;
}

int** createSignMat(grayImage* img, imgPos start, unsigned char threshold)
{
	int** res;
	int cols, rows;
	int i;
	cols = img->cols;
	rows = img->rows;

	res = (int**)malloc(rows * sizeof(int*));
	checkMemoryAllocation(res);
	for (i = 0; i < rows; i++)
	{
		res[i] = (int*)calloc(cols, sizeof(int));
		checkMemoryAllocation(res[i]);
	}

	return res;
}

void updateSignMatWithValues(int** signMat, unsigned char maxGrayValue, unsigned char minGrayValue, grayImage* img)
{
	int i, j;
	int rows, cols;
	rows = img->rows;
	cols = img->cols;

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			if ((img->pixels[i][j] < minGrayValue) || (img->pixels[i][j]> maxGrayValue))
				signMat[i][j] = 1;
		}
	}
}


int countNeighbors(imgPos start, grayImage* img, int** signMatrix)
{
	int counter = 0;

	//Start checking the matrix from the top-left corner->top-right corner->top row->bottom-right->right  column
	//->bottom row->bottom-left column->left column.
	//1. (0,0)-
	if (start[0] == 0 && start[1] == 0)
	{
		if (signMatrix[0][1] == 0)
			counter++;
		if (signMatrix[1][1] == 0)
			counter++;
		if (signMatrix[1][0] == 0)
			counter++;
	}

	//2.(0,cols-1)
	else if (start[0] == 0 && start[1] == img->cols - 1)
	{
		if (signMatrix[1][img->cols - 1] == 0)
			counter++;
		if (signMatrix[1][img->cols - 2] == 0)
			counter++;
		if (signMatrix[0][img->cols - 2] == 0)
			counter++;
	}

	//3.First Row(0,?)
	else if (start[0] == 0)
	{
		if (signMatrix[0][start[1] + 1] == 0)
			counter++;
		if (signMatrix[1][start[1] + 1] == 0)
			counter++;
		if (signMatrix[1][start[1]] == 0)
			counter++;
		if (signMatrix[1][start[1] - 1] == 0)
			counter++;
		if (signMatrix[0][start[1] - 1] == 0)
			counter++;
	}

	//4.Bottom-right corner - (rows-1,cols-1)
	else if (start[0] == img->rows - 1 && start[1] == img->cols - 1)
	{
		if (signMatrix[img->rows - 2][img->cols - 1] == 0)
			counter++;
		if (signMatrix[img->rows - 1][img->cols - 2] == 0)
			counter++;
		if (signMatrix[img->rows - 2][img->cols - 2] == 0)
			counter++;
	}

	//5. Right column - (?, cols-1)
	else if (start[1] == img->cols - 1)
	{
		if (signMatrix[start[0] - 1][img->cols - 1] == 0)
			counter++;
		if (signMatrix[start[0] + 1][img->cols - 1] == 0)
			counter++;
		if (signMatrix[start[0] + 1][img->cols - 2] == 0)
			counter++;
		if (signMatrix[start[0]][img->cols - 2] == 0)
			counter++;
		if (signMatrix[start[0] - 1][img->cols - 2] == 0)
			counter++;
	}

	//6. Bottom-left corner - (rows-1,0)
	else if (start[0] == img->rows - 1 && start[1] == 0)
	{
		if (signMatrix[img->rows - 2][0] == 0)
			counter++;
		if (signMatrix[img->rows - 2][1] == 0)
			counter++;
		if (signMatrix[img->rows - 1][1] == 0)
			counter++;
	}

	//7. Bottom row - (rows-1,?)
	else if (start[0] == img->rows - 1)
	{
		if (signMatrix[img->rows - 2][start[1]] == 0)
			counter++;
		if (signMatrix[img->rows - 2][start[1] + 1] == 0)
			counter++;
		if (signMatrix[img->rows - 1][start[1] + 1] == 0)
			counter++;
		if (signMatrix[img->rows - 1][start[1] - 1] == 0)
			counter++;
		if (signMatrix[img->rows - 2][start[1] - 1] == 0)
			counter++;
	}

	//8. Left column - (?,0)
	else if (start[1] == 0)
	{
		if (signMatrix[start[0] - 1][0] == 0)
			counter++;
		if (signMatrix[start[0] - 1][1] == 0)
			counter++;
		if (signMatrix[start[0]][1] == 0)
			counter++;
		if (signMatrix[start[0] + 1][1] == 0)
			counter++;
		if (signMatrix[start[0] + 1][0] == 0)
			counter++;
	}

	else
	{
		if (signMatrix[start[0] - 1][start[1]] == 0)
			counter++;
		if (signMatrix[start[0] - 1][start[1] + 1] == 0)
			counter++;
		if (signMatrix[start[0]][start[1] + 1] == 0)
			counter++;
		if (signMatrix[start[0] + 1][start[1] + 1] == 0)
			counter++;
		if (signMatrix[start[0] + 1][start[1]] == 0)
			counter++;
		if (signMatrix[start[0] + 1][start[1] - 1] == 0)
			counter++;
		if (signMatrix[start[0]][start[1] - 1] == 0)
			counter++;
		if (signMatrix[start[0] - 1][start[1] - 1] == 0)
			counter++;
	}

	return counter;
}

void checkMemoryAllocation(void* ptr)
{
	if (ptr == NULL)
	{
		printf("Memory Allocation Failed!");
		exit(1);
	}
}
imgPos* NextPossibleNeighbors(imgPos start, grayImage* img, int** signMatrix, unsigned char threshold)
{
	imgPos* res;
	res = (imgPos*)malloc(sizeof(imgPos));

	//Start checking the matrix from the top-left corner->top-right corner->top row->bottom-right->right  column
	//->bottom row->bottom-left column->left column.
	//1. (0,0)-
	if (start[0] == 0 && start[1] == 0)
	{
		if (fabs((double)img->pixels[0][1] - img->pixels[0][0]) <= threshold)
//		if (signMatrix[0][1] == 0)
		{
			(*res)[0] = 0;
			(*res)[1] = 1;
			return res;
		}
		if (fabs((double)img->pixels[1][1] - img->pixels[0][0]) <= threshold)
//		if (signMatrix[1][1] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = 1;
			return res;

		}
		if (fabs((double)img->pixels[1][0] - img->pixels[0][0]) <= threshold)
//		if (signMatrix[1][0] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = 0;
			return res;
		}
	}

	//2.(0,cols-1)
	else if (start[0] == 0 && start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[1][img->cols - 1] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[1][img->cols - 1] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = img->cols - 1;
			return res;
		}
		if (fabs((double)img->pixels[1][img->cols - 2] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[1][img->cols - 2] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = img->cols - 2;
			return res;
		}
		if (fabs((double)img->pixels[0][img->cols - 2] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[0][img->cols - 2] == 0)
		{
			(*res)[0] = 0;
			(*res)[1] = img->cols - 2;
			return res;
		}
	}

	//3.First Row(0,?)
	else if (start[0] == 0)
	{
		if (fabs((double)img->pixels[0][start[1] + 1] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[0][start[1] + 1] == 0)
		{

			(*res)[0] = 0;
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[1][start[1] + 1] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[1][start[1] + 1] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[1][start[1]] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[1][start[1]] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = start[1];
			return res;
		}
		if (fabs((double)img->pixels[1][start[1] - 1] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[1][start[1] - 1] == 0)
		{
			(*res)[0] = 1;
			(*res)[1] = start[1] - 1;
			return res;
		}
		if (fabs((double)img->pixels[0][start[1] - 1] - img->pixels[0][start[1]]) <= threshold)
//		if (signMatrix[0][start[1] - 1] == 0)
		{
			(*res)[0] = 0;
			(*res)[1] = start[1] - 1;
			return res;
		}
	}

	//4.Bottom-right corner - (rows-1,cols-1)
	else if (start[0] == img->rows - 1 && start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[img->rows - 2][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][img->cols - 1] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = img->cols - 1;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 1][img->cols - 2] == 0)
		{

			(*res)[0] = img->rows - 1;
			(*res)[1] = img->cols - 2;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 2][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][img->cols - 2] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = img->cols - 2;
			return res;
		}
	}

	//5. Right column - (?, cols-1)
	else if (start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[start[0] - 1][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][img->cols - 1] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = img->cols - 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][img->cols - 1] == 0)
		{
			(*res)[0] = start[0] + 1;
			(*res)[1] = img->cols - 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][img->cols - 2] == 0)
		{
			(*res)[0] = start[0] + 1;
			(*res)[1] = img->cols - 2;
			return res;
		}
		if (fabs((double)img->pixels[start[0]][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0]][img->cols - 2] == 0)
		{
			(*res)[0] = start[0];
			(*res)[1] = img->cols - 2;
			return res;
		}
		if (fabs((double)img->pixels[start[0] - 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][img->cols - 2] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = img->cols - 2;
			return res;
		}
	}

	//6. Bottom-left corner - (rows-1,0)
	else if (start[0] == img->rows - 1 && start[1] == 0)
	{
		if (fabs((double)img->pixels[img->rows - 2][0] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][0] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = 0;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 2][1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][1] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = 1;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 1][1] == 0)
		{
			(*res)[0] = img->rows - 1;
			(*res)[1] = 1;
			return res;
		}
	}

	//7. Bottom row - (rows-1,?)
	else if (start[0] == img->rows - 1)
	{
		if (fabs((double)img->pixels[img->rows - 2][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][start[1]] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = start[1];
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 2][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][start[1] + 1] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 1][start[1] + 1] == 0)
		{
			(*res)[0] = img->rows - 1;
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 1][start[1] - 1] == 0)
		{
			(*res)[0] = img->rows - 1;
			(*res)[1] = start[1] - 1;
			return res;
		}
		if (fabs((double)img->pixels[img->rows - 2][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[img->rows - 2][start[1] - 1] == 0)
		{
			(*res)[0] = img->rows - 2;
			(*res)[1] = start[1] - 1;
			return res;
		}
	}

	//8. Left column - (?,0)
	else if (start[1] == 0)
	{
		if (fabs((double)img->pixels[start[0] - 1][0] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][0] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = 0;
			return res;
		}
		if (fabs((double)img->pixels[start[0] - 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][1] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0]][1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0]][1] == 0)
		{
			(*res)[0] = start[0];
			(*res)[1] = 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][1] == 0)
		{
			(*res)[0] = start[0];
			(*res)[1] = 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][0] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][0] == 0)
		{
			(*res)[0] = start[0] + 1;
			(*res)[1] = 0;
			return res;
		}
	}

	else
	{
		if (fabs((double)img->pixels[start[0] - 1][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][start[1]] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = start[1];
			return res;
		}
		if (fabs((double)img->pixels[start[0] - 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][start[1] + 1] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0]][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0]][start[1] + 1] == 0)
		{
			(*res)[0] = start[0];
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][start[1] + 1] == 0)
		{
			(*res)[0] = start[0] + 1;
			(*res)[1] = start[1] + 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][start[1]] == 0)
		{
			(*res)[0] = start[0] + 1;
			(*res)[1] = start[1];
			return res;
		}
		if (fabs((double)img->pixels[start[0] + 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] + 1][start[1] - 1] == 0)
		{
			(*res)[0] = start[0] + 1;
			(*res)[1] = start[1] - 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0]][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0]][start[1] - 1] == 0)
		{
			(*res)[0] = start[0];
			(*res)[1] = start[1] - 1;
			return res;
		}
		if (fabs((double)img->pixels[start[0] - 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
//		if (signMatrix[start[0] - 1][start[1] - 1] == 0)
		{
			(*res)[0] = start[0] - 1;
			(*res)[1] = start[1] - 1;
			return res;
		}
	}

	return NULL;

}

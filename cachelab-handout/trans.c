/* p20150699 JeongTaeYeong Á¤ÅÂ¿µ 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, x, y, t0,t1,t2,t3,t4,t5,t6,t7;
    
    if(M == 61 && N == 67)
    {
    	for(i=0;i<N;i+=16)
        {
         for(j=0;j<M;j+=16)
         {
            for(x=i;x<i+16&& x<N;x++)
            {
                for(y=j;y<j+16 && y<M;y++)
                {
                        B[y][x] = A[x][y]; 
                }
            } 
         }
        }
	}
    
    if(M==32 && N == 32)
    {
        for(i=0;i<N;i+=8)
        {
            for(j=0;j<M;j+=8)
         	{
            	for(x=i;x<i+8;x++)
            	{
               	 	for(y=j;y<j+8;y++)
                	{
                    	if(i!=j)
                    	{
                       		B[y][x] = A[x][y];
                    	}
                        else
                        {
                        	if( (i/8)%2 == 0)B[y][x] = A[x+8][y+8];
                        	else  	B[y][x] = A[x-8][y-8];
                    	}
                	}
            	} 
         	}
        }
        for(i=0;i<N;i+=16)
        {
            for(x=i;x<i+8;x++)
            {
                for(y=i;y<i+8;y++)
                {
                    j = B[x][y];
                    B[x][y] = B[x+8][y+8];
                    B[x+8][y+8] = j;
                }
            }
        }
    }

    if(M == 64 && N == 64)//written in visual studio style
	{
     	for (i = 0; i < 64; i += 8)
		{
			for (j = 0; j < 64; j += 8)
			{
				for (x = i; x <i + 4; x++)
				{
					/*for (y = j; y < j + 4; y++)
					{
						B[y][x] = A[x][y];
						B[y][x + 4] = A[x][y + 4];
					}*/
                    t0 = A[x][j]; t1 = A[x][j+1]; t2 = A[x][j+2]; t3 = A[x][j+3];
                    t4 = A[x][j+4]; t5 = A[x][j+5]; t6 = A[x][j+6]; t7 = A[x][j+7];
                    B[j][x] = t0; B[j][x+4] = t4;
                    B[j+1][x] = t1; B[j+1][x+4] = t5;
                    B[j+2][x] = t2; B[j+2][x+4] = t6;
                    B[j+3][x] = t3; B[j+3][x+4] = t7;

				}
	
				for (y = j; y < j + 4; y+=2)
				{
					t0 = B[y][i + 4];
					t1 = B[y][i + 5];
					t2 = B[y][i + 6];
					t3 = B[y][i + 7];
	
					B[y][i + 4] = A[i + 4][y]; t4 = A[i + 4][y + 1];
					B[y][i + 5] = A[i + 5][y]; t5 = A[i + 5][y + 1];
					B[y][i + 6] = A[i + 6][y]; t6 = A[i + 6][y + 1];
					B[y][i + 7] = A[i + 7][y]; t7 = A[i + 7][y + 1];
	
					B[y + 4][i] = t0;
					B[y + 4][i + 1] = t1;
					B[y + 4][i + 2] = t2;
					B[y + 4][i + 3] = t3;
	
					t0 = B[y+1][i + 4];
					t1 = B[y+1][i + 5];
					t2 = B[y+1][i + 6];
					t3 = B[y+1][i + 7];
	
					B[y + 1][i + 4] = t4;
					B[y + 1][i + 5] = t5;
					B[y + 1][i + 6] = t6;
					B[y + 1][i + 7] = t7;
	
					B[y + 5][i] = t0;
					B[y + 5][i + 1] = t1;
					B[y + 5][i + 2] = t2;
					B[y + 5][i + 3] = t3;
				
				}
		
	
				for (x = i + 4; x < i + 8; x++)
				{
					/*for (y = j + 4; y < j + 8; y++)
					{
						B[y][x] = A[x][y];
					} same as below code, but by using local vars, efficiency improved.*/
                    t0 = A[x][j+4]; t1 = A[x][j+5]; t2 = A[x][j+6]; t3 = A[x][j+7];
                    B[j+4][x] = t0; B[j+5][x] = t1; B[j+6][x] = t2; B[j+7][x] = t3;
				}
			}
			
		}
    }

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 



/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
    /* Register any additional transpose functions */
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}


#ifndef LOGISTIC_REGRESSION_H
#define LOGISTIC_REGRESSION_H

#include <stdio.h>
#include <limits.h>
#include <math.h>

#define PARAM_SIZE 2
static const char ABSENT = '0';
static const char PRESENT = '1';
static const char UNKNOWN = '2';

// Line delimiter can be '\n', '\r', or "\r\n"
typedef enum {newLine, carReturn, carReturnNewLine} EndOfLines;

typedef long double real;
#define STRCONV(s) strtold(s, NULL);
#define _MIN LDBL_MIN
#define _MAX LDBL_MAX
#define _EXP(s) expl(s)
#define _LOG(s) logl(s)
#define _LOG10(s) log10l(s)
#define _ERF(s) erfl(s)
#define _SQRT(s) sqrtl(s)
#define _ABS(s) fabsl(s)
#define _PRTNEUTRAL "%s%c%i%c%LG%c%LG%c%LG"
#define _PRTUNIVARIATE "%s%c%s%c%i%c%LG%c%LG%c%LG%c%LG%c%LG"


/*#define _SUFFIXE l
#define _EXP(s) exp##_SUFFIXE(s)
#define _LN(s) ln_SUFFIXE(s)
#define _ERF(s) erf_SUFFIXE(s)
#define _SQRT(s) sqrt_SUFFIXE(s)
#define _ABS(s) fabs_SUFFIXE (s)
*/




// Macro for matrices
#define MAT_POS(mat, i, j) #mat[#j][#i]

typedef struct
{
	int nbPoints;
	int nbEnv;
	int nbMark;
	int nbMarkTot;
	int numFirstMark;
	char** headerEnv;
	char** headerMark;
	real** dataEnv;
	char** maskEnv;
	char** dataMark;
	
	int comp_size;
	char* mask;
	real *X, *Y, *Xb, *new_Xb, *exp_Xb, *pi_hat, *interm, *intermScores,
		*beta_hat, *new_beta_hat, *scores,
		*J_info, *inv_J_info;
}
Computation_Variables;

typedef struct 
{
	char* name;
	char mandatory;
	char present;
	int size;
	char* contents; 
} 
ParameterSetData;

typedef struct
{
	int numMark;
	int state;
	real loglikelihood;
	real prob;
	real beta_hat;
}
ResultsNeutralModel;

typedef struct
{
	int numMark;
	int numEnv;
	int state;
	real loglikelihood;
	real Gscore;
	real WaldScore;
	real beta_hat_0;
	real beta_hat_1;
}
ResultsUnivariateModel;

typedef struct
{
	FILE* outputNeutral;
	FILE* outputUnivariate;
	char* filenameNeutral;
	char* filenameUnivariate;
	char delimWords;
	EndOfLines delimLines; 
	ResultsNeutralModel* resNeutral; // All neutral models are kept
	ResultsUnivariateModel* resUnivariate;
	int resUniSize;	// Size of array for univariate models
	int firstUniModelToWrite;
	int currentModel;
	char keepBestModels;
	real thresholdPValue;
	real thresholdGScore;
}
Results;

int initialisation(int argc, const char * argv[], const int MAX_WORD_SIZE, Computation_Variables* blackboard, Results* backup);

void initParameters(int* nbParam, ParameterSetData** param);
void eraseParameters(int* nbParam, ParameterSetData* param);
int searchEndOfLines(FILE* input, EndOfLines* delimLines); 
int readParam(FILE* input, int nbParam, ParameterSetData* param, const int max_word_size);

int readHeader(FILE* input, const int size, char** header, const char delimWords, const int max_word_size, const int nbSup, char** namesSup, char* headerMask);
int readEnv(FILE* input, Computation_Variables* blackboard, const char delimWords, const int max_word_size, const int colWidth, const char* headerMask);
int readMark(FILE* input, Computation_Variables* blackboard, const char delimWords, const int max_word_size, const int colWidth, const char* headerMask);

int createModels(Computation_Variables* blackboard, Results* backup);

void tokenize(int *size, char* input, char*** output, char delimWords, const int max_word_size);
void intToStr(int n, char** s, int* size, int leadingWhiteSpaces);

real khiSqLaw(real x);
real khiSqDensity(real x);
real invKhiSqLaw(real x, real convergenceThreshold);

// Comparison of univariate models for sorting
// qsort is ascending, we want the models with decreasing Gscore
int compareUnivariateModels(const void * p1, const void * p2);

#endif
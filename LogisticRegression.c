#include "LogisticRegression.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

int initialisation(int argc, const char * argv[], const int MAX_WORD_SIZE, Computation_Variables* blackboard, Results* backup)
{
	if ((argc != 2) && (argc != 4))
	{
		printf("Error: There are %i arguments, 2 or 4 required.\n", argc);
		return 1;
	}
	
	FILE* pf=fopen(argv[1], "r");
	if (ferror(pf))
	{
		printf("Error while opening parameter file.");
		return 2;
	}
	
	int nbParam;
	ParameterSetData* param;
	initParameters(&nbParam, &param);
	
	readParam(pf, nbParam, param, MAX_WORD_SIZE);
	
	fclose(pf);
	
	for (int i=0; i< nbParam; ++i)
	{
		printf("%s %c %c %i \n %s\n", param[i].name, param[i].mandatory, param[i].present, param[i].size, 
			   param[i].contents);
	}
	
	// Processing parameters
	for (int i=0; i< nbParam; ++i)
	{
		if (param[i].mandatory == '1' && param[i].present=='0')
		{
			printf("Error: Parameter %s is mandatory.\n", param[i].name);
			return 3;
		}
	}
	
	int currentParam=0;
	// FILENAME
	char filenameEnv[MAX_WORD_SIZE], filenameMark[MAX_WORD_SIZE];
	if (param[currentParam].present=='1')
	{
		char** filenames;
		int taille = 0;
		//printf("%s\n", param[currentParam].contents);
		tokenize(&taille, param[currentParam].contents, &filenames, ' ', MAX_WORD_SIZE);
		if (taille != 2)
		{
			printf("Error: Two data files required.\n");
			return 3;
		}
		strcpy(filenameEnv, filenames[0]);
		strcpy(filenameMark, filenames[1]);
		free(filenames[0]);
		free(filenames[1]);
	}
	else if (argc==4)
	{
		strcpy(filenameEnv, argv[2]);
		strcpy(filenameMark, argv[3]);
	}
	else
	{
		printf("Error: Data files are unspecified.\n");
		return 3;
	}
	// Creating names for outputs files
	// The names are based on the markers file
	int size=strlen(filenameMark);
	char* point=strrchr(filenameMark, '.');
	int offset=point-filenameMark;
	// "-ResNeutral" = 11 char
	// "-ResUnivariate" = 14 char
	backup->filenameNeutral = (char*) malloc((size+12) * sizeof(char));
	backup->filenameUnivariate = (char*) malloc((size+15) * sizeof(char));	
	// We know the position of the last '.' in the name
	strncpy(backup->filenameNeutral, filenameMark, offset);
	strncpy(&(backup->filenameNeutral[offset]), "-ResNeutral", 11);
	strcpy(&(backup->filenameNeutral[offset+11]), point);
	strncpy(backup->filenameUnivariate, filenameMark, offset);	
	strncpy(&(backup->filenameUnivariate[offset]), "-ResUnivariate", 14);	
	strcpy(&(backup->filenameUnivariate[offset+14]), point);
	//printf("%s %s\n", backup->filenameNeutral, backup->filenameUnivariate);
	++currentParam;
	
	// HEADERS
	char headers = '0';
	if ((param[currentParam].present=='1') && ((strcmp(param[currentParam].contents, "YES")==0) || strcmp(param[currentParam].contents, "Y")==0 || strcmp(param[currentParam].contents, "1")==0))
	{
		headers = '1';
	}
	++currentParam;
	
	// WORDDELIM
	backup->delimWords=' ';
	if (param[currentParam].present=='1' )//&& (strlen(param[currentParam].contents)==1))
	{
		printf("Chose %s\n", param[currentParam].contents);
		// The delimiter might be surrounded by backets
		int size = strlen(param[currentParam].contents);
		if (size==1)
		{
			backup->delimWords = param[currentParam].contents[0];	
		}			
		else if (size>1)
		{
			backup->delimWords = param[currentParam].contents[1];	
		}
	}
	++currentParam;
	
	// NUMVARENV
	int rawNbEnv=atoi(param[currentParam].contents);
	++currentParam;
	
	// NUMMARK
	int rawNbMark=atoi(param[currentParam].contents);
	++currentParam;
	
	// NUMMARKTOT
	if (param[currentParam].present=='1')
	{
		blackboard->nbMarkTot=atoi(param[currentParam].contents);
		char name[MAX_WORD_SIZE];
		strcpy(name, filenameMark);
		char* point=strrchr(name, '.');
		// Replace the point with '\0' to cut the string
		*point = '\0';
		char* hyphen=strrchr(name,'-');
		if (hyphen != NULL) 
		{
			blackboard->numFirstMark=atoi(hyphen);
		}
		else 
		{
			blackboard->numFirstMark=0;
		}
		
		//if (strchr(filenameMark, '.') != NULL)
	}
	else // The case NUMMARKTOT == NUMMARK is considered while processing COLSUPMARK
	{
		blackboard->nbMarkTot=-1;
		blackboard->numFirstMark=0;
	}
	++currentParam;
	
	// NUMINDIV
	blackboard->nbPoints=atoi(param[currentParam].contents);
	++currentParam;
	
	// COLSUPENV
	int nbSupEnv=0;
	char** colSupEnv;
	if (param[currentParam].present=='1')
	{
		tokenize(&nbSupEnv, param[currentParam].contents, &colSupEnv, ' ', MAX_WORD_SIZE);		
	}
	blackboard->nbEnv=rawNbEnv-nbSupEnv;
	++currentParam;
	
	// COLSUPMARK
	int nbSupMark=0;
	char** colSupMark;
	if (param[currentParam].present=='1')
	{
		tokenize(&nbSupMark, param[currentParam].contents, &colSupMark, ' ', MAX_WORD_SIZE);		
	}
	blackboard->nbMark=rawNbMark-nbSupMark;
	if (blackboard->nbMarkTot == -1) 
	{
		blackboard->nbMarkTot=blackboard->nbMark;
	}
	++currentParam;
	
	
	// SAVETYPE
	char **savingParam;
	int nbSavingParams=2;
	tokenize(&nbSavingParams, param[currentParam].contents, &savingParam, ' ', MAX_WORD_SIZE);
	if (strcmp(savingParam[0], "ALL")==0)
	{
		backup->keepBestModels='0';
		backup->thresholdPValue=1;
		backup->thresholdGScore=0;
	}
	else
	{
		backup->keepBestModels='1';
		backup->thresholdPValue=STRCONV(savingParam[1]);
		backup->thresholdGScore=invKhiSqLaw(1.-(backup->thresholdPValue/(blackboard->nbEnv*blackboard->nbMark)), _SQRT(_MIN));	
		printf("Seuil: %i %i %s %Lg \n", blackboard->nbEnv, blackboard->nbMark, savingParam[1], backup->thresholdGScore);
	}
	++currentParam;	
	
	// MEMORY
	if (param[currentParam].present=='1')
	{
		real memorySize=STRCONV(param[currentParam].contents);
		backup->resUniSize=(int) floor(memorySize*blackboard->nbEnv*sizeof(ResultsUnivariateModel)/(131072.)); // 8/(1024*1024) = 1/131072;
	}
	else
	{
		backup->resUniSize= 1000*blackboard->nbEnv;
	}
	// The arrays will be allocated when starting model computations
	++currentParam;
	
	
	// Processing environmental variables
	pf = fopen(filenameEnv, "r");
	if (ferror(pf))
	{
		printf("Error while opening environmental file.");
		return 2;
	}
	
	// Headers
	blackboard->headerEnv= (char **) malloc((blackboard->nbEnv)*sizeof(char*));
	char* headerMaskEnv= (char *) malloc((rawNbEnv+1)*sizeof(char));
	headerMaskEnv[rawNbEnv]='\0';
	if (headers == '1')
	{
		readHeader(pf, blackboard->nbEnv, blackboard->headerEnv, backup->delimWords, MAX_WORD_SIZE, nbSupEnv, colSupEnv, headerMaskEnv);
	}
	else 
	{
		// Env variables are numbered from 1 to nbEnv
		int size=0;
		int currentEnv=0;
		char* currentLabel;	// For comparisons with col sup list
		for (int i=1; i<=rawNbEnv; ++i)
		{
			// Test whether variable is in sup list
			// Caution : user's i : 1->nbEnv!!!

			char isSup='0';
			intToStr(i, &currentLabel, &size, 0);
			//printf("Label %s\n", currentLabel);
			for (int j=0; j<nbSupEnv && isSup=='0'; ++j)
			{
				if (strcmp(currentLabel, colSupEnv[j])==0)
				{
					isSup='1';
				}
			}
			if (isSup=='0')
			{
				
				headerMaskEnv[i-1]='1';
				
				intToStr(i, &(blackboard->headerEnv[currentEnv]), &size, 1);
				(blackboard->headerEnv[currentEnv])[0] = 'P';
				printf("%s\n", blackboard->headerEnv[currentEnv]);
				++currentEnv;
				
				
			}
			else 
			{
				headerMaskEnv[i-1]='0';
			}
			free(currentLabel);
			
			

		}
	}
	printf("Mask for environmental parameters:\n%s %s\n", headerMaskEnv, blackboard->headerEnv[0]);

	
	readEnv(pf, blackboard, backup->delimWords, MAX_WORD_SIZE, rawNbEnv, headerMaskEnv);
	
	fclose(pf);
	
/*	for (int i=0; i<blackboard->nbPoints; ++i)
	{
		for (int j=0; j<blackboard->nbEnv; ++j)
		{
			printf("%Lf ", blackboard->dataEnv[j][i]);
		}
		printf("\n");
	}
*/	
	// Processing genetic markers
	pf = fopen(filenameMark, "r");
	if (ferror(pf))
	{
		printf("Error while opening marker file.");
		return 2;
	}
	
	// Headers
	blackboard->headerMark= (char **) malloc((blackboard->nbMark)*sizeof(char*));
	char* headerMaskMark= (char *) malloc((rawNbMark+1)*sizeof(char));

	headerMaskMark[rawNbMark]='\0';
	if (headers == '1')
	{
		readHeader(pf, blackboard->nbMark, blackboard->headerMark, backup->delimWords, MAX_WORD_SIZE, nbSupMark, colSupMark, headerMaskMark);
			printf("Truc %s\n", headerMaskMark);
	}
	else 
	{
		// Markers are numbered from 1 to nbMark
		int size=0;
		int currentMark=0;
		char* currentLabel;	// For comparisons with col sup list
		for (int i=1; i<=rawNbMark; ++i)
		{
			// Test whether variable is in sup list
			// Caution : user's i : 1->nbMark!!!
			
			char isSup='0';
			intToStr(i+blackboard->numFirstMark, &currentLabel, &size, 0);
			//printf("Label %s\n", currentLabel);
			
			for (int j=0; j<nbSupMark && isSup=='0'; ++j)
			{
				if (strcmp(currentLabel, colSupMark[j])==0)
				{
					isSup='1';
				}
			}
			if (isSup=='0')
			{
				
				headerMaskMark[i-1]='1';
				
				intToStr(i+blackboard->numFirstMark, &(blackboard->headerMark[currentMark]), &size, 1);
				(blackboard->headerMark[currentMark])[0] = 'M';
				//printf("%s\n", blackboard->headerMark[currentMark]);
				++currentMark;
				
				
			}
			else 
			{
				headerMaskMark[i-1]='0';
			}
			free(currentLabel);
			
			//printf("%s\n", blackboard->headerMark[i-1]);
		}
	}
	
	readMark(pf, blackboard, backup->delimWords, MAX_WORD_SIZE, rawNbMark, headerMaskMark);
	fclose(pf);
/*	for (int i=0; i<blackboard->nbPoints; ++i)
	{
		for (int j=0; j<blackboard->nbMark; ++j)
		{
			printf("%c ", blackboard->dataMark[j][i]);
		}
		printf("\n");
	}*/
	
	return 0;
}


void initParameters(int* nbParam, ParameterSetData** param)
{
	*nbParam = 11;
	*param = ( ParameterSetData* )malloc(*nbParam * sizeof( ParameterSetData ));
	
	
	int position=0;
	ParameterSetData current={"", '0', '0', 0, NULL};
	
	// FILENAME
	current.name="FILENAME";
	(*param)[position]=current;
	++position;
	
	// HEADERS
	current.name="HEADERS";	
	(*param)[position]=current;
	++position;
	
	// WORDDELIM
	current.name="WORDDELIM";
	//current.removeWhiteSpaces='1';
	(*param)[position]=current;
	++position;
	
	// NUMVARENV
	current.name="NUMVARENV";	
	current.mandatory='1';
	(*param)[position]=current;
	++position;
	
	// NUMMARK
	current.name="NUMMARK";	
	(*param)[position]=current;
	++position;
	
	// NUMMARKTOT
	current.name="NUMMARKTOT";
	current.mandatory='0';
	(*param)[position]=current;
	++position;
	
	// NUMINDIV
	current.mandatory='1';
	current.name="NUMINDIV";	
	(*param)[position]=current;
	++position;
	
	// COLSUPENV
	current.name="COLSUPENV";
	current.mandatory='0';
	//current.removeWhiteSpaces='0';
	(*param)[position]=current;
	++position;
	
	// COLSUPMARK
	current.name="COLSUPMARK";	
	(*param)[position]=current;
	++position;
	
	// SAVETYPE
	current.name="SAVETYPE";	
	current.mandatory='1';
	(*param)[position]=current;
	++position;
	
	// MEMORY
	current.name="MEMORY";	
	current.mandatory='0';
	(*param)[position]=current;
	++position;
	
}

void eraseParameters(int* nbParam, ParameterSetData* param)
{
	for (int i=0; i<*nbParam; ++i)
	{
		free(param[i].contents);
	}
	free(param);
}

int searchEndOfLines(FILE* input, EndOfLines* delimLines)
{
	char read='0', nextChar='0';
	int found=0;
	while (found==0 && (read=fgetc(input))!=EOF) 
	{
		if (read == '\n')
		{
			*delimLines=newLine;
			found = 1;
		}
		else if (read == '\r')
		{
			nextChar=fgetc(input);
			if (nextChar=='\n') 
			{
				*delimLines=carReturnNewLine;
				found = 1;
			}
			else
			{
				*delimLines=carReturn;
				found = 1;
			}
		}
	}
	rewind(input);
	return 1-found;
}


int readParam(FILE* input, int nbParam, ParameterSetData* param, const int max_word_size)
{
	char word[max_word_size];
	int position=0;
	char read='0';
	char delimWords=' ';
	int expectingTag = 1;
	int currentTag=0;
	
	while ((read=fgetc(input))!=EOF)
	{
		if ((read==delimWords && expectingTag==1) || (read<=0x19) || (read>=0x7F))
		{
			if (position>0)
			{
				word[position]='\0';
				// Processing
				if (expectingTag==1)
				{
					// Test whether word is a valid tag
					currentTag=-1;
					for (int i=0; i<nbParam && currentTag == -1; ++i)
					{
						if (strcmp(word, param[i].name)==0)
						{
							currentTag=i;
						}
					}
					if (currentTag==-1)
					{
						printf("Unknown tag: %s", word);
						exit(1); // unknown tag
					}
					else 
					{
						expectingTag=0;
					}
				}
				else // looking for value
				{
					param[currentTag].contents = (char *) malloc((position+1)*sizeof(char));
					strcpy(param[currentTag].contents, word);
					param[currentTag].size=position;
					param[currentTag].present='1';
					expectingTag=1;
				}
				
				position=0;
			}			
		}
		else
		{
			word[position]=read;
			++position;
		}
	}
	return 0;
}

int readHeader(FILE* input, const int size, char** header, const char delimWords, const int max_word_size, const int nbSup, char** namesSup, char* headerMask)
{
	const int totalHeaderSize = size + nbSup;
	int nbWords = 0;
	int nbWordsKept= 0;
	char word[max_word_size];
	int position=0;
	char read='0';
	
	int inner = 0; 
	
	while (nbWords<totalHeaderSize && (read=fgetc(input))!=EOF)
	{
		//putchar(read);
		if (read=='"')
		{
			inner=1-inner;
		}
		if (((read==delimWords || read==' ') && inner==0) || (read<=0x19) || (read>=0x7F))
		{
			if (position>0)
			{
				word[position]='\0';
				// Processing
				// Test whether word is in sup list
				char isSup='0';
				for (int i=0; i<nbSup && isSup=='0'; ++i)
				{
					if (strcmp(word, namesSup[i])==0)
					{
						isSup='1';
					}
				}
				if (isSup=='0')
				{
					header[nbWordsKept] = (char *) malloc(position*sizeof(char));
					strcpy(header[nbWordsKept], word);
					headerMask[nbWords]='1';
					++nbWordsKept;
				}
				else 
				{
					headerMask[nbWords]='0';
				}
				
				position=0;
				++nbWords;
			}			
		}
		else
		{
			word[position]=read;
			++position;
		}
	}
	if (read == EOF)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

int readEnv(FILE* input, Computation_Variables* blackboard, const char delimWords, const int max_word_size, const int colWidth, const char* headerMask)
{
	const int dataSize = blackboard->nbPoints*blackboard->nbEnv;
	
	// Memory allocation !
	blackboard->dataEnv= (real**) malloc(blackboard->nbEnv*sizeof(real*));
	blackboard->maskEnv= (char**) malloc(blackboard->nbEnv*sizeof(char*));
	for (int i=0; i<blackboard->nbEnv; ++i)
	{
		blackboard->dataEnv[i] = (real*) malloc(blackboard->nbPoints*sizeof(real));
		blackboard->maskEnv[i] = (char*) malloc(blackboard->nbPoints*sizeof(char));
	}
	
	int nbWords = 0;
	char word[max_word_size];
	int position=0;
	char read='0';
	real value = 0;
	
	int numLine = 0;
	int numCol = 0;
	int nbReadOnLine = 0;
	
	while ((read=fgetc(input))!=EOF && nbWords<dataSize)
	{
		//putchar(read);
		if (read==delimWords || (read<=0x20) || (read>=0x7F))
		{
		
			if (position>0)
			{
				word[position]='\0';
				if (headerMask[numCol]=='1')
				{
					// Processing
					value = STRCONV(word);
					if (isnan(value))
					{
						blackboard->dataEnv[nbReadOnLine][numLine] = 0;
						blackboard->maskEnv[nbReadOnLine][numLine] = UNKNOWN;
					}
					else
					{
						//printf("$ %i %i %Lf\n", nbReadOnLine, numLine, value);
						blackboard->dataEnv[nbReadOnLine][numLine] = value;
						blackboard->maskEnv[nbReadOnLine][numLine] = PRESENT;						
					}
					++nbReadOnLine;
					++nbWords;
					
				}
				++numCol;
				
				if (numCol==colWidth)
				{
					++numLine;
					nbReadOnLine=0;
					numCol=0;
				}
				position=0;
				
			}			
		}
		else
		{
			word[position]=read;
			++position;
		}
	}
	if (read == EOF)
	{
		// Processing last input
		if (position>0)
		{
			word[position]='\0';
			
			if (headerMask[numCol]=='1')
			{
				// Processing
				value = STRCONV(word);
				if (isnan(value))
				{
					blackboard->dataEnv[nbReadOnLine][numLine] = 0;
					blackboard->maskEnv[nbReadOnLine][numLine] = UNKNOWN;
				}
				else
				{
					blackboard->dataEnv[nbReadOnLine][numLine] = value;
					blackboard->maskEnv[nbReadOnLine][numLine] = PRESENT;						
				}
				++nbReadOnLine;
				
			}
			++numCol;
			
		}	
		return 1;
	}
	else 
	{
		return 0;
	}
	
}

int readMark(FILE* input, Computation_Variables* blackboard, const char delimWords, const int max_word_size, const int colWidth, const char* headerMask)
{
	const int dataSize = blackboard->nbPoints*blackboard->nbMark;
	
	// Memory allocation !
	blackboard->dataMark= (char**) malloc(blackboard->nbMark*sizeof(char*));
	for (int i=0; i<blackboard->nbMark; ++i)
	{
		blackboard->dataMark[i] = (char*) malloc(blackboard->nbPoints*sizeof(char));
	}
	
	
	int nbWords = 0;
	char word[max_word_size];
	int position=0;
	char read='0';
	
	int numLine = 0;
	int numCol = 0;
	int nbReadOnLine = 0;
	
	while ((read=fgetc(input))!=EOF && nbWords<dataSize)
	{
		//putchar(read);
		if (read==delimWords || (read<=0x20) || (read>=0x7F))
		{
			if (position>0)
			{
				word[position]='\0';				
				if (headerMask[numCol]=='1')
				{
					// Traitement
					if ((strcmp(word, "0"))==0)
					{
						blackboard->dataMark[nbReadOnLine][numLine] = ABSENT;
					}
					else if (strcmp(word, "1")==0)
					{
						blackboard->dataMark[nbReadOnLine][numLine] = PRESENT;
					}
					else 
					{
						blackboard->dataMark[nbReadOnLine][numLine] = UNKNOWN;
					}
					//	printf("$ %i %i %s %c\n", nbReadOnLine, numLine, word, blackboard->dataMark[nbReadOnLine][numLine]);
					++nbReadOnLine;
					++nbWords;
				}
				++numCol;
				if (numCol==colWidth)
				{
					++numLine;
					nbReadOnLine=0;
					numCol=0;
				}
				position=0;

			}			
		}
		else
		{
			word[position]=read;
			++position;
		}
	}
	
	if (read == EOF)
	{
		// Processing last input
		if (position>0)
		{
			word[position]='\0';
			
			if (headerMask[numCol]=='1')
			{
				// Processing
				if ((strcmp(word, "0"))==0)
				{
					blackboard->dataMark[nbReadOnLine][numLine] = ABSENT;
				}
				else if ((strcmp(word, "1"))==0)
				{
					blackboard->dataMark[nbReadOnLine][numLine] = PRESENT;
				}
				else 
				{
					blackboard->dataMark[nbReadOnLine][numLine] = UNKNOWN;
				}
				++nbReadOnLine;
			}
			++numCol;
		}		
		
		return 1;
	}
	else 
	{
		return 0;
	}
	
}

// Possible errors in models:
// 1: exponential divergence
// 2: J_info is singular
// 3: divergence of beta
// 4: iterations limit is reached
// 5: constant marker
int createModels(Computation_Variables* blackboard, Results* backup)
{
	// Open output files
	backup->outputNeutral=fopen(backup->filenameNeutral, "w");
	backup->outputUnivariate=fopen(backup->filenameUnivariate, "w");
	if (ferror(backup->outputNeutral)!=0 || ferror(backup->outputUnivariate)!=0 )
	{
		printf("Error while opening output files.");
		return 1;
	}
	
	// Allocate results arrays
	// All neutral models are kept
	backup->resNeutral = (ResultsNeutralModel *) malloc(blackboard->nbMark*sizeof(ResultsNeutralModel));
	backup->resUnivariate = (ResultsUnivariateModel *) malloc(backup->resUniSize*sizeof(ResultsUnivariateModel));
	
	backup->firstUniModelToWrite=0;
	backup->currentModel=0;
	
	// Initialising computational data
	blackboard->comp_size=2;
	blackboard->mask = (char *) malloc(blackboard->nbPoints*sizeof(char));
	
	int realSize=sizeof(real);
	blackboard->X = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->Y = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->Xb = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->new_Xb = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->exp_Xb = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->pi_hat = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->interm = (real *) malloc(blackboard->nbPoints*realSize);
	blackboard->intermScores = (real *) malloc(blackboard->nbPoints*realSize);
	
	blackboard->beta_hat = (real *) malloc(blackboard->comp_size*realSize);
	blackboard->new_beta_hat = (real *) malloc(blackboard->comp_size*realSize);
	blackboard->scores = (real *) malloc(blackboard->comp_size*realSize);
	
	// WARNING : J_info and inv_J_info are 2x2 (column major) matrices represented as vectors!
	// J_info[0][0] -> 0, J_info[1][0] -> 1, J_info[0][1] -> 2, J_info[1][1] -> 3,
	blackboard->J_info = (real *) malloc(blackboard->comp_size*blackboard->comp_size*realSize);
	blackboard->inv_J_info = (real *) malloc(blackboard->comp_size*blackboard->comp_size*realSize);
	
	
	
	ResultsNeutralModel* currentNeutral;
	ResultsUnivariateModel* currentUnivariate;
	
	// Number of markers present and size of the current model (1->nbPoints)
	// Allow checks for monomorphism
	real sumY=0, sizeY=0;	
	int currentSize = 0; 
	
	const int nbIterMax = 100; // Limit of iterations
	const real limitExp = ( _LOG( _MAX / 2. ) < 700 ? _LOG( _MAX / 2. ): 700.0),
	convCriterion=1e-08, minDet=1e-10;
	int numIter=0, continueComputation=0;
	real maxXb=0, det_J_info;
	
	for (int i=0; i<blackboard->nbMark; ++i)
	{
		currentNeutral = &(backup->resNeutral[i]);
		currentNeutral->numMark=i;
		
		sumY=0;
		sizeY=0;
		for (int j=0; j<blackboard->nbPoints; ++j)
		{
			if (blackboard->dataMark[i][j] == PRESENT) 
			{
				++sizeY;
				++sumY;
			}
			else if (blackboard->dataMark[i][j] == ABSENT) 
			{
				++sizeY;
			}
		}
		
		// Test whether marker is monomorphic
		if (sumY == 0 || sizeY == 0 || sumY == sizeY)
		{
			currentNeutral->state=5; // constant model
			currentNeutral->prob=(sumY>0 ? 1 : 0);
			currentNeutral->loglikelihood=0;
			currentNeutral->beta_hat=0;
		}
		else	
			// compute neutral model
		{
			currentNeutral->state=0; // No error
			currentNeutral->prob=(sumY/sizeY);
			currentNeutral->beta_hat=log(sumY /(sizeY-sumY) );
			//printf("%i %Lg\n", i, sizeY / (sizeY-sumY));
			currentNeutral->loglikelihood=sumY*currentNeutral->beta_hat - sizeY*logl((sizeY) / (sizeY-sumY));
			
		}
		// Results are written below
		
		// Compute univariate models if no error has occured
		if (currentNeutral->state==0)
		{
			// For each environmental variable
			for (int j=0; j<blackboard->nbEnv; ++j)
			{
				currentUnivariate = &(backup->resUnivariate[backup->currentModel]);
				
				currentUnivariate->state=0;
				currentUnivariate->numMark=i;
				currentUnivariate->numEnv=j;
				currentUnivariate->loglikelihood=0;
				currentUnivariate->Gscore=0;
				currentUnivariate->WaldScore=0;
				currentUnivariate->beta_hat_0=0;
				currentUnivariate->beta_hat_1=0;
				
				// Initialising variables
				currentSize=0;
				sumY=0;
				// Check whether samples are valid
				for (int k=0; k<blackboard->nbPoints; ++k)
				{
					if (blackboard->dataMark[i][k]!=UNKNOWN && blackboard->maskEnv[j][k]!=UNKNOWN)
					{
						blackboard->Y[currentSize] = (blackboard->dataMark[i][k]==PRESENT ? 1 : 0);
						blackboard->X[currentSize] = blackboard->dataEnv[j][k];
						sumY+=(blackboard->Y[currentSize]);
						++currentSize;
					}
				}
				// Check for monomorphism
				if (sumY == 0 || currentSize==0 || sumY==currentSize)
				{
					// In case of monomorphism, model is kept if all results are saved
					if (backup->keepBestModels=='0')
					{
						currentUnivariate->state=5;
						++backup->currentModel;
					}
				}
				else	// Actual regression
				{
					blackboard->beta_hat[0]=0;
					blackboard->beta_hat[1]=0;
					for (int l=0; l<currentSize; ++l)
					{
						blackboard->Xb[l]=0.;
						blackboard->exp_Xb[l]=0.;
					}
					numIter=0;
					continueComputation=1;
					
					while (continueComputation==1 && (numIter < nbIterMax))
					{
						maxXb=0;
						for (int l=0; l<currentSize; ++l)
						{
							blackboard->new_Xb[l]=blackboard->beta_hat[0]+blackboard->beta_hat[1]*blackboard->X[l];
							if (_ABS(blackboard->new_Xb[l]) > maxXb) 
							{
								maxXb = _ABS(blackboard->new_Xb[l]);
							}
						}
						if (maxXb >= limitExp) // Divergence detected
						{
							continueComputation=0;
							currentUnivariate->state=1;			
							continue;
						}
						
						// No divergence
						++numIter;
						
						// Compute pi and intermediate results
						blackboard->scores[0]=0.;
						blackboard->scores[1]=0.;
						blackboard->J_info[0]=0;
						blackboard->J_info[2]=0;
						blackboard->J_info[1]=0;
						blackboard->J_info[3]=0;
						
						for (int l=0; l<currentSize; ++l)
						{
							blackboard->Xb[l] = blackboard->new_Xb[l];
							blackboard->exp_Xb[l] =_EXP(blackboard->Xb[l]);
							blackboard->pi_hat[l] = blackboard->exp_Xb[l] / (1. + blackboard->exp_Xb[l]); 
							blackboard->interm[l] = blackboard->pi_hat[l] * (1. - blackboard->pi_hat[l]);
							blackboard->intermScores[l] = blackboard->Y[l] - blackboard->pi_hat[l];
							blackboard->scores[0]+=blackboard->intermScores[l];
							blackboard->scores[1]+=blackboard->intermScores[l]*blackboard->X[l];
							
							blackboard->J_info[0]+=blackboard->interm[l];
							blackboard->J_info[1]+=blackboard->interm[l]*blackboard->X[l];
							blackboard->J_info[3]+=blackboard->interm[l]*blackboard->X[l]*blackboard->X[l];
						}
						// J_info is symmetric
						blackboard->J_info[2] = blackboard->J_info[1];
						
						det_J_info = blackboard->J_info[0]*blackboard->J_info[3] - blackboard->J_info[1]*blackboard->J_info[2];
						
						if (_ABS(det_J_info) < minDet)
						{
							// J_info is singular
							continueComputation=0;
							currentUnivariate->state=2;
							continue;
						}
						
						// Compute inverse of J_info
						blackboard->inv_J_info[0] = blackboard->J_info[3] / det_J_info;
						blackboard->inv_J_info[3] = blackboard->J_info[0] / det_J_info;
						blackboard->inv_J_info[2] = -blackboard->J_info[2] / det_J_info;
						blackboard->inv_J_info[1] = blackboard->inv_J_info[2];
						
						// new_beta_hat = beta_hat + inv_J_info * scores
						blackboard->new_beta_hat[0] = blackboard->beta_hat[0] + blackboard->inv_J_info[0]*blackboard->scores[0] + blackboard->inv_J_info[2]*blackboard->scores[1];
						blackboard->new_beta_hat[1] = blackboard->beta_hat[1] + blackboard->inv_J_info[1]*blackboard->scores[0] + blackboard->inv_J_info[3]*blackboard->scores[1];
						
						if ( _ABS(blackboard->new_beta_hat[0]-blackboard->beta_hat[0]) < ( convCriterion * ( (_MIN > _ABS(blackboard->beta_hat[0])) ? _MIN :  _ABS(blackboard->beta_hat[0]))) 
							&&	 _ABS(blackboard->new_beta_hat[1]-blackboard->beta_hat[1]) < ( convCriterion * ( (_MIN > _ABS(blackboard->beta_hat[1])) ? _MIN :  _ABS(blackboard->beta_hat[1])))  )
						{
							// Convergence achieved
							continueComputation=0;
							currentUnivariate->beta_hat_0=blackboard->new_beta_hat[0];
							currentUnivariate->beta_hat_1=blackboard->new_beta_hat[1];
						}
						else
						{
							blackboard->beta_hat[0]=blackboard->new_beta_hat[0];
							blackboard->beta_hat[1]=blackboard->new_beta_hat[1];
						}
						
					}
					
					if (numIter==nbIterMax)
					{
						currentUnivariate->state=4;
					}
					
					// Loglikelihood is computed in every case
					for (int l=0; l<currentSize; ++l)
					{
						currentUnivariate->loglikelihood+=(blackboard->Y[l]*blackboard->Xb[l] - _LOG(1.+blackboard->exp_Xb[l]));
					}
					
					// Scores are computed if the computation is converged
					if (currentUnivariate->state==0)
					{
						currentUnivariate->Gscore = 2.0 * (currentUnivariate->loglikelihood - currentNeutral->loglikelihood);
						currentUnivariate->WaldScore = currentUnivariate->beta_hat_1 * currentUnivariate->beta_hat_1 / blackboard->inv_J_info[3];
						if (((currentUnivariate->Gscore >= backup->thresholdGScore) &&  currentUnivariate->WaldScore >= backup->thresholdGScore) || (backup->keepBestModels=='0'))
						{
							++(backup->currentModel);
						}
					}
					
				}
					
			}
			// Writing results!!!!
			// Results are written if the number of remaining avaible slots for univariate models is less than nbEnv 
			// We also write the results if the current marker is the last one ;)
			if ( ((backup->resUniSize-backup->currentModel-1) < (blackboard->nbEnv)) || (i==(blackboard->nbMark-1) ) )
			{
				/*for (int truc=0; truc<blackboard->nbEnv; ++ truc)
				{
						printf("P %i : %s\n", truc, blackboard->headerEnv[truc]);
				}*/
				// Writting neutral models
				for (int l=backup->firstUniModelToWrite; l<=i; ++l)
				{
					fprintf(backup->outputNeutral, _PRTNEUTRAL, blackboard->headerMark[backup->resNeutral[l].numMark], backup->delimWords, backup->resNeutral[l].state, backup->delimWords, 
							backup->resNeutral[l].loglikelihood, backup->delimWords, backup->resNeutral[l].prob, backup->delimWords, backup->resNeutral[l].beta_hat);
					
					// Add end of line
					switch (backup->delimLines)
					{
						case newLine:
							fprintf(backup->outputNeutral, "\n");
							break;
						case carReturn:
							fprintf(backup->outputNeutral, "\r");
							break;
						default:
							fprintf(backup->outputNeutral, "\r\n");
							break;
					}
					
				}
				backup->firstUniModelToWrite=i+1;
				
				// Sorting univariate models
				qsort(backup->resUnivariate, (backup->currentModel), sizeof(ResultsUnivariateModel), compareUnivariateModels);
				
				for (int l=0; l<=(backup->currentModel); ++l)
				{
					fprintf(backup->outputUnivariate, _PRTUNIVARIATE, blackboard->headerMark[backup->resUnivariate[l].numMark], backup->delimWords, blackboard->headerEnv[backup->resUnivariate[l].numEnv], backup->delimWords, backup->resUnivariate[l].state, backup->delimWords,
							backup->resUnivariate[l].loglikelihood, backup->delimWords, backup->resUnivariate[l].Gscore, backup->delimWords, backup->resUnivariate[l].WaldScore, backup->delimWords, 
							backup->resUnivariate[l].beta_hat_0, backup->delimWords, backup->resUnivariate[l].beta_hat_1); 	
					
					
					// Add end of line
					switch (backup->delimLines) 
					{
						case newLine:
							fprintf(backup->outputUnivariate, "\n");
							break;
						case carReturn:
							fprintf(backup->outputUnivariate, "\r");
							break;
						default:
							fprintf(backup->outputUnivariate, "\r\n");
							break;
					}
					
				}
				backup->currentModel=0;
			}		
			
		}
		
	}
	
	
	
	
	
	free(blackboard->mask);
	free(blackboard->X);
	free(blackboard->Y);
	free(blackboard->Xb);
	free(blackboard->exp_Xb);
	free(blackboard->pi_hat);
	free(blackboard->interm);
	free(blackboard->intermScores);
	free(blackboard->beta_hat);
	free(blackboard->new_beta_hat);
	free(blackboard->scores);
	free(blackboard->J_info);
	free(blackboard->inv_J_info);
	
	free(backup->resNeutral);
	free(backup->resUnivariate);
	free(backup->filenameNeutral);
	free(backup->filenameUnivariate);
	
	fclose(backup->outputNeutral);
	fclose(backup->outputUnivariate);
	return 0;
}


void tokenize(int *size, char* input, char*** output, char delimWords, const int max_word_size)
{
	if (*size > 0)
	{
		*output = (char**) calloc(*size, sizeof(char*));			
	}
	else
	{
		*output = (char**) calloc(0, sizeof(char*));			
	}
	
	int nbWords = 0;
	char word[max_word_size];
	int length=0;	// length of word
	
	int position=0;  // char currently read in input
	char read='0';
	
	
	int inner = 0; 
	
	read=input[position];
	while (read!='\0')
	{
		//putchar(read);
		if (read=='"')
		{
			inner=1-inner;
		}
		//if (((read==delimWords || read==' ') && inner==0) || (read<=0x19) || (read>=0x7F))
		if (read==delimWords && inner==0) // end of a word
		{
			if (length>0)
			{
				word[length]='\0';
				// Processing
				// Test whether word is in sup list
				if (nbWords>=*size)
				{
					*output = (char**) realloc(*output, (nbWords+1)*sizeof(char*));			
				}
				
				(*output)[nbWords] = (char *) malloc(length*sizeof(char));
				strcpy((*output)[nbWords], word);
				
				length=0;
				++nbWords;
			}
		}
		else if (/*(read == ' ' && inner ==1) || */(read>0x19 && read<0x7F) )
		{
			word[length]=read;
			++length;
			
		}
		++position;
		read=input[position];
		
	}
	// Processing last word
	if (length>0)
	{
		word[length]='\0';
		// Processing
		// Test whether word is in sup list
		if (nbWords>=*size)
		{
			*output = (char**) realloc(*output, (nbWords+1)*sizeof(char*));			
		}
		
		(*output)[nbWords] = (char *) malloc(length*sizeof(char));
		strcpy((*output)[nbWords], word);
		
		length=0;
		++nbWords;
	}
	
	*size = nbWords;
	
}

void intToStr(int n, char** s, int* size, int leadingWhiteSpaces)
{
	int isNeg=0;
	if (n<0) 
	{
		n=-n;
		isNeg=1;
	}
	int numDigits=1;
	if (n>0)
	{
		numDigits=ceil(log10((double)(n+1)));
	}
	*size = numDigits+1+isNeg+leadingWhiteSpaces; // +1 for the end of the string
	*s = (char *) malloc(*size * sizeof(char));
	int digit;
	(*s)[*size-1]='\0';
	for(int position = *size-2; position>=(leadingWhiteSpaces+isNeg); --position)
	{
		digit = n%10;
		
		(*s)[position] = 48 + digit;
		n = (n-digit)/10;
	}
	if (isNeg==1)
	{
		(*s)[leadingWhiteSpaces]='-';
	}
	for (int position = 0; position<leadingWhiteSpaces; ++position)
	{
		(*s)[position]=' ';
	}
}

inline real khiSqLaw(real x)
{
	return _ERF( _SQRT( x/2. ) );
}

inline real khiSqDensity(real x)
{
	const real valeur_2_pi = 4* asin(1.0);
	return (_EXP(-x/2.))/_SQRT(valeur_2_pi*x);
}

real invKhiSqLaw(real x, real convergenceThreshold)
{
	real score=_MIN;
	real residual = khiSqLaw(score)-x;
	int counter=0, limitIter=1000;
	do
	{
		//printf("* %Lg %Lg %Lg\n", score, khiSqLaw(score), khiSqDensity(score));
		
		score = score - (khiSqLaw(score)-x)/khiSqDensity(score);
		/*if (score<0) 
		 {
		 score=_MIN;
		 }*/
		residual=khiSqLaw(score)-x;
		//printf("%Lg %Lg\n", score, khiSqLaw(score));
		++counter;
	}
	while ((_ABS(residual)>convergenceThreshold) && (counter<limitIter));
	//printf("invKhi( %Lg ) = %Lg\n",x, score);
	return score;
}

int compareUnivariateModels(const void * p1, const void * p2)
{
	ResultsUnivariateModel* m1 = (ResultsUnivariateModel*) p1, * m2 = (ResultsUnivariateModel*) p2;
	return (m1->Gscore > m2->Gscore ? 1 : ( m1->Gscore < m2->Gscore ? -1 : 0 ) );
}

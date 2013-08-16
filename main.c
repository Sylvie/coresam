#include "LogisticRegression.h"
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <time.h>

int main (int argc, const char * argv[])
{
	/*
	printf("%Lg %Lg %Lg \n", khiSqLaw(3.841), khiSqLaw(5.024), khiSqLaw(6.635));
	printf("%Lg %Lg %Lg \n", khiSqDensity(1), khiSqDensity(2), khiSqDensity(4));

	printf("%Lg %Lg %Lg \n", invKhiSqLaw(0.95,1e-06), invKhiSqLaw(0.99,1e-06), invKhiSqLaw(.999, 1e-06));
	printf("%Lg %Lg %Lg \n", khiSqLaw(invKhiSqLaw(.95,1e-06)), khiSqLaw(invKhiSqLaw(.99,1e-06)), khiSqLaw(invKhiSqLaw(.999,1e-06)));
	printf("%g %g %Lg \n", DBL_MIN, sqrt( DBL_MIN ), invKhiSqLaw(1.-1./(90900), 1e-10));
	*/
	/*
	 int nbEnv=3+9*13, nbMark=1000;//3000000*10;
	int nbPoints=1000;
	printf("%lu \n", (long)floor(8.*(nbEnv*(sizeof(real)+sizeof(char))+nbMark*sizeof(char))*nbPoints/(1024*1024)));
	printf("%lu %lu %lu\n", 5*sizeof(real), sizeof(ResultsNeutralModel), (long)floor(8.*nbMark*sizeof(ResultsNeutralModel)/(1024*1024)));
	printf("%lu %lu %lu\n", 8*sizeof(real), sizeof(ResultsUnivariateModel), (long)floor(8.*nbEnv*nbMark*sizeof(ResultsUnivariateModel)/(1024*1024)));
	*/
	/*
	int tab[8] = {-0, 1, 99, 1000, -1, -9, -1000, 12.5};
	for (int i=0; i<8; ++i)
	{
		char* chaine;
		int taille;
		intToStr(tab[i], &chaine, &taille, 2);
		printf("\"%s\"\n", chaine);
		free(chaine);
	}
	 */
	const int MAX_WORD_SIZE=10000;
	
	Computation_Variables blackboard;
	Results backup;
	
	printf("***%s\n***%s\n***%s\n", argv[0], argv[1], argv[2]);
	initialisation(argc, argv, MAX_WORD_SIZE, &blackboard, &backup);
	
	time_t time_start=time(NULL);
	createModels(&blackboard, &backup);
	time_t time_end=time(NULL);
	printf("Time for computations : %g\n", difftime(time_end, time_start));
	
    return 0;
}

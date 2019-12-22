//Implements  x(t) = A*exp(-k/t)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char** argv) {
	
	int i, nsteps;
	double step, x,a,T,k;
	double dur;

	if (argc != 4) {printf(" Usage: expdecay duration T(decay rate) steps(how many points will be generated). 3 arguments");
							return 1;}

	dur = atof(argv[1]);
	T = atof(argv[2]);
	nsteps = atof(argv[3]);

	k= dur/nsteps;
	a= exp(-k/T);
	x = 1.0;
	step=0.0;

	for (i =0; i< nsteps; i++){
	printf("%.41f\t%.81f\n", step, x);
	x= a*x;
	step += k;
	}
	return 0;
}
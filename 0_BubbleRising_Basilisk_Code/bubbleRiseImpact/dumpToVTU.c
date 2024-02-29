#include "navier-stokes/centered.h"
#include "output_vtu_foreach.h"
#include "two-phase.h"
#include "tension.h"

double tmax=***;
double tsnap=***;
int main(int argc, char const *argv[]) {
    mkdir("paraviewResults", 0700);
    for(double t_real = 0.0 ; t_real < tmax; t_real=t_real+ tsnap)
    {
        char nameOut[80];
        sprintf (nameOut, "Intermediate/snapshot-%5.4f", t_real);
        if (restore(file=nameOut)){
            char vtkOut[80];
            sprintf (vtkOut, "paraviewResults/out-%5.4f-CC", t_real/tsnap*10);
            output_vtu((scalar *){f,p}, (vector *){u}, vtkOut); 
            fprintf (ferr, "snapshot-%5.4f have been converted to out-%5.4f-CC\n", t_real,t_real/tsnap*10);
        }else{
            continue;
        }
    }
}



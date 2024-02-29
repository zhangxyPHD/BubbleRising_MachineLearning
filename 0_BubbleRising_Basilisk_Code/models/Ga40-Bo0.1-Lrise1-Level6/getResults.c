#include "axi.h"
#include "navier-stokes/centered.h"
#define FILTERED
#define mu(f)  (1./(clamp(f,0,1)*(1./mu1 - 1./mu2) + 1./mu2))
#include "two-phase.h"
#include "tension.h"
#include "output_boundary.h"

double domainSize,L_rise;
#define circleDrop (1.0-sqrt(sq(x - (-L_rise-1.0)) + sq(y)))  

#define rhoRatio (0.001)
#define mu_ratio (0.01)

//refine level
int radiusLevel,basicLevel;
#define minLevel (basicLevel+2) //  minimum resolution level
#define maxLevel (basicLevel+radiusLevel) //  maximum resolution level
double tmax;
double tsnap=0.1;


//boundary
u.n[top] = neumann(0.);
p[top] = neumann(0.);
u.n[right]=dirichlet(0.0);
u.t[right]=dirichlet(0.0);
f[right] = dirichlet(0.0);
u.n[left] = neumann(0.);
u.t[left] = neumann(0.);
p[left] = dirichlet(1.0*domainSize);
pf[left] = dirichlet(1.0*domainSize);


// Error tolerances
#define fErr (1e-3)                                 // error tolerance in VOF
#define KErr (1e-6)                               // error tolerance in KAPPA
#define VelErr (1e-2)                            // error tolerances in velocity
#define DissErr (1e-2)                            // error tolerances in dissipation
#define OmegaErr (1e-2)                            // error tolerances in velocity

double Galilei,Bond;

int main(int a, char const *argv[]) {
  Galilei=atof(argv[1]);
  Bond=atof(argv[2]);
  domainSize=atof(argv[3]);
  L_rise=atof(argv[4]);
  basicLevel=atof(argv[5]);
  radiusLevel=atof(argv[6]);
  tmax=atof(argv[7]);
  tsnap=atof(argv[8]);

  L0=domainSize;
  origin(-L0, 0.0);
  init_grid (1 << (basicLevel)); 

  //可以避免速度出现不正常跳动
  TOLERANCE=1e-6;
  NITERMAX=300;
  NITERMIN=10;

  rho2 = 1.;
  rho1 = rhoRatio;
  mu2 = 1. / Galilei;
  mu1 = mu_ratio * mu2;
  f.sigma = 1. / Bond;

  FILE *fp_shape;
  char shapeResults[200];
  sprintf (shapeResults, "results.dat");
  fp_shape = fopen (shapeResults, "w");
  
  for(double t_real = 0.0 ; t_real < tmax+ tsnap; t_real=t_real+ tsnap)
  {
    char nameOut[80];
    sprintf (nameOut, "Intermediate/snapshot-%5.4f", t_real);
    if (restore(file=nameOut)){
      double x_max,x_min,y_max,seAll,ratio;
      output_boundary(f,&x_max,&x_min,&y_max,&seAll);
      ratio=2*y_max/(x_max-x_min);

      double xb = 0., vb = 0.,sb = 0.;
      foreach (reduction(+: xb) reduction(+ : vb) reduction(+ : sb))
      {
        double dv = (2*pi*y)* clamp(f[], 0., 1.)*sq(Delta);
        vb += u.x[] * dv;
        xb += x * dv;
        sb += dv;
      }
      // get shape 
      if (t_real == 0){
        fprintf(fp_shape, "time,velocity,ratio,centerX,Xmax,Xmin,Ymax\n");    
      }
      fprintf(fp_shape, "%g,%g,%g,%g,%g,%g,%g\n", t, vb / sb,ratio,-xb / sb, -x_max,-x_min,y_max); fflush(fp_shape);
    }
  }
}


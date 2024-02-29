#include "axi.h"
#include "navier-stokes/centered.h"
#include "contact.h"
#define FILTERED
#define mu(f)  (1./(clamp(f,0,1)*(1./mu1 - 1./mu2) + 1./mu2))
#include "two-phase.h"
#include "tension.h"
#include "adapt_wavelet_limited.h"
#include "output_vtu_foreach.h"
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

int main(int argc, char const *argv[]) {
// Galilei,  Bond,  domainSize,  L_rise,  basicLevel,  radiusLevel, t_max
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
  run();
}

event acceleration (i++) {
  face vector av = a;
  foreach_face(x)
    av.x[] -= 1.0;
}

event init(t = 0){
  if (!restore(file = "dump"))
  {
    refine(circleDrop>-1.0 && level<basicLevel+3);  //细化网格的区域需要大于初始化网格的尺寸
    refine(fabs(circleDrop)<0.1 && level<maxLevel);  //细化网格的区域需要大于初始化网格的尺寸
    fraction(f, circleDrop);
    
    boundary((scalar *){f, u.x, u.y});
  }
}

int refRegion(double x, double y, double z){
  return ( 
      ((y<1.5)&&(x>-0.01)) ? maxLevel+2:
      ((y<1.5)&&(x>-0.02)) ? maxLevel+1:
      (y<2.0) ? maxLevel:
      (y<4.0) ? maxLevel-1:
      (y<8.0) ? maxLevel-2:
      (y<16.0) ? maxLevel-3:
      (y<32.0) ? maxLevel-4:
      maxLevel-5);
}

event adapt(i++){  
  scalar KAPPA[];
  curvature(f, KAPPA);
  scalar omega[];
  vorticity (u, omega);
  scalar D2c[];
  foreach(){
    double D11 = (u.y[0,1] - u.y[0,-1])/(2*Delta);
    double D22 = (u.y[]/max(y,1e-20));
    double D33 = (u.x[1,0] - u.x[-1,0])/(2*Delta);
    double D13 = 0.5*( (u.y[1,0] - u.y[-1,0] + u.x[0,1] - u.x[0,-1])/(2*Delta) );
    double D2 = (sq(D11)+sq(D22)+sq(D33)+2.0*sq(D13));
    D2c[] = D2*f[]; //对整个区域的D2c和omega进行细化
    omega[]=omega[]*f[];
  }
  boundary((scalar *){D2c, KAPPA, omega});


  adapt_wavelet_limited ((scalar *){f, KAPPA, u.x, u.y, D2c, omega},
     (double[]){fErr, KErr, VelErr, VelErr, DissErr, OmegaErr},
      refRegion, minLevel);

  unrefine(y>8.0 && level > (basicLevel-1));
  unrefine(x<-domainSize+2.0 && level > basicLevel);
  
}

//stop condition

event stop (t=tmax) 
{  
  
}

//**************************************************************************************************************************************//
//****************************************************Post progress*********************************************************************//
//**************************************************************************************************************************************//

event init_File(t=0){
  mkdir("Intermediate", 0700);
  mkdir("Log", 0700);
  static FILE *fp_p;
  fp_p = fopen("Log/log_properties", "w");
  fprintf(fp_p, "domainSize %g, minLevel %i, maxLevel %i \n", L0, minLevel, maxLevel);  fflush(fp_p);
  fprintf(fp_p, "rho1 %g, rho2 %g, mu1 %g mu2 %g, sigma %3.2e, g 1.0\n", rho1, rho2, mu1, mu2, f.sigma);  fflush(fp_p);
  fprintf(fp_p, "Galilei number %g, Bond number %3.2e\n", Galilei, Bond); fflush(fp_p); 
}

event writingFiles (t += tsnap) {
  p.nodump=false;
  dump (file = "dump");
  char nameOut[80];
  sprintf (nameOut, "Intermediate/snapshot-%5.4f", t);
  dump (file = nameOut);
}


double t_last=0.0;
double DeltaT=0.0;
event logstats (t += tsnap)
{
  static FILE *fp= fopen("Log/log_status", "a");
  timing s = timer_timing(perf.gt, i, perf.tnc, NULL);
  DeltaT=perf.t/60.0-t_last;
  t_last=perf.t/60.0;

  if (i == 0)
  {
    fprintf(fp, "t dt i #Cell(pid) #Cell(All) Wallclocktime(min) CPUtime(min) time(minMPI) time(maxMPI) deltaT\n"); fflush(fp);
  }
  fprintf(fp, "%g %g %i %ld %ld %g %g %g %g %g\n", t, dt, i,  grid->n, grid->tn, perf.t/60.0, s.cpu/60.0,s.min/60.0,s.max/60.0,DeltaT); fflush(fp);
}

double b_center= 0.0;
double V_last=0.0;
double V_now=0.0;
int countSmallV=0;
event bubbles(t += tsnap)
{
  static FILE *fp_shape= fopen("results_shape.dat", "a");
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
  if (t == 0){
    fprintf(fp_shape, "time,velocity,ratio,centerX,Xmax,Xmin,Ymax\n");    
  }
  fprintf(fp_shape, "%g,%g,%g,%g,%g,%g,%g\n", t, vb / sb,ratio,-xb / sb, -x_max,-x_min,y_max); fflush(fp_shape);

  V_now=vb;
  if (V_last<0.0 && V_now>0.0){
    return 1;
  }
  V_last=vb;

  // for nobounce
  if (vb / sb<0.002 && vb >0){
    countSmallV++;
    if (countSmallV>50){
      return 1;
    }
  }
}

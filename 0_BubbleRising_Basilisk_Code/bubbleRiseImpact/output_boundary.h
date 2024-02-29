#include "fractions.h"


struct OutputBoundary {
  scalar c;
  double *x_max;
  double *x_min;
  double *y_max;
  double *se;
  face vector s; // optional: default is none

};

trace
// void output_boundary (struct OutputBoundary p)
void output_boundary (struct OutputBoundary p)
{
  scalar c = p.c;
  face vector s = p.s;
  if (!s.x.i) s.x.i = -1;
  double x_small = 100, x_large = -100, y_small = 100, y_large = -100;
  double se1 = 0.;

  // foreach()
  foreach(reduction(min: x_small) reduction(max: x_large) reduction(min: y_small) reduction(max: y_large) reduction(+:se1)) {
    if (c[] > 1e-6 && c[] < 1. - 1e-6) {
      coord n = facet_normal (point, c, s);
      double alpha = plane_alpha (c[], n);
      coord segment[2];
      if (facets (n, alpha, segment) == 2)
        x_small=((x_small<(x + segment[0].x*Delta)&&x_small<(x + segment[1].x*Delta)) ? x_small : ((x + segment[0].x*Delta)<x_small&&(x + segment[0].x*Delta)<(x + segment[1].x*Delta)) ? (x + segment[0].x*Delta) : (x + segment[1].x*Delta));
        x_large=((x_large>(x + segment[0].x*Delta)&&x_large>(x + segment[1].x*Delta)) ? x_large : ((x + segment[0].x*Delta)>x_large&&(x + segment[0].x*Delta)>(x + segment[1].x*Delta)) ? (x + segment[0].x*Delta) : (x + segment[1].x*Delta));
        
        y_small=((y_small<(y + segment[0].y*Delta)&&y_small<(y + segment[1].y*Delta)) ? y_small : ((y + segment[0].y*Delta)<y_small&&(y + segment[0].y*Delta)<(y + segment[1].y*Delta)) ? (y + segment[0].y*Delta) : (y + segment[1].y*Delta));
        y_large=((y_large>(y + segment[0].y*Delta)&&y_large>(y + segment[1].y*Delta)) ? y_large : ((y + segment[0].y*Delta)>y_large&&(y + segment[0].y*Delta)>(y + segment[1].y*Delta)) ? (y + segment[0].y*Delta) : (y + segment[1].y*Delta));
        se1 += 2.*pi*( y + 0.5*(segment[0].y+segment[1].y)*Delta )*(sqrt(sq(segment[0].x-segment[1].x)+sq(segment[0].y-segment[1].y))*Delta);
    }
  }

  // fprintf (p.fp, "%g %g %g %g %g %g %g %g %g %g\n", t, x_small, x_large, y_small, y_large, t*t_ref*1000, x_small*Radius*1000, x_large*Radius*1000, y_small*Radius*1000, y_large*Radius*1000);
  // fflush (p.fp);
  *(p.x_max)=x_large;
  *(p.x_min)=x_small;
  *(p.y_max)=y_large;
  *(p.se)=se1;
}
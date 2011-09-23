#ifndef FtQLiAlgorithm_hh
#define FtQLiAlgorithm_hh
#include <limits>


namespace FMath
{

#undef SQR
#define SQR(a) (a == 0.0 ? 0.0 : a*a)

  inline double pythag(double a, double b)
  {
    double absa,absb;
    absa=fabs(a);
    absb=fabs(b);
    if (absa > absb) return absa*sqrt(1.0+SQR(absb/absa));
    else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
  }
#undef SQR


  struct RowMajor
  {
    RowMajor( double* const data, unsigned int dim ): data( data ), dim( dim ){}
    inline double& operator()( unsigned int i, unsigned int j )
    { return data[ i*dim + j ];}
    double * const data;
    const unsigned int dim;
  };

  struct ColumnMajor
  {
    ColumnMajor( double*const data, unsigned int dim ): data( data ), dim( dim ){}
    inline double& operator()( unsigned int i, unsigned int j )
    { return data[ j*dim + i ];}
    double * const data;
    const unsigned int dim;
  };

  template< class Access>
  inline void tQLiAlgorithm(Access access, F::FVector& d, F::FVector& e )
  {
    const unsigned int n=d.getDimension();

#undef SIGN
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))


    unsigned int m,l,iter,i,k;
    double s,r,p,g,f,dd,c,b;

    for(i=2;i<=n;i++) e[i-2]=e[i-1];
    e[n-1]=0.0;

    for(l=1;l<=n;l++)
      {
        iter=0;
        do
      {
        for(m=l;m<=n-1;m++)
          {
            dd=fabs(d[m-1])+fabs(d[m]);
            double temp = (double)(fabs(e[m-1])+dd);
            if( fabs( temp - dd ) <= std::numeric_limits<double>::epsilon()) break;
          }
        if (m != l)
          {
            if (iter++ == 30)
                THROW_DEFAULT_EXCEPTION( FTooManyIterationsException );

            g=(d[l]-d[l-1])/(2.0*e[l-1]);
            r=pythag(g,1.0);
            g=d[m-1]-d[l-1]+e[l-1]/(g+SIGN(r,g));
            s=c=1.0;
            p=0.0;
            for (i=m-1;i>=l;i--)
          {
            f=s*e[i-1];
            b=c*e[i-1];
            e[i]=(r=pythag(f,g));
            if (r == 0.0)
              {
                d[i] -= p;
                e[m-1]=0.0;
                break;
              }
            s=f/r;
            c=g/r;
            g=d[i]-p;
            r=(d[i-1]-g)*s+2.0*c*b;
            d[i]=g+(p=s*r);
            g=c*r-b;

            for(k=1;k<=n;k++)
              {
                f=access(k-1, i);
                access( k-1, i ) = s*access( k-1,i-1 )+c*f;
                access( k-1, i-1 ) = c*access( k-1,i-1 )-s*f;
              }
          }
            if (r == 0.0 && i >= l) continue;
            d[l-1] -= p;
            e[l-1]=g;
            e[m-1]=0.0;
          }
      } while (m != l);
      }
#undef SIGN
  }


template<class Access>
  inline void tridiagonalize(Access access, F::FVector& d, F::FVector& e)
  {
    const unsigned int n=d.getDimension();

#ifndef NODEBUG

    if((d.getDimension()!=n) || (e.getDimension()!=n))
      THROW_EXCEPTION( FInvalidDimensionException, "ERROR: invalid Vector dimension!");
#endif

    unsigned  int k,j,i,l,c;

    double scale, hh, h, g, f;

    for(i=n; i>=2; i--)
      {
        l=i-1;
        h=scale=0.0;
        if(l>1)
      {
        for(k=1; k<=l; k++)
          scale += fabs(access(i-1,k-1));
        if(scale == 0.0)
          e[i-1]=access(i-1,l-1);
        else
          {
            for(k=1; k<=l; k++)
          {
            access(i-1, k-1 ) = access(i-1, k-1) / scale;
            h += access(i-1, k-1)*access(i-1, k-1);
          }
            f = access(i-1,l-1);
            g = (f >= 0.0 ? -sqrt(h) : sqrt(h));
            e[i-1] = scale*g;
            h -= f*g;
            access(i-1, l-1 ) = f-g;
            f=0.0;
            for(j=1; j<=l; j++)
          {
            access( j-1, i-1 ) = access(i-1, j-1)/h;
            g=0.0;
            for(k=1; k<=j; k++)
              g += access(j-1,k-1)*access(i-1,k-1);
            for(k=j+1; k<=l; k++)
              g += access(k-1,j-1)*access(i-1,k-1);
            e[j-1] = g/h;
            f += e[j-1] * access(i-1,j-1);
          }
            hh=f/(h+h);
            for(j=1; j<=l; j++)
          {
            f=access(i-1,j-1);
            e[j-1] = g = e[j-1] - hh*f;
            for(k=1; k<=j; k++)
              access(j-1, k-1 ) = access(j-1,k-1) - (f*e[k-1]+g*access(i-1,k-1));
          }
          }
      }
        else
      e[i-1] = access(i-1,l-1);
        d[i-1] = h;
      }

    d[0] = 0.0;
    e[0] = 0.0;

    for(i=1; i<=n; i++)
      {
        c=i-1;
        if(d[i-1])
      {
        for(j=1;j <= c; j++)
          {
            g = 0.0;
            for(k=1; k <= c; k++) g+=access(i-1,k-1)*access(k-1,j-1);
            for(k=1; k <= c; k++) access(k-1, j-1 ) = ( access(k-1,j-1) - g*access(k-1,i-1));
          }
      }
        d[i-1]=access(i-1,i-1);
        access(i-1, i-1 ) = 1.0;
        for(j=1; j <= c; j++)
      {
        access(j-1, i-1 ) = 0.0;
        access(i-1, j-1 ) = 0.0;
      }
      }
  }

} // namespace FMath

#endif

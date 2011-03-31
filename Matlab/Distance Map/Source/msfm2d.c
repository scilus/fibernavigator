#include "mex.h"
#include "math.h"
#define eps 2.3e-016
#define doublemax 1e300
#define INF 1e305
#ifndef min
#define min(a,b)        ((a) < (b) ? (a): (b))
#endif
#ifndef max
#define max(a,b)        ((a) > (b) ? (a): (b))
#endif


/*
 * This function MSFM2D calculates the shortest distance from a list of
 * points to all other pixels in an image, using the
 *Multistencil Fast Marching Method (MSFM). This method gives more accurate
 *distances by using second order derivatives and cross neighbours.
 *
 *T=msfm2d(F, SourcePoints, UseSecond, UseCross)
 *
 *inputs,
 *  F: The speed image
 *  SourcePoints : A list of starting points [2 x N] (distance zero)
 *  UseSecond : Boolean Set to true if not only first but also second
 *               order derivatives are used (default)
 *  UseCross: Boolean Set to true if also cross neighbours
 *               are used (default)
 *outputs,
 *  T : Image with distance from SourcePoints to all pixels
 *
 *Function is written by D.Kroon University of Twente (June 2009)
*/

/* Find minimum value of an array and return its index */
int minarray(double *A, int l) {
    int i;
    int minind=0;
    for (i=0; i<l; i++) { if(A[i]<A[minind]){ minind=i; } }
    return minind;
}

double pow2(double val) { return val*val; }
void roots(double* Coeff, double* ans) {
    double a=Coeff[0];
    double b=Coeff[1];
    double c=Coeff[2];
    
    if(a!=0) {
        ans[0]= (-b - sqrt(pow2(b)-4.0*a*c)) / (2.0*a);
        ans[1]= (-b + sqrt(pow2(b)-4.0*a*c)) / (2.0*a);
    }
    else {
        ans[0]= (2.0*c)/(-b - sqrt(pow2(b)-4.0*a*c));
        ans[1]= (2.0*c)/(-b + sqrt(pow2(b)-4.0*a*c));
    }
}

int maxarray(double *A, int l) {
    int i;
    int maxind=0;
    for (i=0; i<l; i++) { if(A[i]>A[maxind]){ maxind=i; } }
    return maxind;
}

__inline mindex2(int x, int y, int sizx) { return x+y*sizx; }    
        
__inline bool IsFinite(double x) { return (x <= doublemax && x >= -doublemax); }
__inline bool IsInf(double x) { return (x >= doublemax); }

__inline bool isntfrozen2d(int i, int j, int *dims, bool *Frozen)
{
    return (i>=0)&&(j>=0)&&(i<dims[0])&&(j<dims[1])&&(Frozen[i+j*dims[0]]==0);
}
__inline bool isfrozen2d(int i, int j, int *dims, bool *Frozen)
{
    return (i>=0)&&(j>=0)&&(i<dims[0])&&(j<dims[1])&&(Frozen[i+j*dims[0]]==1);
}


int p2x(int x) /* 2^x */
{
/*    return pow(2,x); */
    int y=1; 
    int p2x[16] ={1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    while(x>15) { x=x-15; y=y*32768; }
    return y*p2x[x];
}

void show_list(double **listval, int *listprop) {
    int z, k;
    for(z=0;z<listprop[1]; z++) {
        for(k=0;k<p2x(z+1); k++) {
            if((z>0)&&(listval[z-1][(int)floor(k/2)]>listval[z][k])) {
                printf("*%15.5f", listval[z][k]);
            }
            else {
                printf(" %15.5f", listval[z][k]);
            }
        }
        printf("\n");
    }
}

void initialize_list(double ** listval, int *listprop) {
    /* Loop variables */
    int i;
    /* Current Length, Orde and Current Max Length */
    listprop[0]=0; listprop[1]=1; listprop[2]=2;
    /* Make first orde storage of 2 values */
    listval[0]=(double*)malloc(2 * sizeof(double));
    /* Initialize on infinite */
    for(i=0;i<2;i++) { listval[0][i]=INF; }
}

void destroy_list(double ** listval, int *listprop) {
    /* Loop variables */
    int i, list_orde;
    /* Get list orde */
    list_orde=listprop[1];
    /* Free memory */
    for(i=0;i<list_orde;i++) { free(listval[i]); }
    free(listval);
    free(listprop);
}

void list_add(double ** listval, int *listprop, double val) {
    /* List parameters */
    int list_length, list_orde, list_lengthmax;
    /* Loop variable */
    int i, j;
    /* Temporary list location */
    int listp;
    /* Get list parameters */
    list_length=listprop[0]; list_orde=listprop[1]; list_lengthmax=listprop[2];
    /* If list is full expand list */
    if(list_length==list_lengthmax) {
        list_lengthmax=list_lengthmax*2;
        for (i=list_orde; i>0; i--) {
            listval[i]=listval[i-1];
            listval[i] = (double *)realloc(listval[i], p2x(i+1)*sizeof(double));
            for(j=p2x(i); j<p2x(i+1); j++) { listval[i][j]=INF;  }
        }
        listval[0]=(double *)malloc(2*sizeof(double));
        listval[0][0]=min(listval[1][0], listval[1][1]);
        listval[0][1]=INF;
        list_orde++;
    }
    /* Add a value to the list */
    listp=list_length;
    list_length++;
    listval[list_orde-1][listp]=val;
    /* Update the links minimum */
    for (i=list_orde-1; i>0; i--) {
        listp=(int)floor(((double)listp)/2);
        if(val<listval[i-1][listp]) { listval[i-1][listp]=val; } else { break; }
    }
    /* Set list parameters */
    listprop[0]=list_length; listprop[1]=list_orde; listprop[2]=list_lengthmax;
}

int list_minimum(double ** listval, int *listprop) {
    /* List parameters */
    int list_length, list_orde, list_lengthmax;
    /* Loop variable */
    int i;
    /* Temporary list location */
    int listp;
    /* Index of Minimum */
    int minindex;
    /* Get list parameters */
    list_length=listprop[0]; list_orde=listprop[1]; list_lengthmax=listprop[2];
    /* Follow the minimum through the binary tree */
    listp=0;
    for(i=0;i<(list_orde-1);i++) {
        if(listval[i][listp]<listval[i][listp+1]) { listp=listp*2; } else { listp=(listp+1)*2; }
    }
    i=list_orde-1;
    if(listval[i][listp]<listval[i][listp+1]){minindex=listp; } else { minindex=listp+1; }
    return minindex;
}
void list_remove(double ** listval, int *listprop, int index) {
    /* List parameters */
    int list_length, list_orde, list_lengthmax;
    /* Loop variable */
    int i;
    /* Temp index */
    int index2;
    double val;
    double valmin;
    /* Get list parameters */
    list_length=listprop[0];
    list_orde=listprop[1];
    list_lengthmax=listprop[2];
    /* Temporary store current value */
    val=listval[list_orde-1][index];
    valmin=INF;
    /* Replace value by infinite */
    listval[list_orde-1][index]=INF;
    /* Follow the binary tree to replace value by minimum values from 
     * the other values in the binary tree. 
     */
    i=list_orde-1;
    while(true) {
        if((index%2)==0) { index2=index+1; } else { index2=index-1; }
        if(val<listval[i][index2]) {
            index=(int)floor(((double)index2)/2.0);
            if(listval[i][index2]<valmin) { valmin=listval[i][index2]; }
            listval[i-1][index]=valmin;
            i--; if(i==0) { break; }
        }
        else { break; }
    }
}

void list_remove_replace(double ** listval, int *listprop, int index) {
    /* List parameters  */
    int list_length, list_orde, list_lengthmax;
    /* Loop variable  */
    int i, listp;
    /* Temporary store value  */
    double val;
    int templ;
    /* Get list parameters */
    list_length=listprop[0];
    list_orde=listprop[1];
    list_lengthmax=listprop[2];
    /* Remove the value */
    list_remove(listval, listprop, index);
    /* Replace the removed value by the last in the list. (if it was
     * not already the last value)  */
    if(index<(list_length-1)) {
        /* Temporary store last value in the list  */
        val=listval[list_orde-1][list_length-1];
        /* Remove last value in the list  */
        list_remove(listval, listprop, list_length-1);
        /* Add a value to the list  */
        listp=index;
        listval[list_orde-1][index]=val;
        /* Update the links minimum  */
        for (i=(list_orde-1); i>0; i--) {
            listp=(int)floor(((double)listp)/2);
            if(val<listval[i-1][listp]) { listval[i-1][listp]=val; } else {  break; }
        }
    }
    /* List is now shorter */
    list_length--;
    /* Remove trunk of binary tree  / Free memory if list becomes shorter */
    if(list_orde>2&&IsInf(listval[0][1])) {
        list_orde--; 
        list_lengthmax=list_lengthmax/2;
        /* Remove trunk array */
        free(listval[0]);
        /* Move the other arrays one up */
        templ=2;
        for (i=0; i<list_orde; i++) {
            listval[i]=listval[i+1];
            /* Resize arrays to their new shorter size */
            listval[i] = (double *)realloc(listval[i], templ*sizeof(double));
            templ*=2;
        }
    }
    /* Set list parameters */
    listprop[0]=list_length; listprop[1]=list_orde; listprop[2]=list_lengthmax;
}

void listupdate(double **listval, int *listprop, int index, double val) {
    /* List parameters */
    int list_length, list_orde, list_lengthmax;
    /* loop variable */
    int i,listp;
    /* Get list parameters  */
    list_length=listprop[0];
    list_orde=listprop[1];
    list_lengthmax=listprop[2];
    /* Add a value to the list */
    listp=index;
    listval[list_orde-1][index]=val;
    /* Update the links minimum */
    for (i=(list_orde-1); i>0; i--) {
        listp=(int)floor(((double)listp)/2);
        if(val<listval[i-1][listp]) { listval[i-1][listp]=val; } else { break; }
    }
    /* Set list parameters */
    listprop[0]=list_length; listprop[1]=list_orde; listprop[2]=list_lengthmax;
}

/* The matlab mex function  */
void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] ) {
    /* The input variables */
    double *F, *SourcePoints;
    bool *useseconda, *usecrossa;
    bool usesecond=true;
    bool usecross=true;
    
    /* The output distance image */
    double *T;
    
    /* Current distance values */
    double Tt, Tt2;
    
    /* Derivatives */
    double Tm[4]={0, 0, 0, 0};
    double Tm2[4]={0, 0, 0, 0};
    double Coeff[3];
    
    /* local derivatives in distance image */
    double Txm1,Txm2,Txp1,Txp2;
    double Tym1,Tym2,Typ1,Typ2;
    double Tr1m1,Tr1m2,Tr1p1,Tr1p2;
    double Tr2m1,Tr2m2,Tr2p1,Tr2p2;

    /* Constant cross term */
    const double c1=0.5;
    
    /* Matrix containing the Frozen Pixels" */
    bool *Frozen;
    
    /* Size of input image */
    const mwSize *dims_c;
    mwSize dims[2];
    
    /* Size of  SourcePoints array */
    const mwSize *dims_sp_c;
    mwSize dims_sp[2];
            
    /* Return values root of polynomial */
    double ansroot[2]={0, 0};
    
    /* Number of pixels in image */
    int npixels;
    
    /* Derivative checks */
    bool ch1, ch2;
    
    /* Order derivatives in a certain direction */
    int Order[4]={0, 0, 0, 0};
    
    /* Neighbour list */
    int neg_free;
    int neg_pos;
    double *neg_listv;
    double *neg_listx;
    double *neg_listy;
    
    int *listprop;
    double **listval;
    
    /* Neighbours 4x2 */
    int ne[8]={-1, 0, 1, 0, 0, -1, 0, 1};
    /* Loop variables  */
    int z, k, itt, q, t;
    
    /* Current location */
    int x, y, i, j, in, jn;
    
    /* Index */
    int IJ_index, XY_index, index;
    
    /* Check for proper number of input and output arguments. */
    if(nrhs<3) {
        mexErrMsgTxt("2 to 4 inputs are required.");
    } else if(nlhs!=1) {
        mexErrMsgTxt("One output required");
    }
    
    /* Check data input types  */
    if(mxGetClassID(prhs[0])!=mxDOUBLE_CLASS) {
        mexErrMsgTxt("Speed image must be of class double");
    }
    if(mxGetClassID(prhs[1])!=mxDOUBLE_CLASS) {
        mexErrMsgTxt("SourcePoints must be of class double");
    }
    
    if((nrhs>2)&&(mxGetClassID(prhs[2])!= mxLOGICAL_CLASS)) {
        mexErrMsgTxt("UseSecond must be of class boolean / logical");
    }
    if((nrhs>3)&&(mxGetClassID(prhs[3])!= mxLOGICAL_CLASS)) {
        mexErrMsgTxt("UseCross must be of class boolean / logical");
    }
    
    
    /* Get the sizes of the input image */
    if(mxGetNumberOfDimensions(prhs[0])==2)
    {
        dims_c = mxGetDimensions(prhs[0]);
        dims[0]=dims_c[0]; dims[1]=dims_c[1];
        npixels=dims[0]*dims[1];
    }
    else
    {
        mexErrMsgTxt("Speed image must be 2d.");        
    }
    
    /* Get the sizes of the  SourcePoints */
    dims_sp_c = mxGetDimensions(prhs[1]);
    
    if(dims_sp_c[0]!=2) {
        mexErrMsgTxt("SourcePoints must be a 2xn matrix.");
    }
    dims_sp[0]=dims_sp_c [0]; dims_sp[1]=dims_sp_c[1];
    
    
    /* Get pointers/data from  to each input. */
    F=(double*)mxGetPr(prhs[0]);
    SourcePoints=(double*)mxGetPr(prhs[1]);
    if(nrhs>2){ useseconda = (bool*)mxGetPr(prhs[2]); usesecond=useseconda[0];}
    if(nrhs>3){ usecrossa = (bool*)mxGetPr(prhs[3]); usecross=usecrossa[0];}
    
    /* Create the distance output array */
    plhs[0] = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    /* Assign pointer to output.  */
    /*Distance image, also used to store the index of narrowband pixels  */
    /*during marching process  */
    T= mxGetPr(plhs[0]);
    
    /* Pixels which are processed and have a final distance are frozen */
    Frozen = (bool*)malloc( npixels* sizeof(bool) );
    for(q=0;q<npixels;q++){Frozen[q]=0; T[q]=-1;}
    
    /*Free memory to store neighbours of the (segmented) region */
    neg_free = 100000;
    neg_pos=0;
    neg_listx = (double *)malloc( neg_free*sizeof(double) );
    neg_listy = (double *)malloc( neg_free*sizeof(double) );
    
    /* List parameters array */
    listprop=(int*)malloc(3* sizeof(int));
    /* Make jagged list to store a maximum of 2^64 values */
    listval= (double **)malloc( 64* sizeof(double *) );
    /* Initialize parameter list */
    initialize_list(listval, listprop);
    neg_listv=listval[listprop[1]-1];
    
    
    /*(There are 3 pixel classes:
     *  - frozen (processed)
     *  - narrow band (boundary) (in list to check for the next pixel with smallest distance)
     *  - far (not yet used)
     */
    
    
    /* set all starting points to distance zero and frozen  */
    /* and add all neighbours of the starting points to narrow list  */
 
    for (z=0; z<dims_sp[1]; z++) 
    {
        /*starting point  */
        x= (int)SourcePoints[0+z*2]-1;
        y= (int)SourcePoints[1+z*2]-1;
        XY_index=x+y*dims[0];
        
        /*Set starting point to frozen and distance to zero  */
        Frozen[XY_index]=1;
        T[XY_index]=0;
    }
    
    for (z=0; z<dims_sp[1]; z++) {
        /*starting point  */
        x= (int)SourcePoints[0+z*2]-1;
        y= (int)SourcePoints[1+z*2]-1;
        XY_index=x+y*dims[0];
        
            /* Add neigbours of starting points  */
            for (k=0; k<4; k++) {
                /*Location of neighbour  */
                i=x+ne[k];
                j=y+ne[k+4];
                IJ_index=i+j*dims[0];

                /*Check if current neighbour is not yet frozen and inside the
                 *picture  */
                if(isntfrozen2d(i,j, dims, Frozen)) {
                    Tt=(1/(F[IJ_index]+eps));
                    /*Update distance in neigbour list or add to neigbour list */
                    if(T[IJ_index]>0) {
                        if(neg_listv[(int)T[IJ_index]]>Tt) {
                            listupdate(listval, listprop, (int)T[IJ_index], Tt);
                        }
                    }
                    else {
                        /*If running out of memory at a new block  */
                        if(neg_pos>=neg_free) {
                            neg_free+=100000;
                            neg_listx = (double *)realloc(neg_listx, neg_free*sizeof(double) );
                            neg_listy = (double *)realloc(neg_listy, neg_free*sizeof(double) );
                        }
                        list_add(listval, listprop, Tt);
                        neg_listv=listval[listprop[1]-1];
                        neg_listx[neg_pos]=i;
                        neg_listy[neg_pos]=j;
                        T[IJ_index]=neg_pos;
                        neg_pos++;
                    }
                }
        }
    }
    
    /*Loop through all pixels of the image  */
    for (itt=0; itt<npixels; itt++)
    {
        /*Get the pixel from narrow list (boundary list) with smallest
         *distance value and set it to current pixel location  */
        index=list_minimum(listval, listprop);
        neg_listv=listval[listprop[1]-1];    
        /* Stop if pixel distance is infinite (all pixels are processed)  */
        if(IsInf(neg_listv[index])) { break; }
        x=(int)neg_listx[index]; y=(int)neg_listy[index];
        
        XY_index=x+y*dims[0];
        Frozen[XY_index]=1;
        T[XY_index]=neg_listv[index];
       
        
        /*Remove min value by replacing it with the last value in the array  */
        list_remove_replace(listval, listprop, index) ;
        neg_listv=listval[listprop[1]-1];
        if(index<(neg_pos-1)) {
            neg_listx[index]=neg_listx[neg_pos-1];
            neg_listy[index]=neg_listy[neg_pos-1];
            T[(int)(neg_listx[index]+neg_listy[index]*dims[0])]=index;
        }
        neg_pos =neg_pos-1;
        
        /*Loop through all 4 neighbours of current pixel  */
        for (k=0;k<4;k++) {
            
            /*Location of neighbour  */
            i=x+ne[k]; j=y+ne[k+4];
            IJ_index=i+j*dims[0];
            
            /*Check if current neighbour is not yet frozen and inside the  */
            /*picture  */
            if(isntfrozen2d(i,j, dims, Frozen)) {
                
                /*Get First order derivatives (only use frozen pixel)  */
                in=i-1; jn=j+0; if(isfrozen2d(in, jn, dims, Frozen)) { Txm1=T[mindex2(in,jn,dims[0])]; } else { Txm1=INF; }
                in=i+0; jn=j-1; if(isfrozen2d(in, jn, dims, Frozen)) { Tym1=T[mindex2(in,jn,dims[0])]; } else { Tym1=INF; }
                in=i+0; jn=j+1; if(isfrozen2d(in, jn, dims, Frozen)) { Typ1=T[mindex2(in,jn,dims[0])]; } else { Typ1=INF; }
                in=i+1; jn=j+0; if(isfrozen2d(in, jn, dims, Frozen)) { Txp1=T[mindex2(in,jn,dims[0])]; } else { Txp1=INF; }
                if(usecross) {
                    in=i-1; jn=j-1; if(isfrozen2d(in, jn, dims, Frozen)) { Tr1m1=T[mindex2(in,jn,dims[0])]; } else { Tr1m1=INF; }
                    in=i-1; jn=j+1; if(isfrozen2d(in, jn, dims, Frozen)) { Tr2m1=T[mindex2(in,jn,dims[0])]; } else { Tr2m1=INF; }
                    in=i+1; jn=j-1; if(isfrozen2d(in, jn, dims, Frozen)) { Tr2p1=T[mindex2(in,jn,dims[0])]; } else { Tr2p1=INF; }
                    in=i+1; jn=j+1; if(isfrozen2d(in, jn, dims, Frozen)) { Tr1p1=T[mindex2(in,jn,dims[0])]; } else { Tr1p1=INF; }
                }
                /*The values in order is 0 if no neighbours in that direction  */
                /*1 if 1e order derivatives is used and 2 if second order  */
                /*derivatives are used  */
                Order[0]=0; Order[1]=0; Order[2]=0; Order[3]=0;
                
                /*Make 1e order derivatives in x and y direction  */
                Tm[0] = min( Txm1 , Txp1); if(IsFinite(Tm[0])){ Order[0]=1; }
                Tm[1] = min( Tym1 , Typ1); if(IsFinite(Tm[1])){ Order[1]=1; }
                /*Make 1e order derivatives in cross directions  */
                if(usecross) {
                    Tm[2] = min( Tr1m1 , Tr1p1); if(IsFinite(Tm[2])){ Order[2]=1; }
                    Tm[3] = min( Tr2m1 , Tr2p1); if(IsFinite(Tm[3])){ Order[3]=1; }
                }
                /*Make 2e order derivatives  */
                if(usesecond) {
                    /*Get Second order derivatives (only use frozen pixel) */
                    in=i-2; jn=j+0; if(isfrozen2d(in, jn, dims, Frozen)) { Txm2=T[mindex2(in,jn,dims[0])]; } else { Txm2=INF; }
                    in=i+2; jn=j+0; if(isfrozen2d(in, jn, dims, Frozen)) { Txp2=T[mindex2(in,jn,dims[0])]; } else { Txp2=INF; }
                    in=i+0; jn=j-2; if(isfrozen2d(in, jn, dims, Frozen)) { Tym2=T[mindex2(in,jn,dims[0])]; } else { Tym2=INF; }
                    in=i+0; jn=j+2; if(isfrozen2d(in, jn, dims, Frozen)) { Typ2=T[mindex2(in,jn,dims[0])]; } else { Typ2=INF; }
                    if(usecross) {                
                        in=i-2; jn=j-2; if(isfrozen2d(in, jn, dims, Frozen)) { Tr1m2=T[mindex2(in,jn,dims[0])]; } else { Tr1m2=INF; }
                        in=i-2; jn=j+2; if(isfrozen2d(in, jn, dims, Frozen)) { Tr2m2=T[mindex2(in,jn,dims[0])]; } else { Tr2m2=INF; }
                        in=i+2; jn=j-2; if(isfrozen2d(in, jn, dims, Frozen)) { Tr2p2=T[mindex2(in,jn,dims[0])]; } else { Tr2p2=INF; }
                        in=i+2; jn=j+2; if(isfrozen2d(in, jn, dims, Frozen)) { Tr1p2=T[mindex2(in,jn,dims[0])]; } else { Tr1p2=INF; }
                    }

                    Tm2[0]=0; Tm2[1]=0;Tm2[2]=0; Tm2[3]=0;
                    /*pixels with a pixeldistance 2 from the center must be */
                    /*lower in value otherwise use other side or first order */
                    ch1=(Txm2<Txm1)&&IsFinite(Txm1); ch2=(Txp2<Txp1)&&IsFinite(Txp1);
                    
                    if(ch1&&ch2) {
                        Tm2[0] =min( (4.0*Txm1-Txm2)/3.0 , (4.0*Txp1-Txp2)/3.0);  Order[0]=2;
                    }
                    else if (ch1) {
                        Tm2[0]=(4.0*Txm1-Txm2)/3.0; Order[0]=2;
                    }
                    else if(ch2) {
                        Tm2[0] =(4.0*Txp1-Txp2)/3.0; Order[0]=2;
                    }
                    
                    ch1=(Tym2<Tym1)&&IsFinite(Tym1); ch2=(Typ2<Typ1)&&IsFinite(Typ1);
                    
                    if(ch1&&ch2) {
                        Tm2[1] =min( (4.0*Tym1-Tym2)/3.0 , (4.0*Typ1-Typ2)/3.0); Order[1]=2;
                    }
                    else if(ch1) {
                        Tm2[1]=(4.0*Tym1-Tym2)/3.0; Order[1]=2;
                    }
                    else if(ch2) {
                        Tm2[1]=(4.0*Typ1-Typ2)/3.0; Order[1]=2;
                    }
                    if(usecross) {
                        ch1=(Tr1m2<Tr1m1)&&IsFinite(Tr1m1); ch2=(Tr1p2<Tr1p1)&&IsFinite(Tr1p1);
                        if(ch1&&ch2) {
                            Tm2[2] =min( (4.0*Tr1m1-Tr1m2)/3.0 , (4.0*Tr1p1-Tr1p2)/3.0); Order[2]=2;
                        }
                        else if(ch1) {
                            Tm2[2]=(4.0*Tr1m1-Tr1m2)/3.0; Order[2]=2;
                        }
                        else if(ch2){
                            Tm2[2]=(4.0*Tr1p1-Tr1p2)/3.0; Order[2]=2;
                        }
                        
                        ch1=(Tr2m2<Tr2m1)&&IsFinite(Tr2m1); ch2=(Tr2p2<Tr2p1)&&IsFinite(Tr2p1);
                        if(ch1&&ch2){
                            Tm2[3] =min( (4.0*Tr2m1-Tr2m2)/3.0 , (4.0*Tr2p1-Tr2p2)/3.0); Order[3]=2;
                        }
                        else if(ch1) {
                            Tm2[3]=(4.0*Tr2m1-Tr2m2)/3.0; Order[3]=2;
                        }
                        else if(ch2) {
                            Tm2[3]=(4.0*Tr2p1-Tr2p2)/3.0; Order[3]=2;
                        }
                    }
                }
                
                /*Calculate the distance using x and y direction */
                Coeff[0]=0; Coeff[1]=0; Coeff[2]=-1/(pow2(F[IJ_index])+eps);
                
                for (t=0; t<2; t++) {
                    switch(Order[t]) {
                        case 1:
                            Coeff[0]+=1; Coeff[1]+=-2*Tm[t]; Coeff[2]+=pow2(Tm[t]);
                            break;
                        case 2:
                            Coeff[0]+=(2.25); Coeff[1]+=-2*Tm2[t]*(2.25); Coeff[2]+=pow2(Tm2[t])*(2.25);
                            break;
                    }
                }
                roots(Coeff, ansroot);
                Tt=max(ansroot[0], ansroot[1]);
                
                /*Calculate the distance using the cross directions */
                if(usecross) {
                    Coeff[0]=0; Coeff[1]=0; Coeff[2]=-1/(pow2(F[IJ_index])+eps);
                    for (t=2; t<4; t++) {
                        switch(Order[t]) {
                            case 1:
                                Coeff[0]+=c1; Coeff[1]+=-2.0*c1*Tm[t]; Coeff[2]+=c1*pow2(Tm[t]);
                                break;
                            case 2:
                                Coeff[0]+=c1*2.25; Coeff[1]+=-2*c1*Tm2[t]*(2.25); Coeff[2]+=pow2(Tm2[t])*c1*2.25;
                                break;
                        }
                    }
                    if(Coeff[0]>0) {
                        roots(Coeff, ansroot);
                        Tt2=max(ansroot[0], ansroot[1]);
                        /*Select minimum distance value of both stensils */
                        Tt=min(Tt, Tt2);
                    }
                }
                /*Upwind condition check, current distance must be larger */
                /*then direct neighbours used in solution */
                /*(Will this ever happen?) */
                if(usecross) {
                    for(q=0; q<4; q++) { if(IsFinite(Tm[q])&&(Tt<Tm[q])) { Tt=Tm[minarray(Tm, 4)]+(1/(F[IJ_index]+eps));}}
                }
                else
                {
                    for(q=0; q<2; q++) { if(IsFinite(Tm[q])&&(Tt<Tm[q])) { Tt=Tm[minarray(Tm, 2)]+(1/(F[IJ_index]+eps));}}
                }
                
                /*Update distance in neigbour list or add to neigbour list */
                IJ_index=i+j*dims[0];
                if((T[IJ_index]>-1)&&T[IJ_index]<=listprop[0]) {
                    if(neg_listv[(int)T[IJ_index]]>Tt) {
                        listupdate(listval, listprop,    (int)T[IJ_index], Tt);
                    }
                }
                else {
                    /*If running out of memory at a new block */
                    if(neg_pos>=neg_free) {
                        neg_free+=100000;
                        neg_listx = (double *)realloc(neg_listx, neg_free*sizeof(double) );
                        neg_listy = (double *)realloc(neg_listy, neg_free*sizeof(double) );
                    }
                    list_add(listval, listprop, Tt);
                    neg_listv=listval[listprop[1]-1];
                    neg_listx[neg_pos]=i; neg_listy[neg_pos]=j;
                    T[IJ_index]=neg_pos;
                    neg_pos++;
                }
            }
        }
        
    } 
    /* Free memory */
    /* Destroy parameter list */
    destroy_list(listval, listprop);
    free(neg_listx);
    free(neg_listy);
    free(Frozen);
}





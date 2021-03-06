#include <math.h>
#include "mex.h"

#define DEBUG_ON 0
void classForest(int *mdim, int *ntest, int *nclass, int *maxcat,
        int *nrnodes, int *ntree, double *x, double *xbestsplit,
        double *pid, double *cutoff, double *countts, int *treemap,
        int *nodestatus, int *cat, int *nodeclass, int *jts,
        int *jet, int *bestvar, int *node, int *treeSize,
        int *keepPred, int *prox, double *proxMat, int *nodes,
		int *path_stat, int path_length);

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{ 
    if (DEBUG_ON) { mexPrintf("Number of parameters passed %d\n",nrhs);fflush(stdout);}
    
    int i;
    int p_size = mxGetM(prhs[0]); int mdim = p_size; // feature dimension
    int n_size = mxGetN(prhs[0]); int nsample = n_size; // number of samples
    int dimx[]={p_size, n_size};
    
    if (DEBUG_ON) { mexPrintf("p_size %d, n_size %d\n",p_size,n_size);fflush(stdout);}
    
    int nclass =  (int)mxGetScalar(prhs[11]);
    
    int* cat = (int*)mxCalloc(p_size,sizeof(int));
    for(i=0; i<p_size; i++) 
        cat[i]=1;
    
    int maxcat=1;
    int sampsize=n_size;
    int nsum = sampsize;
    int strata = 1;
    int addclass = 0;
    int importance=0;
    int localImp=0;
    int proximity=(int)mxGetScalar(prhs[13]);
    int oob_prox=0;
    int do_trace=1;
    int keep_forest=1;
    int replace=1;
    int stratify=0;
    int keep_inbag=0;
    int Options[]={addclass,importance,localImp,proximity,oob_prox,do_trace,keep_forest,replace,stratify,keep_inbag};
    
    int ntree=(int)mxGetScalar(prhs[2]); int nt=ntree;
    int ipi=0;
    double* cutoff=(double*)mxGetData(prhs[5]);
    
    int nodesize=1;
    double prox=1;
    double impout=p_size;
    double impSD=1;
    double impmat=1; 
    double *X = mxGetPr(prhs[0]);
    int nrnodes = (int)mxGetScalar(prhs[1]);
    if (DEBUG_ON) { mexPrintf("nrnodes %d\n",nrnodes);}
        
    int* ndbigtree = (int*) mxGetData(prhs[10]); 
    int* nodestatus = (int*) mxGetData(prhs[7]);
    int* bestvar = (int*) mxGetData(prhs[9]);
    int* treemap = (int*) mxGetData(prhs[6]);
    int* nodeclass = (int*) mxGetData(prhs[8]); //nodepred
    double* xbestsplit = (double*)mxGetData(prhs[3]);
    
    int testdat=0;
    double xts=1;
    int clts = 1; 
    int ntest = n_size;
    double* countts;// = (double*) mxCalloc(nclass * ntest,sizeof(double));
    int outclts = 1;
    int labelts=0;
    double proxts=1;
    double errts=1;
    //int* inbag = (int*) mxCalloc(n_size,sizeof(int));
    int* jts; 
    int* jet;
    int keepPred=(int)mxGetScalar(prhs[12]);
    int nodes=(int)mxGetScalar(prhs[14]);
    int* nodexts;
	int path_length = (int)mxGetScalar(prhs[15]);
    int* path_stat;
	
    int ndim = 2;
    int dims_ntest[]={1,1};
	
	if (path_length) 
    {
        dims_ntest[0] = ntest;
        dims_ntest[1] = ntree;
        plhs[5] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
        path_stat = (int*) mxGetData(plhs[5]);
    } 
    else 
    {
        dims_ntest[0] = ntest;
        dims_ntest[1] = 1;
        plhs[5] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
        path_stat = (int*) mxGetData(plhs[5]);
    }
    
    if (nodes) 
    {
        dims_ntest[0] = ntest;
        dims_ntest[1] = ntree;
        plhs[4] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
        nodexts = (int*)mxGetData(plhs[4]);
        //nodexts = (int*)mxCalloc(ntest*ntree,sizeof(int));
    } 
    else 
    {
        dims_ntest[0] = ntest;
        dims_ntest[1] = 1;
        plhs[4] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
        nodexts = (int*)mxGetData(plhs[4]);
        //nodexts = (int*)mxCalloc(ntest,sizeof(int));
    }
    
    double *proxMat;
        
    if(proximity)
    {
        //proxMat = (double*)mxCalloc(ntest*ntest,sizeof(double));
        dims_ntest[0] = ntest;
        dims_ntest[1] = ntest;
        plhs[3] = mxCreateNumericArray(ndim, dims_ntest, mxDOUBLE_CLASS, mxREAL);
        proxMat = (double*)mxGetData(plhs[3]);
    }
    else
    {
        dims_ntest[0] = 1;
        dims_ntest[1] = 1;
        plhs[3] = mxCreateNumericArray(ndim, dims_ntest, mxDOUBLE_CLASS, mxREAL);
        proxMat = (double*)mxGetData(plhs[3]);
        //proxMat = (double*)mxCalloc(1,sizeof(double));
        proxMat[0]=1;
    }
    int* treeSize = ndbigtree;
    double* pid = (double*)mxGetData(prhs[4]);
    
    if (DEBUG_ON) { 
        mexPrintf("ntest %d\n\n\n\n",ntest);
        mexPrintf("ntest %d\n",ntest);
        mexPrintf("p_size %d, n_size %d\n",p_size,n_size);fflush(stdout);
        mexPrintf("ntree %d\n",ntree);
        mexPrintf("nclass %d\n",nclass);
        mexPrintf("maxcat %d\n",maxcat);
        mexPrintf("nrnodes %d\n",nrnodes);
        mexPrintf("keepPred %d\n",keepPred);
        //mexPrintf("intProxmity %d\n",intProximity);
    }
    
    dims_ntest[0] = ntest;
    dims_ntest[1] = 1;
    plhs[0] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
    
    dims_ntest[0]=nclass;
    dims_ntest[1]=ntest;
    plhs[2] = mxCreateNumericArray(ndim, dims_ntest, mxDOUBLE_CLASS, mxREAL);
    jet = (int*)mxGetData(plhs[0]);
    
    if (keepPred) 
	{
        dims_ntest[0]=ntest;
        dims_ntest[1]=ntree;
        plhs[1] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
        jts = (int*)mxGetData(plhs[1]);
    } 
	else 
	{
        dims_ntest[0]=ntest;
        dims_ntest[1]=1;
        plhs[1] = mxCreateNumericArray(ndim, dims_ntest, mxINT32_CLASS, mxREAL);
        jts = (int*)mxGetData(plhs[1]);
    }
    
    countts = (double*)mxGetPr(plhs[2]);
    
    // realised in classRF.cpp
    classForest(&mdim, &ntest, &nclass, &maxcat,
                &nrnodes, &ntree, X, xbestsplit,
                pid, cutoff, countts, treemap,
                nodestatus, cat, nodeclass, jts,
                jet, bestvar, nodexts, treeSize,
                &keepPred, &proximity, proxMat, &nodes,
				path_stat, path_length);
   
    if (DEBUG_ON) { 
        mexPrintf("\n\n\nntest %d\n",ntest);
        mexPrintf("ndim=%d, dims_ntest={%d,%d}\n",ndim,dims_ntest[0],dims_ntest[1]);
        mexPrintf("p_size %d, n_size %d\n",p_size,n_size);fflush(stdout);
        mexPrintf("ntree %d\n",ntree);
        mexPrintf("nclass %d\n",nclass);
        mexPrintf("maxcat %d\n",maxcat);
        mexPrintf("nrnodes %d\n",nrnodes);
        mexPrintf("keepPred %d\n",keepPred);
        //mexPrintf("intProxmity %d\n",intProximity);
        mexPrintf("countts 0x%x\n",countts);
        mexPrintf("cat 0x%x\n",cat);
        mexPrintf("jts 0x%x\n",jts);
        mexPrintf("nodexts 0x%x\n",nodexts);
        mexPrintf("proxMat 0x%x\n",proxMat);
    }
    fflush(stdout);
    
    mxFree(cat);
    //mxFree(nodexts);
    //free(proxMat);    
}
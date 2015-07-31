/** \file
  \brief Extends Eigen::SparseMatrix class
*/

/** Create sparse matrix from R-triplet sparse matrix */
template<class Type>
Eigen::SparseMatrix<Type> asSparseMatrix(SEXP M){
  int *i=INTEGER(R_do_slot(M,install("i")));
  int *j=INTEGER(R_do_slot(M,install("j")));
  double *x=REAL(R_do_slot(M,install("x")));
  int n=LENGTH(R_do_slot(M,install("x")));
  int *dim=INTEGER(R_do_slot(M,install("Dim")));
  typedef Eigen::Triplet<Type> T;
  std::vector<T> tripletList;
  for(int k=0;k<n;k++){
    tripletList.push_back(T(i[k],j[k],x[k]));
  }
  Eigen::SparseMatrix<Type> mat(dim[0],dim[1]);
  mat.setFromTriplets(tripletList.begin(), tripletList.end());
  return mat;
}

/** Create sparse matrix from dense matrix */
template<class Type>
Eigen::SparseMatrix<Type> asSparseMatrix(matrix<Type> x){
  typedef Eigen::Triplet<Type> T;
  std::vector<T> tripletList;
  for(int i=0;i<x.rows();i++)
    for(int j=0;j<x.cols();j++)
      if( (x(i,j)!=Type(0)) || CppAD::Variable(x(i,j)) )
	tripletList.push_back(T(i,j,x(i,j)));
  Eigen::SparseMatrix<Type> mat(x.rows(),x.cols());
  mat.setFromTriplets(tripletList.begin(), tripletList.end());
  return mat;  
}

/** Create sparse vector from dense vector */
template<class Type>
Eigen::SparseVector<Type> asSparseVector(vector<Type> x){
  typedef Eigen::Triplet<Type> T;
  std::vector<T> tripletList;
  Eigen::SparseVector<Type> mat(x.rows());
  for(int i=0;i<x.rows();i++)
    if( (x(i)!=Type(0)) || CppAD::Variable(x(i)) )
      mat.coeffRef(i)=x(i);
  return mat;  
}

/** Kronecker product of two sparse matrices */
template <class Type>
Eigen::SparseMatrix<Type> kronecker(Eigen::SparseMatrix<Type> x,
				    Eigen::SparseMatrix<Type> y){
  typedef Eigen::Triplet<Type> T;
  typedef typename Eigen::SparseMatrix<Type>::InnerIterator Iterator;
  std::vector<T> tripletList;
  int n1=x.rows(),n2=x.cols(),n3=y.rows(),n4=y.cols();
  int i,j,k,l;
  // Loop over nonzeros of x
  for (int cx=0; cx<x.outerSize(); cx++)
    for (Iterator itx(x,cx); itx; ++itx)
      // Loop over nonzeros of y
      for (int cy=0; cy<y.outerSize(); cy++)
  	for (Iterator ity(y,cy); ity; ++ity)
  	  {
  	    i=itx.row();
  	    j=itx.col();
  	    k=ity.row();
  	    l=ity.col();
  	    tripletList.push_back(T(i*n3+k,j*n4+l, itx.value()*ity.value() ));
   	  }
  Eigen::SparseMatrix<Type> mat(n1*n3,n2*n4);
  mat.setFromTriplets(tripletList.begin(), tripletList.end());
  return mat;
}

/** Solve discrete Lyapunov equation V=AVA'+I */
template <class Type>
matrix<Type> discrLyap(matrix<Type> A_){
  matrix<Type> I_(A_.rows(),A_.cols());
  I_.setIdentity();
  /* Sparse representations */
  Eigen::SparseMatrix<Type> A=asSparseMatrix(A_);
  Eigen::SparseMatrix<Type> I=asSparseMatrix(I_);
  /* Kronecker */
  Eigen::SparseMatrix<Type> AxA=kronecker(A,A);
  Eigen::SparseMatrix<Type> IxI=kronecker(I,I);
  matrix<Type> vecI=I_.vec().matrix();
  Eigen::SparseMatrix<Type> C=IxI-AxA;
  /* Solve system C*vecV=vecI.
     Note: No assumptions on symmetry or eigenvalue-range of C. Therefore 
     rewrite it as (C'*C)*vecV=(C*vecI). Since (C'*C) is symmetric, the 
     LDL'-factorization can be applied.
   */
  Eigen::SimplicialLDLT< Eigen::SparseMatrix<Type> > ldlt(C.transpose()*C);
  matrix<Type> z = ldlt.solve(C.transpose()*vecI);
  matrix<Type> V(A_.rows(),A_.cols());
  for(int i=0;i<z.size();i++)V(i)=z(i);
  return V;
}

/** Inverse of PD sparse matrix */
template <class Type>
matrix<Type> invertSparseMatrix(Eigen::SparseMatrix<Type> A){
  matrix<Type> I(A.rows(),A.cols());
  I.setIdentity();
  Eigen::SimplicialLDLT< Eigen::SparseMatrix<Type> > ldlt(A);
  matrix<Type> ans = ldlt.solve(I);
  return ans;
}

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2013 Soumyajit De
 * Written (W) 2015 Yingrui Chang, Fernando Iglesias
 */

#include <gtest/gtest.h>

#include <shogun/lib/common.h>
#include <shogun/lib/SGVector.h>
#include <shogun/lib/SGSparseVector.h>
#include <shogun/lib/SGSparseMatrix.h>
#include <shogun/io/LibSVMFile.h>
#include <shogun/lib/SGMatrix.h>
#include <shogun/mathematics/Random.h>

using namespace shogun;

#ifdef HAVE_EIGEN3
#include <shogun/mathematics/eigen3.h>
using namespace Eigen;
#endif // HAVE_EIGEN3

//function can generate both sparse and dense matrix accroding to sparse convention
template<class MatrixType>
void GenerateMatrix(float64_t sparseLevel, int32_t m, int32_t n, int32_t randSeed, MatrixType* matrix)
{
	CRandom randGenerator(randSeed);
	for (index_t i=0; i<m; ++i)
		for (index_t j=0; j<n; ++j)
		{
			float64_t randomNumber=randGenerator.random(0.0,1.0);
			if (randomNumber<=sparseLevel)
				(*matrix)(i,j)=randomNumber*100;
		}
}

#ifdef HAVE_EIGEN3
TEST(SGSparseMatrix, multiply_float64_int32)
{
	const int32_t size=10;
	const int32_t num_feat=size/2;

	SGSparseMatrix<float64_t> m(size, size);
	for (index_t i=0; i<size; ++i)
	{
		m.sparse_matrix[i]=SGSparseVector<float64_t>(num_feat);
		for (index_t j=0; j<num_feat; ++j)
		{
			SGSparseVectorEntry<float64_t> entry;
			entry.feat_index=(j+1)*2;
			entry.entry=0.5;
			m.sparse_matrix[i].features[j]=entry;
		}
	}

	SGVector<int32_t> v(size);
	v.set_const(2);

	SGVector<float64_t> result=m*v;
	Map<VectorXd> r(result.vector, result.vlen);

	EXPECT_NEAR(r.norm(), 12.64911064067351809115, 1E-16);
}

TEST(SGSparseMatrix, multiply_complex128_int32)
{
	const int32_t size=10;
	const int32_t num_feat=size/2;

	SGSparseMatrix<complex128_t> m(size, size);
	for (index_t i=0; i<size; ++i)
	{
		m.sparse_matrix[i]=SGSparseVector<complex128_t>(num_feat);
		for (index_t j=0; j<num_feat; ++j)
		{
			SGSparseVectorEntry<complex128_t> entry;
			entry.feat_index=(j+1)*2;
			entry.entry=complex128_t(0.5, 0.75);
			m.sparse_matrix[i].features[j]=entry;
		}
	}

	SGVector<int32_t> v(size);
	v.set_const(2);

	SGVector<complex128_t> result=m*v;
	Map<VectorXcd> r(result.vector, result.vlen);

	EXPECT_NEAR(r.norm(), 22.80350850198275836078, 1E-16);
}

TEST(SGSparseMatrix, multiply_complex128_float64)
{
	const int32_t size=10;
	const int32_t num_feat=size/2;

	SGSparseMatrix<complex128_t> m(size, size);
	for (index_t i=0; i<size; ++i)
	{
		m.sparse_matrix[i]=SGSparseVector<complex128_t>(num_feat);
		for (index_t j=0; j<num_feat; ++j)
		{
			SGSparseVectorEntry<complex128_t> entry;
			entry.feat_index=(j+1)*2;
			entry.entry=complex128_t(0.5, 0.75);
			m.sparse_matrix[i].features[j]=entry;
		}
	}

	SGVector<float64_t> v(size);
	v.set_const(2);

	SGVector<complex128_t> result=m*v;
	Map<VectorXcd> r(result.vector, result.vlen);

	EXPECT_NEAR(r.norm(), 22.80350850198275836078, 1E-16);
}
#endif // HAVE_EIGEN3

TEST(SGSparseMatrix, access_by_index)
{
	const index_t size=2;

	SGSparseMatrix<int32_t> m(size, size);
	for (index_t i=0; i<size; ++i)
		m(i,i)=i+1;
	m.sort_features();

	for (index_t i=0; i<size; ++i)
		EXPECT_EQ(m(i,i), i+1);
}

TEST(SGSparseMatrix, io_libsvm)
{
	const int32_t size=10;
	const int32_t num_feat=size/2;

	CLibSVMFile* fin;
	CLibSVMFile* fout;

	SGSparseMatrix<float64_t> m(size, size);
	SGSparseMatrix<float64_t> m_from_file;
	SGVector<float64_t> labels(size);
	for (index_t i=0; i<size; ++i)
	{
		m.sparse_matrix[i]=SGSparseVector<float64_t>(num_feat);
		labels.vector[i]=(float64_t) (i%2);
		for (index_t j=0; j<num_feat; ++j)
		{
			SGSparseVectorEntry<float64_t> entry;
			entry.feat_index=(j+1)*2 - 1;
			entry.entry=0.5;
			m.sparse_matrix[i].features[j]=entry;
		}
	}

	fout=new CLibSVMFile("SGSparseMatrix_io_libsvm_output.txt",'w', NULL);
	m.save_with_labels(fout, labels);
	SG_UNREF(fout);

	fin=new CLibSVMFile("SGSparseMatrix_io_libsvm_output.txt",'r', NULL);
	SGVector<float64_t> labels_from_file=m_from_file.load_with_labels(fin, false);
	SG_UNREF(fin);

	for (int32_t i=0; i<size; i++)
	{
		EXPECT_EQ(labels[i], labels_from_file[i]);
		for (index_t j=0; j<num_feat; ++j)
		{
			EXPECT_EQ(m[i].features[j].feat_index, m_from_file[i].features[j].feat_index);
			EXPECT_NEAR(m[i].features[j].entry, m_from_file[i].features[j].entry, 1E-14);
		}
	}

	//remove file generated by test
	unlink("SGSparseMatrix_io_libsvm_output.txt");
}

TEST(SGSparseMatrix, access_by_index_non_square)
{
	const float64_t sparseLevel=0.1;
	const index_t numberOfFeatures=50;
	const index_t numberOfVectors=100;
	const index_t randSeed=0;

	//generate a sparse and dense matrix and compare this entries
	SGSparseMatrix<float64_t> sparseMatrix(numberOfFeatures, numberOfVectors);
	GenerateMatrix<SGSparseMatrix<float64_t> >(sparseLevel, numberOfFeatures, numberOfVectors, randSeed, &sparseMatrix);

	SGMatrix<float64_t> denseMatrix(numberOfFeatures, numberOfVectors);
	denseMatrix.zero();
	GenerateMatrix<SGMatrix<float64_t> >(sparseLevel, numberOfFeatures, numberOfVectors, randSeed, &denseMatrix);

	for (index_t featIndex=0; featIndex<numberOfFeatures; ++featIndex)
		for (index_t vecIndex=0; vecIndex<numberOfVectors; ++vecIndex)
			EXPECT_EQ(sparseMatrix(featIndex,vecIndex), denseMatrix(featIndex,vecIndex));
}

TEST(SGSparseMatrix, get_transposed_more_features)
{
	const float64_t sparseLevel=0.1;
	const index_t numberOfFeatures=100;
	const index_t numberOfVectors=50;
	const index_t randSeed=0;

	SGSparseMatrix<float64_t> sparseMatrix(numberOfFeatures, numberOfVectors);
	GenerateMatrix<SGSparseMatrix<float64_t> >(sparseLevel, numberOfFeatures, numberOfVectors, randSeed, &sparseMatrix);

	SGSparseMatrix<float64_t> sparseMatrixT=sparseMatrix.get_transposed();

	//check dimension
	EXPECT_EQ(sparseMatrix.num_features, sparseMatrixT.num_vectors);
	EXPECT_EQ(sparseMatrix.num_vectors, sparseMatrixT.num_features);

	//check contents
	for (index_t featIndex=0; featIndex<numberOfFeatures; ++featIndex)
		for (index_t vecIndex=0; vecIndex<numberOfVectors; ++vecIndex)
			EXPECT_EQ(sparseMatrix(featIndex,vecIndex), sparseMatrixT(vecIndex,featIndex));
}

TEST(SGSparseMatrix, get_transposed_more_vectors)
{
	const float64_t sparseLevel=0.1;
	const index_t numberOfFeatures=50;
	const index_t numberOfVectors=100;
	const index_t randSeed=0;

	SGSparseMatrix<float64_t> sparseMatrix(numberOfFeatures, numberOfVectors);
	GenerateMatrix<SGSparseMatrix<float64_t> >(sparseLevel, numberOfFeatures, numberOfVectors, randSeed, &sparseMatrix);

	SGSparseMatrix<float64_t> sparseMatrixT=sparseMatrix.get_transposed();

	//check dimension
	EXPECT_EQ(sparseMatrix.num_features, sparseMatrixT.num_vectors);
	EXPECT_EQ(sparseMatrix.num_vectors, sparseMatrixT.num_features);

	//check contents
	for (index_t featIndex=0; featIndex<numberOfFeatures; ++featIndex)
		for (index_t vecIndex=0; vecIndex<numberOfVectors; ++vecIndex)
			EXPECT_EQ(sparseMatrix(featIndex,vecIndex), sparseMatrixT(vecIndex,featIndex));
}

TEST(SGSparseMatrix, from_dense)
{

	const float64_t sparseLevel=0.1;
	const index_t numberOfFeatures=50;
	const index_t numberOfVectors=100;
	const index_t randSeed=0;

	SGMatrix<float64_t> denseMatrix(numberOfFeatures, numberOfVectors);
	denseMatrix.zero();
	GenerateMatrix<SGMatrix<float64_t> >(sparseLevel, numberOfFeatures, numberOfVectors, randSeed, &denseMatrix);

	SGSparseMatrix<float64_t> sparseMatrix;
	sparseMatrix.from_dense(denseMatrix);

	//check dimension
	EXPECT_EQ(sparseMatrix.num_features, numberOfFeatures);
	EXPECT_EQ(sparseMatrix.num_vectors, numberOfVectors);

	//check contents
	for (index_t featIndex=0; featIndex<numberOfFeatures; ++featIndex)
		for (index_t vecIndex=0; vecIndex<numberOfVectors; ++vecIndex)
			EXPECT_EQ(sparseMatrix(featIndex,vecIndex), denseMatrix(featIndex,vecIndex));
}

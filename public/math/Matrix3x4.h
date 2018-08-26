/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef PUBLIC_MATH_MATRIX3X4_H
#define PUBLIC_MATH_MATRIX3X4_H

#include <cstring>

/**
*	3x4 Row major matrix.
*/
struct Matrix3x4 final
{
	/**
	*	Constructor.
	*	@param bInitialize Whether to make this a zero matrix or leave it uninitialized.
	*/
	Matrix3x4( const bool bInitialize = false )
	{
		if( bInitialize )
		{
			MakeZero();
		}
	}

	/**
	*	Constructor.
	*	Initializes the matrix to the given values.
	*/
	Matrix3x4( vec_t m00, vec_t m01, vec_t m02, vec_t m03,
			   vec_t m10, vec_t m11, vec_t m12, vec_t m13,
			   vec_t m20, vec_t m21, vec_t m22, vec_t m23 )
	{
		Init( m00, m01, m02, m03,
			  m10, m11, m12, m13,
			  m20, m21, m22, m23 );
	}

	/**
	*	Copy constructor.
	*/
	Matrix3x4( const Matrix3x4& other )
	{
		Init( other );
	}

	/**
	*	Assignment operator.
	*/
	Matrix3x4& operator=( const Matrix3x4& other )
	{
		Init( other );

		return *this;
	}

	/**
	*	Initializes the matrix to the given values.
	*/
	void Init( vec_t m00, vec_t m01, vec_t m02, vec_t m03, 
			   vec_t m10, vec_t m11, vec_t m12, vec_t m13,
			   vec_t m20, vec_t m21, vec_t m22, vec_t m23 )
	{
		matrix[ 0 ][ 0 ] = m00; matrix[ 0 ][ 1 ] = m01; matrix[ 0 ][ 2 ] = m02; matrix[ 0 ][ 3 ] = m03;
		matrix[ 1 ][ 0 ] = m10; matrix[ 1 ][ 1 ] = m11; matrix[ 1 ][ 2 ] = m12; matrix[ 1 ][ 3 ] = m13;
		matrix[ 2 ][ 0 ] = m20; matrix[ 2 ][ 1 ] = m21; matrix[ 2 ][ 2 ] = m22; matrix[ 2 ][ 3 ] = m23;
	}

	/**
	*	Initializes the matrix to the other matrix's values.
	*/
	void Init( const Matrix3x4& other )
	{
		memcpy( this, &other, sizeof( *this ) );
	}

	/**
	*	Makes this a zero matrix.
	*/
	void MakeZero()
	{
		Init( 0, 0, 0, 0,
			  0, 0, 0, 0,
			  0, 0, 0, 0 );
	}

	/**
	*	Makes this an identity matrix.
	*/
	void MakeIdentity()
	{
		Init( 1, 0, 0, 0,
			  0, 1, 0, 0, 
			  0, 0, 1, 0 );
	}

	/**
	*	Index operator. Gets a row by index.
	*	@param uiIndex Index.
	*	@return Row. Returns a 3D vector, but the operator[] of that class can access the 4th element.
	*/
	const Vector& operator[]( const size_t uiIndex ) const
	{
		return *reinterpret_cast<const Vector*>( matrix[ uiIndex ] );
	}

	/**
	*	@copydoc operator[]( const size_t uiIndex ) const
	*/
	Vector& operator[]( const size_t uiIndex )
	{
		return *reinterpret_cast<Vector*>( matrix[ uiIndex ] );
	}

	vec_t matrix[ 3 ][ 4 ];
};

#endif //PUBLIC_MATH_MATRIX3X4_H
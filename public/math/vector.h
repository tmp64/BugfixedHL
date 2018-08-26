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
#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
#include <cstring>

//=========================================================
// 2DVector - used for many pathfinding and many other 
// operations that are treated as planar rather than 3d.
//=========================================================
class Vector2D
{
public:
	inline Vector2D(void)									{ }
	inline Vector2D( const Vector2D& other ) = default;
	inline Vector2D& operator=( const Vector2D& other ) = default;
	inline Vector2D(float X, float Y)						{ x = X; y = Y; }
	inline bool operator==( const Vector2D& v )		const	{ return x == v.x && y == v.y; }
	inline bool operator!=( const Vector2D& v )		const	{ return !( *this == v ); }
	inline Vector2D operator+(const Vector2D& v)	const	{ return Vector2D(x+v.x, y+v.y);	}
	inline Vector2D operator-(const Vector2D& v)	const	{ return Vector2D(x-v.x, y-v.y);	}
	inline Vector2D operator*(float fl)				const	{ return Vector2D(x*fl, y*fl);	}
	inline Vector2D operator/(float fl)				const	{ return Vector2D(x/fl, y/fl);	}
	
	inline float Length(void)						const	{ return sqrt(x*x + y*y );		}

	inline Vector2D Normalize ( void ) const
	{
		Vector2D vec2;

		float flLen = Length();
		if ( flLen == 0 )
		{
			return Vector2D( 0, 0 );
		}
		else
		{
			flLen = 1 / flLen;
			return Vector2D( x * flLen, y * flLen );
		}
	}

	vec_t	x, y;
};

inline float DotProduct(const Vector2D& a, const Vector2D& b) { return( a.x*b.x + a.y*b.y ); }
inline Vector2D operator*(float fl, const Vector2D& v)	{ return v * fl; }

//=========================================================
// 3D Vector
//=========================================================
class Vector						// same data-layout as engine's vec3_t,
{								//		which is a vec_t[3]
public:
	// Construction/destruction
	inline Vector()									{}
	inline Vector(float X, float Y, float Z)		{ x = X; y = Y; z = Z;						}

	/**
	*	Initializes all members with flScalar.
	*	Explicit to prevent implicit conversion issues.
	*/
	inline explicit Vector( vec_t flScalar )
		: x( flScalar )
		, y( flScalar )
		, z( flScalar )
	{
	}
	inline Vector(const Vector& v)					{ x = v.x; y = v.y; z = v.z;				} 

	// Operators
	inline Vector operator-(void) const				{ return Vector(-x,-y,-z);				}
	inline bool operator==(const Vector& v) const	{ return x==v.x && y==v.y && z==v.z;	}
	inline bool operator!=(const Vector& v) const	{ return !(*this==v);					}
	inline Vector operator+(const Vector& v) const	{ return Vector(x+v.x, y+v.y, z+v.z);	}
	inline Vector operator-(const Vector& v) const	{ return Vector(x-v.x, y-v.y, z-v.z);	}
	inline Vector operator*(float fl) const			{ return Vector(x*fl, y*fl, z*fl);		}
	inline Vector operator/(float fl) const			{ return Vector(x/fl, y/fl, z/fl);		}

	inline Vector operator+( float fl ) const		{ return Vector( x + fl, y + fl, z + fl ); }
	inline Vector operator-( float fl ) const		{ return Vector( x - fl, y - fl, z - fl ); }
	
	// Methods
	inline void CopyToArray(float* rgfl) const		{ rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; }
	inline float Length(void) const					{ return sqrt(x*x + y*y + z*z); }
	operator float *()								{ return &x; } // Vectors will now automatically convert to float * when needed
	operator const float *() const					{ return &x; } // Vectors will now automatically convert to float * when needed

	/**
	*	Normalizes the vector's components.
	*	@return Old length.
	*/
	inline vec_t NormalizeInPlace()
	{
		float flLen = Length();

		if( flLen == 0 )
		{
			x = y = 0;
			z = 1;
			return flLen;
		}

		const float flInvertedLen = 1 / flLen;

		x *= flInvertedLen;
		y *= flInvertedLen;
		z *= flInvertedLen;

		return flLen;
	}

	inline Vector Normalize() const
	{
		Vector vec = *this;

		vec.NormalizeInPlace();

		return vec;
	}

	inline Vector2D Make2D ( void ) const
	{
		Vector2D	Vec2;

		Vec2.x = x;
		Vec2.y = y;

		return Vec2;
	}
	inline float Length2D(void) const					{ return sqrt(x*x + y*y); }

	inline void Clear()
	{
		x = y = z = 0;
	}

	inline Vector& operator=( const float flScalar )
	{
		x = y = z = flScalar;

		return *this;
	}

	// Members
	vec_t x, y, z;
};
inline Vector operator*(float fl, const Vector& v)	{ return v * fl; }
inline Vector operator+( float fl, const Vector& v ) { return v + fl; }
inline Vector operator-( float fl, const Vector& v ) { return v - fl; }
inline float DotProduct(const Vector& a, const Vector& b) { return(a.x*b.x+a.y*b.y+a.z*b.z); }
inline Vector CrossProduct(const Vector& a, const Vector& b) { return Vector( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x ); }

/**
*	Same data-layout as engine's vec4_t, which is a vec_t[4].
*/
class Vector4D final
{
public:
	/**
	*	Default constructor.
	*/
	Vector4D() {}

	/**
	*	Constructor. Initializes the vector to the given values.
	*/
	Vector4D( vec_t flScalar )
		: x( flScalar )
		, y( flScalar )
		, z( flScalar )
		, w( flScalar )
	{
	}

	/**
	*	Constructor. Initializes the vector to the given values.
	*/
	Vector4D( vec_t x, vec_t y, vec_t z, vec_t w )
		: x( x )
		, y( y )
		, z( z )
		, w( w )
	{
	}

	/**
	*	Copy constructor.
	*/
	Vector4D( const Vector4D& other ) = default;

	/**
	*	Assignment operator.
	*/
	Vector4D& operator=( const Vector4D& other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;

		return *this;
	}

	/**
	*	Scalar assignment operator.
	*/
	Vector4D& operator=( const vec_t flScalar )
	{
		x = y = z = w = flScalar;

		return *this;
	}

	bool operator==( const Vector4D& other ) const;

	bool operator!=( const Vector4D& other ) const;

	vec_t operator[]( const size_t uiIndex ) const;
	vec_t& operator[]( const size_t uiIndex );

	vec_t x, y, z, w;
};

inline bool Vector4D::operator==( const Vector4D& other ) const
{
	return x == other.x && y == other.y && z == other.z && w == other.w;
}

inline bool Vector4D::operator!=( const Vector4D& other ) const
{
	return !( *this == other );
}

inline vec_t Vector4D::operator[]( const size_t uiIndex ) const
{
	return reinterpret_cast<const vec_t*>( this )[ uiIndex ];
}

inline vec_t& Vector4D::operator[]( const size_t uiIndex )
{
	return reinterpret_cast<vec_t*>( this )[ uiIndex ];
}

#endif

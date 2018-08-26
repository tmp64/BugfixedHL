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
// mathlib.h
#ifndef PUBLIC_MATH_MATHLIB_H
#define PUBLIC_MATH_MATHLIB_H

//TODO: tidy this header up - Solokiller

typedef float vec_t;

#include "vector.h"
#include "Matrix3x4.h"

extern const Vector vec3_origin;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

enum : size_t
{
	/**
	*	up / down
	*/
	PITCH	= 0,

	/**
	*	left / right
	*/
	YAW		= 1,

	/**
	*	fall over
	*/
	ROLL	= 2
};

extern const int nanmask;

/**
*	@return Whether the given float is Not a Number.
*/
inline bool IS_NAN( const float flValue )
{
	return ( ( ( *( int* ) &flValue ) & nanmask ) == nanmask );
}

/**
*	Normalizes the vector in place.
*	@return Vector length.
*/
float VectorNormalize( Vector& v );

void VectorMA( const Vector& veca, float scale, const Vector& vecb, Vector& vecc );

/**
*	Converts a directional vector to a yaw value.
*	@param vec Directional vector.
*	@return Yaw.
*/
float UTIL_VecToYaw( const Vector& vec );

/**
*	Converts an angle vector to directional vectors.
*	@param vecAngles Angles.
*	@param[ out ] vecForward Optional. Forward vector.
*	@param[ out ] vecRight Optional. Right vector.
*	@param[ out ] vecUp Optional. Up vector.
*/
void AngleVectors( const Vector& vecAngles, Vector* vecForward, Vector* vecRight, Vector* vecUp );

/**
*	Convenience function for cases where all 3 vectors are used. No need to pass pointers.
*/
inline void AngleVectors( const Vector& angles, Vector& forward, Vector& right, Vector& up )
{
	AngleVectors( angles, &forward, &right, &up );
}

void AngleVectorsTranspose( const Vector& angles, Vector* forward, Vector* right, Vector* up );

/**
*	Convenience function for cases where all 3 vectors are used. No need to pass pointers.
*/
inline void AngleVectorsTranspose( const Vector& angles, Vector& forward, Vector& right, Vector& up )
{
	AngleVectorsTranspose( angles, &forward, &right, &up );
}

void VectorMatrix( Vector& forward, Vector& right, Vector& up );

/**
*	Converts a directional vector to angles.
*	@param vecForward Directional vector.
*	@param[ out ] vecAngles Angles.
*/
void VectorAngles( const Vector& vecForward, Vector& vecAngles );

void AngleIMatrix( const Vector& angles, Matrix3x4& matrix );

void VectorTransform( const Vector& in1, const Matrix3x4& in2, Vector& out );

void NormalizeAngles( Vector& vecAngles );

float FixAngle( float angle );

void FixupAngles( Vector& v );

float AngleBetweenVectors( const Vector& v1, const Vector& v2 );

float Distance( const Vector& v1, const Vector& v2 );

void InterpolateAngles( Vector& start, Vector&end, Vector& output, float frac );

int Q_log2( int val );

float anglemod( float a );

float UTIL_AngleMod( float a );

float UTIL_AngleDiff( float destAngle, float srcAngle );

Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize );

float UTIL_Approach( float target, float value, float speed );
float UTIL_ApproachAngle( float target, float value, float speed );
float UTIL_AngleDistance( float next, float cur );

/**
*	Use for ease-in, ease-out style interpolation (accel/decel)
*	Used by ducking code.
*	@param value Value.
*	@param scale Acceleration scale.
*	@return Spline fraction.
*/
float UTIL_SplineFraction( float value, float scale );

// Sorta like FInViewCone, but for nonmonsters. 
extern float UTIL_DotPoints( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir );

void AngleMatrix( const Vector& angles, Matrix3x4& matrix );
void ConcatTransforms( const Matrix3x4& in1, const Matrix3x4& in2, Matrix3x4& out );
void QuaternionMatrix( const Vector4D& quaternion, Matrix3x4& matrix );
void QuaternionSlerp( const Vector4D& p, Vector4D& q, float t, Vector4D& qt );
void AngleQuaternion( const Vector& vecAngles, Vector4D& quaternion );

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

#endif //PUBLIC_MATH_MATHLIB_H
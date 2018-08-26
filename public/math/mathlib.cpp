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
// mathlib.cpp -- math primitives
#include <cstring>

#include "mathlib.h"

const Vector vec3_origin( 0, 0, 0 );

const int nanmask = 255 << 23;

float VectorNormalize( Vector& v )
{
	float	length, ilength;

	length = v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ];
	length = sqrt( length );		// FIXME

	if( length )
	{
		ilength = 1 / length;
		v[ 0 ] *= ilength;
		v[ 1 ] *= ilength;
		v[ 2 ] *= ilength;
	}

	return length;

}

void VectorMA( const Vector& veca, float scale, const Vector& vecb, Vector& vecc )
{
	vecc[ 0 ] = veca[ 0 ] + scale*vecb[ 0 ];
	vecc[ 1 ] = veca[ 1 ] + scale*vecb[ 1 ];
	vecc[ 2 ] = veca[ 2 ] + scale*vecb[ 2 ];
}

float UTIL_VecToYaw( const Vector& vec )
{
	//Extracted from VectorAngles. - Solokiller
	float yaw;

	if( vec[ 1 ] == 0 && vec[ 0 ] == 0 )
	{
		yaw = 0;
	}
	else
	{
		//Needs to floor the result to match the engine.
		yaw = static_cast<float>( floor( atan2( vec[ 1 ], vec[ 0 ] ) * 180 / M_PI ) );
		if( yaw < 0 )
			yaw += 360;
	}

	return yaw;
}

void AngleVectors( const Vector& vecAngles, Vector* vecForward, Vector* vecRight, Vector* vecUp )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = static_cast<float>( vecAngles[ YAW ] * ( M_PI * 2 / 360 ) );
	sy = sin( angle );
	cy = cos( angle );
	angle = static_cast<float>( vecAngles[ PITCH ] * ( M_PI * 2 / 360 ) );
	sp = sin( angle );
	cp = cos( angle );
	angle = static_cast<float>( vecAngles[ ROLL ] * ( M_PI * 2 / 360 ) );
	sr = sin( angle );
	cr = cos( angle );

	if( vecForward )
	{
		( *vecForward )[ 0 ] = cp*cy;
		( *vecForward )[ 1 ] = cp*sy;
		( *vecForward )[ 2 ] = -sp;
	}

	if( vecRight )
	{
		( *vecRight )[ 0 ] = ( -1 * sr*sp*cy + -1 * cr*-sy );
		( *vecRight )[ 1 ] = ( -1 * sr*sp*sy + -1 * cr*cy );
		( *vecRight )[ 2 ] = -1 * sr*cp;
	}

	if( vecUp )
	{
		( *vecUp )[ 0 ] = ( cr*sp*cy + -sr*-sy );
		( *vecUp )[ 1 ] = ( cr*sp*sy + -sr*cy );
		( *vecUp )[ 2 ] = cr*cp;
	}
}

void AngleVectorsTranspose( const Vector& angles, Vector* forward, Vector* right, Vector* up )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = static_cast<float>( angles[ YAW ] * ( M_PI * 2 / 360 ) );
	sy = sin( angle );
	cy = cos( angle );
	angle = static_cast<float>( angles[ PITCH ] * ( M_PI * 2 / 360 ) );
	sp = sin( angle );
	cp = cos( angle );
	angle = static_cast<float>( angles[ ROLL ] * ( M_PI * 2 / 360 ) );
	sr = sin( angle );
	cr = cos( angle );

	if( forward )
	{
		( *forward )[ 0 ] = cp*cy;
		( *forward )[ 1 ] = ( sr*sp*cy + cr*-sy );
		( *forward )[ 2 ] = ( cr*sp*cy + -sr*-sy );
	}
	if( right )
	{
		( *right )[ 0 ] = cp*sy;
		( *right )[ 1 ] = ( sr*sp*sy + cr*cy );
		( *right )[ 2 ] = ( cr*sp*sy + -sr*cy );
	}
	if( up )
	{
		( *up )[ 0 ] = -sp;
		( *up )[ 1 ] = sr*cp;
		( *up )[ 2 ] = cr*cp;
	}
}

void VectorMatrix( Vector& forward, Vector& right, Vector& up )
{
	Vector tmp;

	if( forward[ 0 ] == 0 && forward[ 1 ] == 0 )
	{
		right[ 0 ] = 1;
		right[ 1 ] = 0;
		right[ 2 ] = 0;
		up[ 0 ] = -forward[ 2 ];
		up[ 1 ] = 0;
		up[ 2 ] = 0;
		return;
	}

	tmp[ 0 ] = 0; tmp[ 1 ] = 0; tmp[ 2 ] = 1.0;
	right = CrossProduct( forward, tmp );
	right = right.Normalize();
	up = CrossProduct( right, forward );
	up = up.Normalize();
}

void VectorAngles( const Vector& vecForward, Vector& vecAngles )
{
	float	tmp, yaw, pitch;

	if( vecForward[ 1 ] == 0 && vecForward[ 0 ] == 0 )
	{
		yaw = 0;
		if( vecForward[ 2 ] > 0 )
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = static_cast<float>( atan2( vecForward[ 1 ], vecForward[ 0 ] ) * 180 / M_PI );
		if( yaw < 0 )
			yaw += 360;

		tmp = sqrt( vecForward[ 0 ] * vecForward[ 0 ] + vecForward[ 1 ] * vecForward[ 1 ] );
		pitch = static_cast<float>( atan2( vecForward[ 2 ], tmp ) * 180 / M_PI );
		if( pitch < 0 )
			pitch += 360;
	}

	vecAngles[ 0 ] = pitch;
	vecAngles[ 1 ] = yaw;
	vecAngles[ 2 ] = 0;
}

void AngleIMatrix( const Vector& angles, Matrix3x4& matrix )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = static_cast<float>( angles[ YAW ] * ( M_PI * 2 / 360 ) );
	sy = sin( angle );
	cy = cos( angle );
	angle = static_cast<float>( angles[ PITCH ] * ( M_PI * 2 / 360 ) );
	sp = sin( angle );
	cp = cos( angle );
	angle = static_cast<float>( angles[ ROLL ] * ( M_PI * 2 / 360 ) );
	sr = sin( angle );
	cr = cos( angle );

	// matrix = (YAW * PITCH) * ROLL
	matrix[ 0 ][ 0 ] = cp*cy;
	matrix[ 0 ][ 1 ] = cp*sy;
	matrix[ 0 ][ 2 ] = -sp;
	matrix[ 1 ][ 0 ] = sr*sp*cy + cr*-sy;
	matrix[ 1 ][ 1 ] = sr*sp*sy + cr*cy;
	matrix[ 1 ][ 2 ] = sr*cp;
	matrix[ 2 ][ 0 ] = ( cr*sp*cy + -sr*-sy );
	matrix[ 2 ][ 1 ] = ( cr*sp*sy + -sr*cy );
	matrix[ 2 ][ 2 ] = cr*cp;
	matrix[ 0 ][ 3 ] = 0.0;
	matrix[ 1 ][ 3 ] = 0.0;
	matrix[ 2 ][ 3 ] = 0.0;
}

void VectorTransform( const Vector& in1, const Matrix3x4& in2, Vector& out )
{
	out[ 0 ] = DotProduct( in1, in2[ 0 ] ) + in2[ 0 ][ 3 ];
	out[ 1 ] = DotProduct( in1, in2[ 1 ] ) + in2[ 1 ][ 3 ];
	out[ 2 ] = DotProduct( in1, in2[ 2 ] ) + in2[ 2 ][ 3 ];
}

void NormalizeAngles( Vector& vecAngles )
{
	// Normalize angles
	for( int i = 0; i < 3; ++i )
	{
		if( vecAngles[ i ] > 180.0 )
		{
			vecAngles[ i ] -= 360.0;
		}
		else if( vecAngles[ i ] < -180.0 )
		{
			vecAngles[ i ] += 360.0;
		}
	}
}

float FixAngle( float angle )
{
	while( angle < 0 )
		angle += 360;
	while( angle > 360 )
		angle -= 360;

	return angle;
}

void FixupAngles( Vector &v )
{
	v.x = FixAngle( v.x );
	v.y = FixAngle( v.y );
	v.z = FixAngle( v.z );
}

/*
===================
AngleBetweenVectors

===================
*/
float AngleBetweenVectors( const Vector& v1, const Vector& v2 )
{
	float angle;
	float l1 = v1.Length();
	float l2 = v2.Length();

	if( !l1 || !l2 )
		return 0.0f;

	angle = acos( DotProduct( v1, v2 ) ) / ( l1*l2 );
	angle = static_cast<float>( ( angle  * 180.0f ) / M_PI );

	return angle;
}

float Distance( const Vector& v1, const Vector& v2 )
{
	return ( v2 - v1 ).Length();
}

/*
===================
InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================
*/
void InterpolateAngles( Vector& start, Vector&end, Vector& output, float frac )
{
	int i;
	float ang1, ang2;
	float d;

	NormalizeAngles( start );
	NormalizeAngles( end );

	for( i = 0; i < 3; i++ )
	{
		ang1 = start[ i ];
		ang2 = end[ i ];

		d = ang2 - ang1;
		if( d > 180 )
		{
			d -= 360;
		}
		else if( d < -180 )
		{
			d += 360;
		}

		output[ i ] = ang1 + d * frac;
	}

	NormalizeAngles( output );
}

int Q_log2( int val )
{
	int answer = 0;
	while( val >>= 1 )
		++answer;
	return answer;
}

float anglemod( float a )
{
	a = static_cast<float>( ( 360.0 / 65536 ) * ( ( int ) ( a*( 65536 / 360.0 ) ) & 65535 ) );
	return a;
}

// ripped this out of the engine
float	UTIL_AngleMod( float a )
{
	if( a < 0 )
	{
		a = a + 360 * ( ( int ) ( a / 360 ) + 1 );
	}
	else if( a >= 360 )
	{
		a = a - 360 * ( ( int ) ( a / 360 ) );
	}
	// a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}

float UTIL_AngleDiff( float destAngle, float srcAngle )
{
	float delta;

	delta = destAngle - srcAngle;
	if( destAngle > srcAngle )
	{
		if( delta >= 180 )
			delta -= 360;
	}
	else
	{
		if( delta <= -180 )
			delta += 360;
	}
	return delta;
}

Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize )
{
	Vector sourceVector = input;

	if( sourceVector.x > clampSize.x )
		sourceVector.x -= clampSize.x;
	else if( sourceVector.x < -clampSize.x )
		sourceVector.x += clampSize.x;
	else
		sourceVector.x = 0;

	if( sourceVector.y > clampSize.y )
		sourceVector.y -= clampSize.y;
	else if( sourceVector.y < -clampSize.y )
		sourceVector.y += clampSize.y;
	else
		sourceVector.y = 0;

	if( sourceVector.z > clampSize.z )
		sourceVector.z -= clampSize.z;
	else if( sourceVector.z < -clampSize.z )
		sourceVector.z += clampSize.z;
	else
		sourceVector.z = 0;

	return sourceVector.Normalize();
}

float UTIL_Approach( float target, float value, float speed )
{
	float delta = target - value;

	if( delta > speed )
		value += speed;
	else if( delta < -speed )
		value -= speed;
	else
		value = target;

	return value;
}

float UTIL_ApproachAngle( float target, float value, float speed )
{
	target = UTIL_AngleMod( target );
	value = UTIL_AngleMod( target );

	float delta = target - value;

	// Speed is assumed to be positive
	if( speed < 0 )
		speed = -speed;

	if( delta < -180 )
		delta += 360;
	else if( delta > 180 )
		delta -= 360;

	if( delta > speed )
		value += speed;
	else if( delta < -speed )
		value -= speed;
	else
		value = target;

	return value;
}

float UTIL_AngleDistance( float next, float cur )
{
	float delta = next - cur;

	if( delta < -180 )
		delta += 360;
	else if( delta > 180 )
		delta -= 360;

	return delta;
}

float UTIL_SplineFraction( float value, float scale )
{
	value = scale * value;
	const float valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

//=========================================================
// UTIL_DotPoints - returns the dot product of a line from
// src to check and vecdir.
//=========================================================
float UTIL_DotPoints( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir )
{
	Vector2D	vec2LOS;

	vec2LOS = ( vecCheck - vecSrc ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	return DotProduct( vec2LOS, ( vecDir.Make2D() ) );
}

/*
====================
AngleMatrix

====================
*/
void AngleMatrix( const Vector& angles, Matrix3x4& matrix )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = static_cast<float>( angles[ YAW ] * ( M_PI * 2 / 360 ) );
	sy = sin( angle );
	cy = cos( angle );
	angle = static_cast<float>( angles[ PITCH ] * ( M_PI * 2 / 360 ) );
	sp = sin( angle );
	cp = cos( angle );
	angle = static_cast<float>( angles[ ROLL ] * ( M_PI * 2 / 360 ) );
	sr = sin( angle );
	cr = cos( angle );

	// matrix = (YAW * PITCH) * ROLL
	matrix[ 0 ][ 0 ] = cp*cy;
	matrix[ 1 ][ 0 ] = cp*sy;
	matrix[ 2 ][ 0 ] = -sp;
	matrix[ 0 ][ 1 ] = sr*sp*cy + cr*-sy;
	matrix[ 1 ][ 1 ] = sr*sp*sy + cr*cy;
	matrix[ 2 ][ 1 ] = sr*cp;
	matrix[ 0 ][ 2 ] = ( cr*sp*cy + -sr*-sy );
	matrix[ 1 ][ 2 ] = ( cr*sp*sy + -sr*cy );
	matrix[ 2 ][ 2 ] = cr*cp;
	matrix[ 0 ][ 3 ] = 0.0;
	matrix[ 1 ][ 3 ] = 0.0;
	matrix[ 2 ][ 3 ] = 0.0;
}

/*
================
ConcatTransforms

================
*/
void ConcatTransforms( const Matrix3x4& in1, const Matrix3x4& in2, Matrix3x4& out )
{
	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 0 ][ 3 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 3 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 0 ][ 3 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 3 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 3 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 1 ][ 3 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 3 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 3 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 2 ][ 3 ];
}

// angles index are not the same as ROLL, PITCH, YAW

/*
====================
AngleQuaternion

====================
*/
void AngleQuaternion( const Vector& vecAngles, Vector4D& quaternion )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	// FIXME: rescale the inputs to 1/2 angle
	angle = static_cast<float>( vecAngles[ 2 ] * 0.5 );
	sy = sin( angle );
	cy = cos( angle );
	angle = static_cast<float>( vecAngles[ 1 ] * 0.5 );
	sp = sin( angle );
	cp = cos( angle );
	angle = static_cast<float>( vecAngles[ 0 ] * 0.5 );
	sr = sin( angle );
	cr = cos( angle );

	quaternion[ 0 ] = sr*cp*cy - cr*sp*sy; // X
	quaternion[ 1 ] = cr*sp*cy + sr*cp*sy; // Y
	quaternion[ 2 ] = cr*cp*sy - sr*sp*cy; // Z
	quaternion[ 3 ] = cr*cp*cy + sr*sp*sy; // W
}

/*
====================
QuaternionSlerp

====================
*/
void QuaternionSlerp( const Vector4D& p, Vector4D& q, float t, Vector4D& qt )
{
	int i;
	float	omega, cosom, sinom, sclp, sclq;

	// decide if one of the quaternions is backwards
	float a = 0;
	float b = 0;

	for( i = 0; i < 4; i++ )
	{
		a += ( p[ i ] - q[ i ] )*( p[ i ] - q[ i ] );
		b += ( p[ i ] + q[ i ] )*( p[ i ] + q[ i ] );
	}
	if( a > b )
	{
		for( i = 0; i < 4; i++ )
		{
			q[ i ] = -q[ i ];
		}
	}

	cosom = p[ 0 ] * q[ 0 ] + p[ 1 ] * q[ 1 ] + p[ 2 ] * q[ 2 ] + p[ 3 ] * q[ 3 ];

	if( ( 1.0 + cosom ) > 0.000001 )
	{
		if( ( 1.0 - cosom ) > 0.000001 )
		{
			omega = acos( cosom );
			sinom = sin( omega );
			sclp = static_cast<float>( sin( ( 1.0 - t )*omega ) / sinom );
			sclq = sin( t*omega ) / sinom;
		}
		else
		{
			sclp = 1.0f - t;
			sclq = t;
		}
		for( i = 0; i < 4; i++ ) {
			qt[ i ] = sclp * p[ i ] + sclq * q[ i ];
		}
	}
	else
	{
		qt[ 0 ] = -q[ 1 ];
		qt[ 1 ] = q[ 0 ];
		qt[ 2 ] = -q[ 3 ];
		qt[ 3 ] = q[ 2 ];
		sclp = static_cast<float>( sin( ( 1.0 - t ) * ( 0.5 * M_PI ) ) );
		sclq = static_cast<float>( sin( t * ( 0.5 * M_PI ) ) );
		for( i = 0; i < 3; i++ )
		{
			qt[ i ] = sclp * p[ i ] + sclq * qt[ i ];
		}
	}
}

/*
====================
QuaternionMatrix

====================
*/
void QuaternionMatrix( const Vector4D& quaternion, Matrix3x4& matrix )
{
	matrix[ 0 ][ 0 ] = static_cast<float>( 1.0 - 2.0 * quaternion[ 1 ] * quaternion[ 1 ] - 2.0 * quaternion[ 2 ] * quaternion[ 2 ] );
	matrix[ 1 ][ 0 ] = static_cast<float>( 2.0 * quaternion[ 0 ] * quaternion[ 1 ] + 2.0 * quaternion[ 3 ] * quaternion[ 2 ] );
	matrix[ 2 ][ 0 ] = static_cast<float>( 2.0 * quaternion[ 0 ] * quaternion[ 2 ] - 2.0 * quaternion[ 3 ] * quaternion[ 1 ] );

	matrix[ 0 ][ 1 ] = static_cast<float>( 2.0 * quaternion[ 0 ] * quaternion[ 1 ] - 2.0 * quaternion[ 3 ] * quaternion[ 2 ] );
	matrix[ 1 ][ 1 ] = static_cast<float>( 1.0 - 2.0 * quaternion[ 0 ] * quaternion[ 0 ] - 2.0 * quaternion[ 2 ] * quaternion[ 2 ] );
	matrix[ 2 ][ 1 ] = static_cast<float>( 2.0 * quaternion[ 1 ] * quaternion[ 2 ] + 2.0 * quaternion[ 3 ] * quaternion[ 0 ] );

	matrix[ 0 ][ 2 ] = static_cast<float>( 2.0 * quaternion[ 0 ] * quaternion[ 2 ] + 2.0 * quaternion[ 3 ] * quaternion[ 1 ] );
	matrix[ 1 ][ 2 ] = static_cast<float>( 2.0 * quaternion[ 1 ] * quaternion[ 2 ] - 2.0 * quaternion[ 3 ] * quaternion[ 0 ] );
	matrix[ 2 ][ 2 ] = static_cast<float>( 1.0 - 2.0 * quaternion[ 0 ] * quaternion[ 0 ] - 2.0 * quaternion[ 1 ] * quaternion[ 1 ] );
}
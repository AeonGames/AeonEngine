/*
Copyright (C) 2009-2017 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef AEONGAMES_MATH_H
#define AEONGAMES_MATH_H
/*! \file
    \brief Inline functions related to 3D Math.
    \author Rodrigo Hernandez.

    All math functions have been defined as inline and don't belong to any class,
    matrices and vectors are represented as float arrays, this is done to avoid castings
    as well as being able to perform operations on contiguous memory.

    Matrices are defined in the same way OpenGL defines them, as a float arrays with 16 elements,
    matrix notation is done in column mayor format to fit OpenGL documentation as well
    (see <a href="http://www.opengl.org/resources/faq/technical/transformations.htm">OpenGL FAQ</a>).

    The matrix m[16] would then be printed as:
    \f[
    \left( \begin{array}{cccc}
    m[0] & m[4] & m[8] & m[12] \\
    m[1] & m[5] & m[9] & m[13] \\
    m[2] & m[6] & m[10] & m[14] \\
    m[3] & m[7] & m[11] & m[15] \end{array} \right)
    \f]

    &copy; 2009-2017 Rodrigo Hernandez
*/

#include <cmath>
#include <cassert>
#include <cstring>
#include <climits>
#include <cfloat>
#include <cstdio>

/*! \name Constants */
// @{
/// The pi Constant
const float PI = 3.1415926535897932384626433832795f;
/// Two times pi
const float TWOPI = 6.2831853071795864769252867665590f;
/// Pi divided between 180
const float PIOVER180 = 0.017453292519943295769236907684883f;
/// 180 divided between pi
const float ONE80OVERPI = 57.295779513082320876798154814105f;
/// Floating point comparison tolerance
const float FLT_TOLERANCE = sqrtf ( FLT_EPSILON );
// @}

inline void RotateVectorByQuat ( const float* q, const float* v, float* out );

/*! \name General Utility Functions */
// @{
/*! \brief Convert degrees to radians.
    \param deg Degree value to convert.
    \return Value in radians.
*/
inline float DEG2RAD ( float deg )
{
    return PIOVER180 * deg;
}
/*! \brief Convert radians to degrees.
    \param rad Radian value to convert.
    \return Value in degrees.
*/
inline float RAD2DEG ( float rad )
{
    return ONE80OVERPI * rad;
}
/*! \brief Single pressision floating point absolute value.

    Returns the absolute (positive) value of a single floating point value.
    \param x [in] Value to evaluate.
    \return Absolute value of x.
*/
inline float sabs ( float x )
{
    return x < 0 ? -x : x;
}
// @}
/*! \name Vector Functions */
// @{
//-----------VECTORS-------------
/*! \brief Cross product.
    \param v1 [in] Left side vector.
    \param v2 [in] Right side vector.
    \param dst [out] Destination vector.
    \return Pointer to destination vector, same as dst.
*/
inline float* Cross3 ( float* v1, float* v2, float* dst )
{
    float r[3];
    r[0] = ( ( v1[1] * v2[2] ) - ( v1[2] * v2[1] ) );
    r[1] = ( ( v1[2] * v2[0] ) - ( v1[0] * v2[2] ) );
    r[2] = ( ( v1[0] * v2[1] ) - ( v1[1] * v2[0] ) );
    memcpy ( dst, r, sizeof ( float ) * 3 );
    return dst;
}

/*! \brief Calculate vector length.
    \param v [in] Vector to calculate length of.
    \return Vector length.
*/
inline float Length ( float const * const v )
{
    return ( float ) sqrtf ( ( v[0] * v[0] ) + ( v[1] * v[1] ) + ( v[2] * v[2] ) );
}

/*! \brief Calculate 4 element vector length.
    \param v [in] Vector to calculate length of.
    \return Vector length.
*/
inline float Length4 ( float* v )
{
    return ( float ) sqrtf ( ( v[0] * v[0] ) + ( v[1] * v[1] ) + ( v[2] * v[2] ) + ( v[3] * v[3] ) );
}

/*! \brief 3 element vector Dot Product.
    \param v1 [in] Left side vector.
    \param v2 [in] Right side vector.
    \return Dot Product.
*/
inline float Dot ( const float v1[], const float v2[] )
{
    return ( float ) ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) + ( v1[2] * v2[2] );
}

/*! \brief 4 element vector Dot Product.
    \param v1 [in] Left side vector.
    \param v2 [in] Right side vector.
    \return Dot Product.
*/
inline float Dot4 ( const float v1[], const float v2[] )
{
    return ( float ) ( v1[0] * v2[0] ) + ( v1[1] * v2[1] ) + ( v1[2] * v2[2] ) + ( v1[3] * v2[3] );
}

/*! \brief Normalize vector.
    \param v [in/out] Vector to normalize.
    \return Pointer to normalized vector, same as v.
*/
inline float* Normalize ( float* v )
{
    float length = Length ( v );
    // do nothing if length = 0
    if ( length )
    {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
    return v;
}

/*! \brief Normalize vector.
    \param v [in/out] Vector to normalize.
    \return Pointer to normalized vector, same as v.
*/
inline float* Normalize4 ( float* v )
{
    float length = Length4 ( v );
    // do nothing if length = 0
    if ( length )
    {
        float oneoverlength = 1.0f / length;
        v[0] *= oneoverlength;
        v[1] *= oneoverlength;
        v[2] *= oneoverlength;
        v[3] *= oneoverlength;
    }
    return v;
}

/*! \brief Normalizes a plane.
\param aPlane [in] Four element float array with the plane to normalize.
\param aOut [out] Four element float array to receive the result, may be the same as aPlane.
\return a pointer to the result, same as aOut.
*/
inline float* NormalizePlane ( float const * const aPlane, float* aOut )
{
    float length = Length ( aPlane );
    // do nothing if length = 0
    if ( length )
    {
        aOut[0] = aPlane[0] / length;
        aOut[1] = aPlane[1] / length;
        aOut[2] = aPlane[2] / length;
        aOut[3] = aPlane[3] / length;
    }
    return aOut;
}


/*! \brief adds two 4 element vectors.
*/
inline float* Add4 ( float* v1, float* v2, float* out )
{
    out[0] = v1[0] + v2[0];
    out[1] = v1[1] + v2[1];
    out[2] = v1[2] + v2[2];
    out[3] = v1[3] + v2[3];
    return out;
}

/*! \brief adds two 4 element vectors.
*/
inline float* Subtract4 ( float* v1, float* v2, float* out )
{
    out[0] = v1[0] - v2[0];
    out[1] = v1[1] - v2[1];
    out[2] = v1[2] - v2[2];
    out[3] = v1[3] - v2[3];
    return out;
}
/*! \brief Multiply 4 element vector by a scalar
*/
inline float* ScalarMultiply4 ( float* v, float s, float* out )
{
    out[0] = v[0] * s;
    out[1] = v[1] * s;
    out[2] = v[2] * s;
    out[3] = v[3] * s;
    return out;
}

/*! \brief Clips Velocity against impacting surface normal.

    Projects the velocity on the surface, efectivelly creating the effect of sliding.
    \param v [in/out] The velocity vector.
    \param normal [in] The impacting surface normal.
    \param overbounce [in] Clipping offset.
*/
inline void ClipVelocity ( float* v, float* normal, float overbounce )
{
    float   backoff;
    float   change;
    int     i;
    backoff =
        v[0] * normal[0] +
        v[1] * normal[1] +
        v[2] * normal[2];
    if ( backoff < 0 )
    {
        backoff *= overbounce;
    }
    else
    {
        backoff /= overbounce;
    }
    for ( i = 0 ; i < 3 ; i++ )
    {
        change = normal[i] * backoff;
        v[i] = v[i] - change;
    }
}
/*! \brief Returns the closest to paralell axis to the provided vector.

    Takes a vector and calculates the closest to paralell axis, and returns it as an index where:
            - 0 = X
            - 1 = Y
            - 2 = Z
    \param v [in] Vector to evaluate.
    \return The closest to paralell axis as a 0-2 index.
 */
inline int ClosestAxis ( float* v )
{
    float a[2];
    int axis = ( a[0] = sabs ( v[0] ) ) < ( a[1] = sabs ( v[1] ) ) ? 1 : 0;
    return a[axis] < sabs ( v[2] ) ? 2 : axis;
}
/*! \brief Interpolates between two vectors.

    Calculates the location between the two vectors where v1 is the origin and v2 is the destination,
    interpolation represents the fraction between the two that should be advanced, a value of zero returns v1
    while a value of one returns v2, anything over one will return a position past v2 in the direction v2-v1.
    The out parameter may be the same as either v1 or v2 in which case the values get overwritten.
    \param v1 [in] Origin vector.
    \param v2 [in] Destination vector.
    \param interpolation [in] Interpolation factor.
    \param out [out] Resulting position vector.
*/
inline void InterpolateVectors ( float* v1, float* v2, float interpolation, float* out )
{
    float localout[3];
    localout[0] = v1[0] + ( ( v2[0] - v1[0] ) * interpolation );
    localout[1] = v1[1] + ( ( v2[1] - v1[1] ) * interpolation );
    localout[2] = v1[2] + ( ( v2[2] - v1[2] ) * interpolation );
    out[0] = localout[0];
    out[1] = localout[1];
    out[2] = localout[2];
}
/*! \brief Returns the squared distance between point a and b.

    Used mostly to avoid a square root operation when the actual distance is not required.
    \param a [in] Vector for point a.
    \param b [in] Vector for point b.
    \return Square distance between a and b.
    \sa Distance
*/
inline float DistanceSquared ( const float a[], const float b[] )
{
    float v[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    return ( v[0] * v[0] ) + ( v[1] * v[1] ) + ( v[2] * v[2] );
}
/*! \brief Returns the distance between point a and b.

    When relative comparations are needed is faster to use the values returned by DistanceSquared than this function.
    \param a [in] Vector for point a.
    \param b [in] Vector for point b.
    \return Distance between a and b.
    \sa DistanceSquared
*/
inline float Distance ( const float a[], const float b[] )
{
    return sqrtf ( DistanceSquared ( a, b ) );
}
/*! \brief Multiplies a vector and a 4x4 matrix.

    The out parameter may be the same as v, in which case the values get overwritten.
    \param v [in] Vector.
    \param m [in] Matrix.
    \param out [out] Resulting vector.
    \todo Column mayor?
*/
inline float* MultVector4x4Matrix ( const float* v, const float* m, float* out )
{
    float vc[3] = {v[0], v[1], v[2]};
    out[0] = vc[0] * m[0] + vc[1] * m[4] + vc[2] * m[ 8] + m[12];
    out[1] = vc[0] * m[1] + vc[1] * m[5] + vc[2] * m[ 9] + m[13];
    out[2] = vc[0] * m[2] + vc[1] * m[6] + vc[2] * m[10] + m[14];
    return out;
}
/*! \brief Multiplies a vector and a Scalar.

    The out parameter may be the same as v, in which case the values get overwritten.
    \param v [in] Vector.
    \param s [in] Scalar.
    \param out [out] Resulting vector.
*/
inline float* MultVectorScalar ( float* v, float s, float* out )
{
    out[0] = v[0] * s;
    out[1] = v[1] * s;
    out[2] = v[2] * s;
    return out;
}
/*! \brief Multiplies a vector and the rotational 3x3 part of a 4x4 matrix.

    The out parameter may be the same as v, in which case the values get overwritten.
    \param v [in] Vector.
    \param m [in] Matrix.
    \param out [out] Resulting vector.
    \todo Column mayor?
*/
inline void MultVector3x3Matrix ( float* v, float* m, float* out )
{
    float vc[3] = {v[0], v[1], v[2]};
    out[0] = vc[0] * m[0] + vc[1] * m[4] + vc[2] * m[ 8];
    out[1] = vc[0] * m[1] + vc[1] * m[5] + vc[2] * m[ 9];
    out[2] = vc[0] * m[2] + vc[1] * m[6] + vc[2] * m[10];
}

/* \brief Returns the multiplicative inverse of a scale (transform) vector. */
inline float* GetScaleVectorInverse ( const float* v, float *out )
{
    out[0] = 1.0f / v[0];
    out[1] = 1.0f / v[1];
    out[2] = 1.0f / v[2];
    return out;
}
/* \brief Returns the additive inverse of a position (transform) vector. */
inline float* GetPositionVectorInverse ( const float* v, float *out )
{
    out[0] = -v[0];
    out[1] = -v[1];
    out[2] = -v[2];
    return out;
}

// @}
/*! \name Matrix Functions */
// @{
/*! \brief Set a matrix to the Identity.
    \param M [out] Matrix to set to Identity.
    \return Pointer to matrix, same as M.
*/
inline float* SetIdentityMatrix4x4 ( float* M )
{
    M[0] = M[5] = M[10] = M[15] = 1.0f;
    M[1] = M[2] = M[3] = M[4] = M[6] = M[7] = M[8] = M[9] = M[11] = M[12] = M[13] = M[14] = 0.0f;
    return M;
}

/*! \brief Extracts a 3x3 matrix from a 4x4 matrix.
\param m [in] 4x4 Matrix.
\param out [out] 3x3 Matrix.
*/
inline float* Extract3x3Matrix ( const float* m, float* out )
{
    out[0] = m[0];
    out[1] = m[1];
    out[2] = m[2];
    out[3] = m[4];
    out[4] = m[5];
    out[5] = m[6];
    out[6] = m[8];
    out[7] = m[9];
    out[8] = m[10];
    return out;
}

/*! \brief Extracts a 3x3 matrix from a 4x4 matrix into a 4x4 matrix.
\param m [in] 4x4 Matrix.
\param out [out] 4x4 Matrix.
*/
inline float* Extract3x3Into4x4 ( const float* m, float* out )
{
    out[0] = m[0];
    out[1] = m[1];
    out[2] = m[2];
    out[3] = 0;
    out[4] = m[4];
    out[5] = m[5];
    out[6] = m[6];
    out[7] = 0;
    out[8] = m[8];
    out[9] = m[9];
    out[10] = m[10];
    out[11] = 0;
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;
    return out;
}

/*! \brief converts a 3x3 matrix to a 4x3 matrix.
\param m [in] 3x3 Matrix.
\param out [out] 4x4 Matrix.
*/
inline float* Convert3x3To4x3 ( const float* m, float* out )
{
    /*  Setting the values backward allow
    for the input matrix to be output matrix
    (in place conversion)*/
    out[11] = 0;
    out[10] = m[8];
    out[9] = m[7];
    out[8] = m[6];
    out[7] = 0;
    out[6] = m[5];
    out[5] = m[4];
    out[4] = m[3];
    out[3] = 0;
    out[2] = m[2];
    out[1] = m[1];
    out[0] = m[0];
    return out;
}

/*! \brief converts a 3x3 matrix to a 4x4 matrix.
\param m [in] 3x3 Matrix.
\param out [out] 4x4 Matrix.
*/
inline float* Convert3x3To4x4 ( const float* m, float* out )
{
    out[15] = 1;
    out[14] = 0;
    out[13] = 0;
    out[12] = 0;
    return Convert3x3To4x3 ( m, out );
}

/*! \brief Transposes a 3x3 Matrix.
\param src [in] 3x3 matrix to transpose.
\param out [out] 3x3 transposed matrix.
*/
inline float* Transpose3x3Matrix ( const float *src, float *dst )
{
    float tmp[9];
    for ( int i = 0; i < 3; i++ )
    {
        tmp[i] = src[i * 3];
        tmp[i + 3] = src[i * 3 + 1];
        tmp[i + 6] = src[i * 3 + 2];
    }
    memcpy ( dst, tmp, sizeof ( float ) * 9 );
    return dst;
}


inline float DeterminantMatrix3 ( const float *src )
{
    float A = src[4] * src[8] - src[5] * src[7];
    float B = - ( src[3] * src[8] - src[5] * src[6] );
    float C = ( src[3] * src[7] - src[4] * src[6] );
    return src[0] * A + src[1] * B + src[2] * C;
}

/*! \brief Inverts a 3x3 Matrix.
\param src [in] 3x3 matrix to invert.
\param out [out] 3x3 inverted matrix.
*/
inline float* Invert3x3Matrix ( const float *src, float *dst )
{
    float A = src[4] * src[8] - src[5] * src[7];
    float B = - ( src[3] * src[8] - src[5] * src[6] );
    float C = ( src[3] * src[7] - src[4] * src[6] );
    float determinant = src[0] * A + src[1] * B + src[2] * C;
    if ( determinant != 0.0f )
    {
        float D = - ( src[1] * src[8] - src[2] * src[7] );
        float E = src[0] * src[8] - src[2] * src[6];
        float F = - ( src[0] * src[7] - src[1] * src[6] );
        float G = src[1] * src[5] - src[2] * src[4];
        float H = - ( src[0] * src[5] - src[2] * src[3] );
        float I = src[0] * src[4] - src[1] * src[3];
        dst[0] = A / determinant;
        dst[3] = B / determinant;
        dst[6] = C / determinant;
        dst[1] = D / determinant;
        dst[4] = E / determinant;
        dst[7] = F / determinant;
        dst[2] = G / determinant;
        dst[5] = H / determinant;
        dst[8] = I / determinant;
    }
    return dst;
}


/*! \brief Inverts a RT (rotation and translation) 4x4 matrix.


    This function is faster than InvertMatrix but
    the rotation part of the matrix must be Orthogonal,
    otherwise the result is not what is expected.

    The operation performed to the matrix is to transpose the rotation matrix
    and recalculate the translation vector so it goes from:

    \f[
    \left( \begin{array}{cccc}
         X.x & Y.x & Z.x & P.x \\
         X.y & Y.y & Z.y & P.y \\
         X.z & Y.z & Z.z & P.z \\
         0   & 0   & 0   & 1   \end{array} \right)
    \f]

    to:

    \f[
    \left( \begin{array}{cccc}
         X.x & X.y & X.z & -dot(P, X)
         Y.x & Y.y & Y.z & -dot(P, Y)
         Z.x & Z.y & Z.z & -dot(P, Z)
         0   & 0   & 0   & 1   \end{array} \right)
    \f]

    \param src [in] A pointer or reference to a 16 element float array containing the 4x4 matrix to invert.
    \param dst [out] A pointer or reference to a 16 element float array in which the inverted matrix will be stored, can be the same as mat.
    \return A pointer to the inverted matrix, same as dest.
    \sa InvertMatrix
*/
inline float* InvertOrthogonalMatrix ( float *src, float *dst )
{
    /*
        From:

        [ X.x Y.x Z.x P.x ]
        [ X.y Y.y Z.y P.y ]
        [ X.z Y.z Z.z P.z ]
        [ 0   0   0   1   ]

        To:

        [ X.x X.y X.z -dot(P, X) ]
        [ Y.x Y.y Y.z -dot(P, Y) ]
        [ Z.x Z.y Z.z -dot(P, Z) ]
        [ 0   0   0    1         ]
    */

    float tmp[16] =
    {
        src[ 0], src[ 4], src[ 8], 0.0f,
        src[ 1], src[ 5], src[ 9], 0.0f,
        src[ 2], src[ 6], src[10], 0.0f,
        - ( src[12]*src[ 0] + src[13]*src[ 1] + src[14]*src[ 2] ),
        - ( src[12]*src[ 4] + src[13]*src[ 5] + src[14]*src[ 6] ),
        - ( src[12]*src[ 8] + src[13]*src[ 9] + src[14]*src[ 10] ),
        1.0f
    };

    memcpy ( dst, tmp, sizeof ( float ) * 16 );

    return dst;
}

/*! \brief Invert a 4x4 matrix

Inverts a 4x4 matrix, it assumes the matrix is represented by a linear 16 element float array,
dest must not be NULL and must contain enought space for 16 floats, it may be the same as math, in which case the original matrix is overwritten.
\param mat [in] A pointer or reference to a 16 element float array containing the 4x4 matrix to invert.
\param dest [out] A pointer or reference to a 16 element float array in which the inverted matrix will be stored, can be the same as mat.
\return A pointer to the inverted matrix, same as dest.

For matrices containing rotational and translational tranforms only, using InvertOrthogonalMatrix is much faster.

\sa InvertOrthogonalMatrix
 */
inline float* InvertMatrix ( float *mat, float *dest )
{
    float dst[16];
    memcpy ( dst, dest, sizeof ( float ) * 16 );
    float tmp[12]; /* temp array for pairs */
    float src[16]; /* array of transpose source matrix */
    float det; /* determinant */
    /* transpose matrix */
    for ( int i = 0; i < 4; i++ )
    {
        src[i] = mat[i * 4];
        src[i + 4] = mat[i * 4 + 1];
        src[i + 8] = mat[i * 4 + 2];
        src[i + 12] = mat[i * 4 + 3];
    }
    /* calculate pairs for first 8 elements (cofactors) */
    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];
    /* calculate first 8 elements (cofactors) */
    dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
    dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
    dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
    dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
    dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
    dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
    dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
    dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
    dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
    dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
    dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
    dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
    /* calculate pairs for second 8 elements (cofactors) */
    tmp[0] = src[2] * src[7];
    tmp[1] = src[3] * src[6];
    tmp[2] = src[1] * src[7];
    tmp[3] = src[3] * src[5];
    tmp[4] = src[1] * src[6];
    tmp[5] = src[2] * src[5];
    tmp[6] = src[0] * src[7];
    tmp[7] = src[3] * src[4];
    tmp[8] = src[0] * src[6];
    tmp[9] = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];
    /* calculate second 8 elements (cofactors) */
    dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
    dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
    dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
    dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
    dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
    dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
    dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
    dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
    dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
    dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
    dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
    dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
    dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
    dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
    dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
    dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];
    /* calculate determinant */
    det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
    /* calculate matrix inverse */
    det = 1 / det;
    for ( float & j : dst )
    {
        j *= det;
    }
    memcpy ( dest, dst, sizeof ( float ) * 16 );
    return dest;
}
/*! \brief Multiplies two 4x4 matrices.

Multiplies two 4x4 matrices, returning a pointer to the resulting matrix,
each of which should be an array of 16 float elements,
dst may be the same as either of the two matrices in which case the matrix is overwritten,
it must not be NULL, and should point to an array of at least 16 float elements.
\param A [in] Pointer or reference to left side matrix
\param B [in] Pointer or reference to right side matrix
\param dst [out] Pointer or reference to space in memory in which to store the result, may be the same as either m1 or m2.
\return A pointer to the resulting matrix, same as dst.
\note Multiplication is done in column mayor order.
*/
inline float* Multiply4x4Matrix ( const float* A, const float* B, float* out )
{
    float result[16];
    float mx1[16];
    float mx2[16];
    memcpy ( mx1, A, sizeof ( float ) * 16 );
    memcpy ( mx2, B, sizeof ( float ) * 16 );
#if 0
    // This code is unsafe, kept here only as reference
    if ( bHasSSE )
    {
#ifdef __GNUC__
        __asm__
        (
            "movss  %0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups %1,%%xmm0\n\t"
            "mulps  %%xmm4,%%xmm0\n\t"
            // ------+
            "movss  4%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 16%1,%%xmm1\n\t"
            "mulps  %%xmm4,%%xmm1\n\t"
            // ------+
            "movss  8%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 32%1,%%xmm2\n\t"
            "mulps  %%xmm4,%%xmm2\n\t"
            // ------+
            "movss  12%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 48%1,%%xmm3\n\t"
            "mulps  %%xmm4,%%xmm3\n\t"
            //-------+
            "addps %%xmm1,%%xmm0\n\t"
            "addps %%xmm2,%%xmm0\n\t"
            "addps %%xmm3,%%xmm0\n\t"
            "movups %%xmm0,%2\n\t"
            // done a side
            "movss  16%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups %1,%%xmm0\n\t"
            "mulps  %%xmm4,%%xmm0\n\t"
            // ------
            "movss 20%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 16%1,%%xmm1\n\t"
            "mulps  %%xmm4,%%xmm1\n\t"
            // ------
            "movss 24%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 32%1,%%xmm2\n\t"
            "mulps  %%xmm4,%%xmm2\n\t"
            // ------
            "movss 28%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 48%1,%%xmm3\n\t"
            "mulps  %%xmm4,%%xmm3\n\t"
            //-------
            "addps %%xmm1,%%xmm0\n\t"
            "addps %%xmm2,%%xmm0\n\t"
            "addps %%xmm3,%%xmm0\n\t"
            "movups %%xmm0,16%2\n\t"
            // done a side
            "movss 32%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups %1,%%xmm0\n\t"
            "mulps  %%xmm4,%%xmm0\n\t"
            // ------
            "movss 36%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 16%1,%%xmm1\n\t"
            "mulps  %%xmm4,%%xmm1\n\t"
            // ------
            "movss 40%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 32%1,%%xmm2\n\t"
            "mulps  %%xmm4,%%xmm2\n\t"
            // ------
            "movss 44%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 48%1,%%xmm3\n\t"
            "mulps  %%xmm4,%%xmm3\n\t"
            //-------
            "addps %%xmm1,%%xmm0\n\t"
            "addps %%xmm2,%%xmm0\n\t"
            "addps %%xmm3,%%xmm0\n\t"
            "movups %%xmm0,32%2\n\t"
            // done a side
            "movss 48%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups %1,%%xmm0\n\t"
            "mulps  %%xmm4,%%xmm0\n\t"
            // ------
            "movss 52%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 16%1,%%xmm1\n\t"
            "mulps  %%xmm4,%%xmm1\n\t"
            // ------
            "movss 56%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 32%1,%%xmm2\n\t"
            "mulps  %%xmm4,%%xmm2\n\t"
            // ------
            "movss 60%0,%%xmm4\n\t"
            "shufps $0x0,%%xmm4,%%xmm4\n\t"
            "movups 48%1,%%xmm3\n\t"
            "mulps  %%xmm4,%%xmm3\n\t"
            //-------
            "addps %%xmm1,%%xmm0\n\t"
            "addps %%xmm2,%%xmm0\n\t"
            "addps %%xmm3,%%xmm0\n\t"
            "movups %%xmm0,48%2\n\t"
            :
            : "m" ( mx[0] ), "m" ( m2[0] ), "m" ( result[0] )
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4"
        );
        //   __asm__ __volatile__ ("rdtsc" : "=A" (nElapsedTime));
        //   fprintf(stdout,"ElapsedTime: %d\n",nElapsedTime-nCurrentTime);

#elif _MSC_VER
        __asm
        {
            // Small preamble, MSVC Inline ASM doesn't
            // automatically reference object variables
            mov eax, this // avoid C4537 Warning
            lea eax, [eax]this.mx
            mov ecx, MX // avoid C4537 Warning
            lea ecx, [ecx]m2
            lea edx, result // local variables need not be moved, if they are, app crashes
            movss  xmm4, [eax]
            shufps xmm4, xmm4, 0
            movups xmm0, [ecx]
            mulps  xmm0, xmm4
            // ------
            movss  xmm4, [eax+ 4]
            shufps xmm4, xmm4, 0
            movups xmm1, [ecx+16]
            mulps  xmm1, xmm4
            // ------
            movss  xmm4, [eax+ 8]
            shufps xmm4, xmm4, 0
            movups xmm2, [ecx+32]
            mulps  xmm2, xmm4
            // ------
            movss  xmm4, [eax+ 12]
            shufps xmm4, xmm4, 0
            movups xmm3, [ecx+48]
            mulps  xmm3, xmm4
            //-------
            addps xmm0, xmm1
            addps xmm0, xmm2
            addps xmm0, xmm3
            movups [edx], xmm0
            // done a side
            movss  xmm4, [eax+ 16]
            shufps xmm4, xmm4, 0
            movups xmm0, [ecx]
            mulps  xmm0, xmm4
            // ------
            movss  xmm4, [eax+ 20]
            shufps xmm4, xmm4, 0
            movups xmm1, [ecx+16]
            mulps  xmm1, xmm4
            // ------
            movss  xmm4, [eax+ 24]
            shufps xmm4, xmm4, 0
            movups xmm2, [ecx+32]
            mulps  xmm2, xmm4
            // ------
            movss  xmm4, [eax+ 28]
            shufps xmm4, xmm4, 0
            movups xmm3, [ecx+48]
            mulps  xmm3, xmm4
            //-------
            addps xmm0, xmm1
            addps xmm0, xmm2
            addps xmm0, xmm3
            movups [edx+16], xmm0
            // done a side
            movss  xmm4, [eax+ 32]
            shufps xmm4, xmm4, 0
            movups xmm0, [ecx]
            mulps  xmm0, xmm4
            // ------
            movss  xmm4, [eax+ 36]
            shufps xmm4, xmm4, 0
            movups xmm1, [ecx+16]
            mulps  xmm1, xmm4
            // ------
            movss  xmm4, [eax+ 40]
            shufps xmm4, xmm4, 0
            movups xmm2, [ecx+32]
            mulps  xmm2, xmm4
            // ------
            movss  xmm4, [eax+ 44]
            shufps xmm4, xmm4, 0
            movups xmm3, [ecx+48]
            mulps  xmm3, xmm4
            //-------
            addps xmm0, xmm1
            addps xmm0, xmm2
            addps xmm0, xmm3
            movups [edx+32], xmm0
            // done a side
            movss  xmm4, [eax+ 48]
            shufps xmm4, xmm4, 0
            movups xmm0, [ecx]
            mulps  xmm0, xmm4
            // ------
            movss  xmm4, [eax+ 52]
            shufps xmm4, xmm4, 0
            movups xmm1, [ecx+16]
            mulps  xmm1, xmm4
            // ------
            movss  xmm4, [eax+ 56]
            shufps xmm4, xmm4, 0
            movups xmm2, [ecx+32]
            mulps  xmm2, xmm4
            // ------
            movss  xmm4, [eax+ 60]
            shufps xmm4, xmm4, 0
            movups xmm3, [ecx+48]
            mulps  xmm3, xmm4
            //-------
            addps xmm0, xmm1
            addps xmm0, xmm2
            addps xmm0, xmm3
            movups [edx+48], xmm0
            // done a side
        }
#endif
    }
    else
    {
#endif
#if 0
        // Row mayor (DX way)
        result[ 0] = mx1[ 0] * mx2[ 0] + mx1[ 1] * mx2[ 4] + mx1[ 2] * mx2[ 8] + mx1[ 3] * mx2[12];
        result[ 1] = mx1[ 0] * mx2[ 1] + mx1[ 1] * mx2[ 5] + mx1[ 2] * mx2[ 9] + mx1[ 3] * mx2[13];
        result[ 2] = mx1[ 0] * mx2[ 2] + mx1[ 1] * mx2[ 6] + mx1[ 2] * mx2[10] + mx1[ 3] * mx2[14];
        result[ 3] = mx1[ 0] * mx2[ 3] + mx1[ 1] * mx2[ 7] + mx1[ 2] * mx2[11] + mx1[ 3] * mx2[15];

        result[ 4] = mx1[ 4] * mx2[ 0] + mx1[ 5] * mx2[ 4] + mx1[ 6] * mx2[ 8] + mx1[ 7] * mx2[12];
        result[ 5] = mx1[ 4] * mx2[ 1] + mx1[ 5] * mx2[ 5] + mx1[ 6] * mx2[ 9] + mx1[ 7] * mx2[13];
        result[ 6] = mx1[ 4] * mx2[ 2] + mx1[ 5] * mx2[ 6] + mx1[ 6] * mx2[10] + mx1[ 7] * mx2[14];
        result[ 7] = mx1[ 4] * mx2[ 3] + mx1[ 5] * mx2[ 7] + mx1[ 6] * mx2[11] + mx1[ 7] * mx2[15];

        result[ 8] = mx1[ 8] * mx2[ 0] + mx1[ 9] * mx2[ 4] + mx1[10] * mx2[ 8] + mx1[11] * mx2[12];
        result[ 9] = mx1[ 8] * mx2[ 1] + mx1[ 9] * mx2[ 5] + mx1[10] * mx2[ 9] + mx1[11] * mx2[13];
        result[10] = mx1[ 8] * mx2[ 2] + mx1[ 9] * mx2[ 6] + mx1[10] * mx2[10] + mx1[11] * mx2[14];
        result[11] = mx1[ 8] * mx2[ 3] + mx1[ 9] * mx2[ 7] + mx1[10] * mx2[11] + mx1[11] * mx2[15];

        result[12] = mx1[12] * mx2[ 0] + mx1[13] * mx2[ 4] + mx1[14] * mx2[ 8] + mx1[15] * mx2[12];
        result[13] = mx1[12] * mx2[ 1] + mx1[13] * mx2[ 5] + mx1[14] * mx2[ 9] + mx1[15] * mx2[13];
        result[14] = mx1[12] * mx2[ 2] + mx1[13] * mx2[ 6] + mx1[14] * mx2[10] + mx1[15] * mx2[14];
        result[15] = mx1[12] * mx2[ 3] + mx1[13] * mx2[ 7] + mx1[14] * mx2[11] + mx1[15] * mx2[15];
#else
        // Column mayor (OpenGL way)
        result[ 0] = mx1[ 0] * mx2[ 0] + mx1[ 4] * mx2[ 1] + mx1[ 8] * mx2[ 2] + mx1[12] * mx2[ 3];
        result[ 1] = mx1[ 1] * mx2[ 0] + mx1[ 5] * mx2[ 1] + mx1[ 9] * mx2[ 2] + mx1[13] * mx2[ 3];
        result[ 2] = mx1[ 2] * mx2[ 0] + mx1[ 6] * mx2[ 1] + mx1[10] * mx2[ 2] + mx1[14] * mx2[ 3];
        result[ 3] = mx1[ 3] * mx2[ 0] + mx1[ 7] * mx2[ 1] + mx1[11] * mx2[ 2] + mx1[15] * mx2[ 3];

        result[ 4] = mx1[ 0] * mx2[ 4] + mx1[ 4] * mx2[ 5] + mx1[ 8] * mx2[ 6] + mx1[12] * mx2[ 7];
        result[ 5] = mx1[ 1] * mx2[ 4] + mx1[ 5] * mx2[ 5] + mx1[ 9] * mx2[ 6] + mx1[13] * mx2[ 7];
        result[ 6] = mx1[ 2] * mx2[ 4] + mx1[ 6] * mx2[ 5] + mx1[10] * mx2[ 6] + mx1[14] * mx2[ 7];
        result[ 7] = mx1[ 3] * mx2[ 4] + mx1[ 7] * mx2[ 5] + mx1[11] * mx2[ 6] + mx1[15] * mx2[ 7];

        result[ 8] = mx1[ 0] * mx2[ 8] + mx1[ 4] * mx2[ 9] + mx1[ 8] * mx2[10] + mx1[12] * mx2[11];
        result[ 9] = mx1[ 1] * mx2[ 8] + mx1[ 5] * mx2[ 9] + mx1[ 9] * mx2[10] + mx1[13] * mx2[11];
        result[10] = mx1[ 2] * mx2[ 8] + mx1[ 6] * mx2[ 9] + mx1[10] * mx2[10] + mx1[14] * mx2[11];
        result[11] = mx1[ 3] * mx2[ 8] + mx1[ 7] * mx2[ 9] + mx1[11] * mx2[10] + mx1[15] * mx2[11];

        result[12] = mx1[ 0] * mx2[12] + mx1[ 4] * mx2[13] + mx1[ 8] * mx2[14] + mx1[12] * mx2[15];
        result[13] = mx1[ 1] * mx2[12] + mx1[ 5] * mx2[13] + mx1[ 9] * mx2[14] + mx1[13] * mx2[15];
        result[14] = mx1[ 2] * mx2[12] + mx1[ 6] * mx2[13] + mx1[10] * mx2[14] + mx1[14] * mx2[15];
        result[15] = mx1[ 3] * mx2[12] + mx1[ 7] * mx2[13] + mx1[11] * mx2[14] + mx1[15] * mx2[15];
#endif
#if 0
    }
#endif
    memcpy ( out, result, sizeof ( float ) * 16 );
    return out;
}
/*! \brief Multiplies only the 3x3 part of two 4x4 matrices.

Multiplies two 4x4 matrices, returning a pointer to the resulting matrix,
each of which should be an array of 16 float elements,
dst may be the same as either of the two matrices in which case the matrix is overwritten,
it must not be NULL, and should point to an array of at least 16 float elements.
This function only takes into consideration the \e rotational part of the matrix,
hence 3x3, the operation it performs is as follows:

    \f[
    \left( \begin{array}{ccc}
    A[0] & A[4] & A[8]  \\
    A[1] & A[5] & A[9]  \\
    A[2] & A[6] & A[10] \end{array} \right)
    \times
    \left( \begin{array}{ccc}
    B[0] & B[4] & B[8]  \\
    B[1] & B[5] & B[9]  \\
    B[2] & B[6] & B[10] \end{array} \right)
    \f]

\param A [in] Pointer or reference to left side matrix
\param B [in] Pointer or reference to right side matrix
\param dst [out] Pointer or reference to space in memory in which to store the result, may be the same as either m1 or m2.
\return A pointer to the resulting matrix, same as dst.
\note Multiplication is done in column mayor order.
\note The values for dst[3],dst[7],dst[11],dst[12],dst[13],dst[14] and dst[15] are left untouched, they must be initialized before the matrix is used.
\todo What's faster? copying both matrices and storing the result in dst or storing results in a separate matrix and then copy?
*/
inline float* Multiply3x3Matrix ( float* A, float* B, float* dst )
{
    float mx1[16];
    float mx2[16];
    memcpy ( mx1, A, sizeof ( float ) * 16 );
    memcpy ( mx2, B, sizeof ( float ) * 16 );
    // Column mayor (OpenGL way)
    dst[ 0] = mx1[ 0] * mx2[ 0] + mx1[ 4] * mx2[ 1] + mx1[ 8] * mx2[ 2];
    dst[ 1] = mx1[ 1] * mx2[ 0] + mx1[ 5] * mx2[ 1] + mx1[ 9] * mx2[ 2];
    dst[ 2] = mx1[ 2] * mx2[ 0] + mx1[ 6] * mx2[ 1] + mx1[10] * mx2[ 2];

    dst[ 4] = mx1[ 0] * mx2[ 4] + mx1[ 4] * mx2[ 5] + mx1[ 8] * mx2[ 6];
    dst[ 5] = mx1[ 1] * mx2[ 4] + mx1[ 5] * mx2[ 5] + mx1[ 9] * mx2[ 6];
    dst[ 6] = mx1[ 2] * mx2[ 4] + mx1[ 6] * mx2[ 5] + mx1[10] * mx2[ 6];

    dst[ 8] = mx1[ 0] * mx2[ 8] + mx1[ 4] * mx2[ 9] + mx1[ 8] * mx2[10];
    dst[ 9] = mx1[ 1] * mx2[ 8] + mx1[ 5] * mx2[ 9] + mx1[ 9] * mx2[10];
    dst[10] = mx1[ 2] * mx2[ 8] + mx1[ 6] * mx2[ 9] + mx1[10] * mx2[10];
    return dst;
}

/*! \brief Linearly interpolate two matrices.

    It is not safe to pass m1 or m2 as o, unlike some other functions.
    If the interpolation factor is less than 0.0 or more than 1.0
    the resulting matrix is clipped to be the same as either m1 or m2 depending on the case.
    \param m1 [in] Origin matrix.
    \param m2 [in] Destination matrix.
    \param o [out] Resulting matrix.
    \param i [in] Interpolation factor.
*/
inline void InterpolateMatrices ( float* m1, float* m2, float* o, float i )
{
    if ( i < 0.0f )
    {
        memcpy ( o, m1, sizeof ( float ) * 16 );
        return;
    }
    else if ( i > 1.0f )
    {
        memcpy ( o, m2, sizeof ( float ) * 16 );
        return;
    }
    o[ 0] = m1[ 0] + ( ( m2[ 0] - m1[ 0] ) * i );
    o[ 1] = m1[ 1] + ( ( m2[ 1] - m1[ 1] ) * i );
    o[ 2] = m1[ 2] + ( ( m2[ 2] - m1[ 2] ) * i );
    o[ 3] = m1[ 3] + ( ( m2[ 3] - m1[ 3] ) * i );
    o[ 4] = m1[ 4] + ( ( m2[ 4] - m1[ 4] ) * i );
    o[ 5] = m1[ 5] + ( ( m2[ 5] - m1[ 5] ) * i );
    o[ 6] = m1[ 6] + ( ( m2[ 6] - m1[ 6] ) * i );
    o[ 7] = m1[ 7] + ( ( m2[ 7] - m1[ 7] ) * i );
    o[ 8] = m1[ 8] + ( ( m2[ 8] - m1[ 8] ) * i );
    o[ 9] = m1[ 9] + ( ( m2[ 9] - m1[ 9] ) * i );
    o[10] = m1[10] + ( ( m2[10] - m1[10] ) * i );
    o[11] = m1[11] + ( ( m2[11] - m1[11] ) * i );
    o[12] = m1[12] + ( ( m2[12] - m1[12] ) * i );
    o[13] = m1[13] + ( ( m2[13] - m1[13] ) * i );
    o[14] = m1[14] + ( ( m2[14] - m1[14] ) * i );
    o[15] = m1[15] + ( ( m2[15] - m1[15] ) * i );
}
/*! \brief Constructs the rotation matrix defined by the axis-angle provided.

    The Matrix returned is a 4x4 matrix constructed using the same formula glRotate* uses.

    \param angle [in] Angle in degrees of rotation.
    \param R [out] 16 element array to store the matrix.
    \param x [in] X element of axis of rotation.
    \param y [in] Y element of axis of rotation.
    \param z [in] Z element of axis of rotation.
    \return pointer to the returned matrix, same as R.
    \note This function is not meant to be used outside this file, it is meant to reduce redundancy on the RotateMatrix functions.
    \private
*/
inline float* GetRotationMatrix ( float* R, float angle, float x, float y, float z )
{
    float radians = float ( ( angle / 180.0f ) * PI );
    float c = cosf ( radians );
    float s = sinf ( radians );
    R[ 0] = x * x * ( 1 - c ) + c;
    R[ 1] = x * y * ( 1 - c ) - z * s;
    R[ 2] = x * z * ( 1 - c ) + y * s;
    R[ 3] = 0;
    R[ 4] = y * x * ( 1 - c ) + z * s;
    R[ 5] = y * y * ( 1 - c ) + c;
    R[ 6] = y * z * ( 1 - c ) - x * s;
    R[ 7] = 0;
    R[ 8] = x * z * ( 1 - c ) - y * s;
    R[ 9] = y * z * ( 1 - c ) + x * s;
    R[10] = z * z * ( 1 - c ) + c;
    R[11] = 0;
    R[12] = 0;
    R[13] = 0;
    R[14] = 0;
    R[15] = 1;
    return R;
}
/*! \brief Rotates a matrix using axis angle notation.

This function rotates a matrix using axis angle notaton where x,y,z represent the axis
and angle is the angle in degrees. Source and destination matrix may point to the same address, in which case the source matrix will be overwriten with the result.
\param src [in] Pointer or reference to source matrix.
\param dst [out] Pointer or reference to destination matrix, may be same as src.
\param angle [in] Angle in degrees of rotation.
\param x [in] X element of axis of rotation.
\param y [in] Y element of axis of rotation.
\param z [in] Z element of axis of rotation.
*/
inline float* RotateMatrix ( float* src, float* dst, float angle, float x, float y, float z )
{
    /*
    This replicates how glRotatef works.
    */
    float r[16];
    GetRotationMatrix ( r, angle, x, y, z );
    return Multiply4x4Matrix ( src, r, dst );
}
/*! \brief Rotates a matrix in object space using axis angle notation.

This function rotates a matrix in object space using axis angle notaton where x,y,z represent the axis
and angle is the angle in degrees. Source and destination matrix may point to the same address,
in which case the source matrix will be overwriten with the result.

\param src [in] Pointer or reference to source matrix.
\param dst [out] Pointer or reference to destination matrix, may be same as src.
\param angle [in] Angle in degrees of rotation.
\param x [in] X element of axis of rotation.
\param y [in] Y element of axis of rotation.
\param z [in] Z element of axis of rotation.
\todo Eliminate dst?
*/
inline float* RotateMatrixObjectSpace ( float* src, float* dst, float angle, float x, float y, float z )
{
    float v[3] = {x, y, z};
    MultVector3x3Matrix ( v, src, v );
    if ( dst != src )
    {
        // safeguards
        dst[ 3] = 0;
        dst[ 7] = 0;
        dst[11] = 0;
        dst[12] = src[12];
        dst[13] = src[13];
        dst[14] = src[14];
        dst[15] = 1;
    }
    float r[16];
    GetRotationMatrix ( r, angle, v[0], v[1], v[2] );
    return Multiply3x3Matrix ( src, r, dst );
}
/*! \brief Rotates a matrix in inertial space using axis angle notation.

This function rotates a matrix in inertial space using axis angle notaton where x,y,z represent the axis
and angle is the angle in degrees. Source and destination matrix may point to the same address,
in which case the source matrix will be overwriten with the result.

\param src [in] Pointer or reference to source matrix.
\param dst [out] Pointer or reference to destination matrix, may be same as src.
\param angle [in] Angle in degrees of rotation.
\param x [in] X element of axis of rotation.
\param y [in] Y element of axis of rotation.
\param z [in] Z element of axis of rotation.
\todo Eliminate dst?
*/
inline float* RotateMatrixInertialSpace ( float* src, float* dst, float angle, float x, float y, float z )
{
    if ( dst != src )
    {
        // safeguards
        dst[ 3] = 0;
        dst[ 7] = 0;
        dst[11] = 0;
        dst[12] = src[12];
        dst[13] = src[13];
        dst[14] = src[14];
        dst[15] = 1;
    }
    float r[16];
    GetRotationMatrix ( r, angle, x, y, z );
    return Multiply3x3Matrix ( src, r, dst );
}
/*! \brief Translates a 4x4 matrix using a vector relative from its current position using the object axis.

    This is a simplified matrix multiplication \f$A \times B\f$:

    \f[A =
    \left( \begin{array}{cccc}
    src[0] & src[4] & src[8] & src[12] \\
    src[1] & src[5] & src[9] & src[13] \\
    src[2] & src[6] & src[10] & src[14] \\
    0 & 0 & 0 & 1 \end{array} \right)
    \times
    B =
    \left( \begin{array}{cccc}
    1 & 0 & 0 & v[0] \\
    0 & 1 & 0 & v[1] \\
    0 & 0 & 1 & v[2] \\
    0 & 0 & 0 & 1 \end{array} \right)
    \f]

    \param v [in] Vector for matrix translation.
    \param src [in] Pointer or reference to source matrix.
    \param dst [out] Pointer or reference to destination matrix, may be same as src.
    \note This is the same as TranslateMatrixInertialSpace with matrix order swapped.
    \sa TranslateMatrixInertialSpace
*/
inline float* TranslateMatrixObjectSpace ( float* v, float* src, float* dst )
{
    float result[16];
#if 0
    result[ 0] = src[ 0];
    result[ 1] = src[ 1];
    result[ 2] = src[ 2];
    result[ 3] = src[ 3];

    result[ 4] = src[ 4];
    result[ 5] = src[ 5];
    result[ 6] = src[ 6];
    result[ 7] = src[ 7];

    result[ 8] = src[ 8];
    result[ 9] = src[ 9];
    result[10] = src[10];
    result[11] = src[11];

    result[12] = v[0] * src[ 0] + v[1] * src[ 4] + v[2] * src[ 8] + src[12];
    result[13] = v[0] * src[ 1] + v[1] * src[ 5] + v[2] * src[ 9] + src[13];
    result[14] = v[0] * src[ 2] + v[1] * src[ 6] + v[2] * src[10] + src[14];
    // in theory src[3], src[7] and src[11] will always be zero and src[15] one, safe to asume so?
    result[15] = v[0] * src[ 3] + v[1] * src[ 7] + v[2] * src[11] + src[15];
#else
    result[ 0] = src[ 0];
    result[ 1] = src[ 1];
    result[ 2] = src[ 2];
    result[ 3] = 0;

    result[ 4] = src[ 4];
    result[ 5] = src[ 5];
    result[ 6] = src[ 6];
    result[ 7] = 0;

    result[ 8] = src[ 8];
    result[ 9] = src[ 9];
    result[10] = src[10];
    result[11] = 0;

    result[12] = src[ 0] * v[0] + src[ 4] * v[1] + src[ 8] * v[2] + src[12];
    result[13] = src[ 1] * v[0] + src[ 5] * v[1] + src[ 9] * v[2] + src[13];
    result[14] = src[ 2] * v[0] + src[ 6] * v[1] + src[10] * v[2] + src[14];
    result[15] = 1;
#endif

    memcpy ( dst, result, sizeof ( float ) * 16 );
    return dst;
}
/*! \brief Translates a 4x4 matrix using a vector relative from its current position using the inertial axis.

    This is a simplified matrix multiplication \f$A \times B\f$:

    \f[A =
    \left( \begin{array}{cccc}
    1 & 0 & 0 & v[0] \\
    0 & 1 & 0 & v[1] \\
    0 & 0 & 1 & v[2] \\
    0 & 0 & 0 & 1 \end{array} \right)
    \times
    B =
    \left( \begin{array}{cccc}
    src[0] & src[4] & src[8] & src[12] \\
    src[1] & src[5] & src[9] & src[13] \\
    src[2] & src[6] & src[10] & src[14] \\
    0 & 0 & 0 & 1 \end{array} \right)
    \f]
    \param v [in] Vector for matrix translation.
    \param src [in] Pointer or reference to source matrix.
    \param dst [out] Pointer or reference to destination matrix, may be same as src.
    \note This is the same as TranslateMatrixObjectSpace with matrix order swapped.
    \sa TranslateMatrixObjectSpace
*/
inline float* TranslateMatrixInertialSpace ( float* v, float* src, float* dst )
{
    float result[16];
#if 0
    result[ 0] = src[ 0] + src[ 3] * v[ 0];
    result[ 1] = src[ 1] + src[ 3] * v[ 1];
    result[ 2] = src[ 2] + src[ 3] * v[ 2];
    result[ 3] = src[ 3];

    result[ 4] = src[ 4] + src[ 7] * v[ 0];
    result[ 5] = src[ 5] + src[ 7] * v[ 1];
    result[ 6] = src[ 6] + src[ 7] * v[ 2];
    result[ 7] = src[ 7];

    result[ 8] = src[ 8] + src[11] * v[ 0];
    result[ 9] = src[ 9] + src[11] * v[ 1];
    result[10] = src[10] + src[11] * v[ 2];
    result[11] = src[11];

    result[12] = src[12] + src[15] * v[ 0];
    result[13] = src[13] + src[15] * v[ 1];
    result[14] = src[14] + src[15] * v[ 2];
    result[15] = src[15];
#else
    result[ 0] = src[ 0];
    result[ 1] = src[ 1];
    result[ 2] = src[ 2];
    result[ 3] = 0;

    result[ 4] = src[ 4];
    result[ 5] = src[ 5];
    result[ 6] = src[ 6];
    result[ 7] = 0;

    result[ 8] = src[ 8];
    result[ 9] = src[ 9];
    result[10] = src[10];
    result[11] = 0;

    result[12] = src[12] + v[ 0];
    result[13] = src[13] + v[ 1];
    result[14] = src[14] + v[ 2];
    result[15] = 1;
#endif
    memcpy ( dst, result, sizeof ( float ) * 16 );
    return dst;
}

inline void QuatTo3x3Matrix ( const float* q, float* m );
inline float* GetQuaternionInverse ( const float* q, float* out );

/*!
\brief Constructs a transformation matrix from SRT vector.
\param srt [in] srt vector [s1,s2,s3,r1,r2,r3,r4,t1,t2,t3].
\param M [out] Transformation matrix.
\return Pointer to transformation matrix, same as M.
*/
inline float* GetMatrixFromSRT ( const float* srt, float* M )
{
    float R[9];
    QuatTo3x3Matrix ( srt + 3, R );
    // Simplified 3x3 scale matrix multiplication
    M[ 0] = R[ 0] * srt[0];
    M[ 1] = R[ 1] * srt[0];
    M[ 2] = R[ 2] * srt[0];
    M[ 3] = 0;

    M[ 4] = R[ 3] * srt[1];
    M[ 5] = R[ 4] * srt[1];
    M[ 6] = R[ 5] * srt[1];
    M[ 7] = 0;

    M[ 8] = R[ 6] * srt[2];
    M[ 9] = R[ 7] * srt[2];
    M[10] = R[ 8] * srt[2];
    M[11] = 0;
    // Simplified translation multiplication
    M[12] = srt[7];
    M[13] = srt[8];
    M[14] = srt[9];
    M[15] = 1;
    return M;
}

/*!
\brief Constructs a transformation matrix from SRT vectors.
\param s [in] Scale vector.
\param r [in] Rotation quaternion.
\param t [in] Translation vector.
\param M [out] Transformation matrix.
\return Pointer to transformation matrix, same as M.
*/

inline float* GetMatrixFromSRT ( float* s, float* r, float* t, float* M )
{
    float srt[10] = {s[0], s[1], s[2], r[0], r[1], r[2], r[3], t[0], t[1], t[2],};
    return GetMatrixFromSRT ( srt, M );
}

/*!
\brief Constructs an inverted transformation matrix from SRT vector.
\param srt [in] srt vector [s1,s2,s3,r1,r2,r3,r4,t1,t2,t3].
\param M [out] Transformation matrix.
\return Pointer to transformation matrix, same as M.
*/
inline float* GetInvertedMatrixFromSRT ( const float* srt, float* M )
{
#if 0
    // This works
    GetMatrixFromSRT ( srt, M );
    InvertOrthogonalMatrix ( M, M );
    return M;
#else
#if 0
    // this does work
    // This is a simple transpose approach
    float R[9];
    QuatTo3x3Matrix ( srt + 3, R );
    // Simplified 3x3 scale matrix multiplication
    M[0 ] = R[0] * srt[0];
    M[4 ] = R[1] * srt[0];
    M[8 ] = R[2] * srt[0];
    M[3] = 0;

    M[1 ] = R[3] * srt[1];
    M[5 ] = R[4] * srt[1];
    M[9 ] = R[5] * srt[1];
    M[7] = 0;

    M[2 ] = R[6] * srt[2];
    M[6 ] = R[7] * srt[2];
    M[10] = R[8] * srt[2];
    M[11] = 0;

    // Simplified translation multiplication
    M[12] = M[0] * srt[7] + M[1] * srt[8] + M[2] * srt[9] + M[3];
    M[13] = M[4] * srt[7] + M[5] * srt[8] + M[6] * srt[9] + M[7];
    M[14] = M[8] * srt[7] + M[9] * srt[8] + M[10] * srt[9] + M[11];
    M[15] = 1;
    return M;
#else
    // This inverts each vector
    float invstr[10] =
    {
        1.0f / srt[0], 1.0f / srt[1], 1.0f / srt[2],
        srt[3], -srt[4], -srt[5], -srt[6],
        -srt[7], -srt[8], -srt[9]
    };
    RotateVectorByQuat ( invstr + 3, invstr + 7, invstr + 7 );
    GetMatrixFromSRT ( invstr, M );
    return M;
#endif
#endif
}

/// Catmull-Rom spline interpolation
inline float* Spline ( const float* p0, const float* p1, const float* p2, const float* p3, double interpolation, float* out )
{
    double i2 = interpolation * interpolation;
    double i3 = i2 * interpolation;
    double t0[3] =
    {
        ( p2[0] - p0[0] ) / 2.0,
        ( p2[1] - p0[1] ) / 2.0,
        ( p2[2] - p0[2] ) / 2.0
    };

    double t1[3] =
    {
        ( p3[0] - p1[0] ) / 2.0,
        ( p3[1] - p1[1] ) / 2.0,
        ( p3[2] - p1[2] ) / 2.0
    };
    out[0] = static_cast<float> ( ( 2 * i3 - 3 * i2 + 1 ) * p1[0] + ( -2 * i3 + 3 * i2 ) * p2[0] + ( i3 - 2 * i2 + interpolation ) * t0[0] + ( i3 - i2 ) * t1[0] );
    out[1] = static_cast<float> ( ( 2 * i3 - 3 * i2 + 1 ) * p1[1] + ( -2 * i3 + 3 * i2 ) * p2[1] + ( i3 - 2 * i2 + interpolation ) * t0[1] + ( i3 - i2 ) * t1[1] );
    out[2] = static_cast<float> ( ( 2 * i3 - 3 * i2 + 1 ) * p1[2] + ( -2 * i3 + 3 * i2 ) * p2[2] + ( i3 - 2 * i2 + interpolation ) * t0[2] + ( i3 - i2 ) * t1[2] );
    return out;
}
// @}
/*! \name Quaternion Functions \note The quaternion scalar is the first element in any quaternion array */
// @{
//------------Quaternions------------------------------------------------
/*! \brief Inverts a unit length quaternion, same as quaternion conjugate */
inline float* GetQuaternionInverse ( const float* q, float* out )
{
    out[0] =  q[0];
    out[1] = -q[1];
    out[2] = -q[2];
    out[3] = -q[3];
    return out;
}
/*! \brief Convert axis angle rotation notation to a quaternion.

    Converts a 4 element float array representing axis angle notation
    into a 4 element float array representing a quaternion.
    The first 3 elements of axis represent the axis itself while the last one
    represents the angle in degres, the quaternion is given as W,X,Y,Z.
    \param angle [in] Angle of rotation in degrees.
    \param x [in] x axis.
    \param y [in] y axis.
    \param z [in] z axis.
    \param quat [out] The resulting quaternion.
*/
inline void AngleAxisToQuat ( float angle, float x, float y, float z, float* quat )
{
    float radians = float ( ( angle / 180.0f ) * PI );
    float result = ( float ) sin ( radians / 2.0f );
    quat[0] = ( float ) cos ( radians / 2.0f );
    quat[1] = float ( x * result );
    quat[2] = float ( y * result );
    quat[3] = float ( z * result );
}

/*! \brief Convert euler angle rotation notation to a quaternion.

    Converts a 3 element float array representing euler angle notation
    into a 4 element float array representing a quaternion.
    The 3 elements of the euler parameter represent, roll, pitch and yaw in that order,
    the quaternion is given as W,X,Y,Z.
    \param angle [in] Angle of rotation in degrees.
    \param euler [in] Three element array repserenting euler angles as roll, pitch, yaw.
    \param quat [out] The resulting quaternion.
    \note Untested function.
*/
inline void EulerToQuat ( float* euler, float* q )
{
    float roll = ( ( euler[0] / 180.0f ) * PI );
    float pitch = ( ( euler[1] / 180.0f ) * PI );
    float yaw = ( ( euler[2] / 180.0f ) * PI );
    q[0] = cos ( roll / 2 ) * cos ( pitch / 2 ) * cos ( yaw / 2 ) + sin ( roll / 2 ) * sin ( pitch / 2 ) * sin ( yaw / 2 );
    q[1] = sin ( roll / 2 ) * cos ( pitch / 2 ) * cos ( yaw / 2 ) - cos ( roll / 2 ) * sin ( pitch / 2 ) * sin ( yaw / 2 );
    q[2] = cos ( roll / 2 ) * sin ( pitch / 2 ) * cos ( yaw / 2 ) + sin ( roll / 2 ) * cos ( pitch / 2 ) * sin ( yaw / 2 );
    q[3] = cos ( roll / 2 ) * cos ( pitch / 2 ) * sin ( yaw / 2 ) - sin ( roll / 2 ) * sin ( pitch / 2 ) * cos ( yaw / 2 );
}

/*! \brief Multiply two quaternions.
    \param q1 [in] Left side quaternion.
    \param q2 [in] Right side quaternion.
    \param qo [out] Resulting quaternion.
*/
inline float* MultQuats ( const float* q1, const float* q2, float* out )
{
    // W,X,Y,Z
    float qc1[4] = {q1[0], q1[1], q1[2], q1[3]};
    float qc2[4] = {q2[0], q2[1], q2[2], q2[3]};
    out[0] = ( qc1[0] * qc2[0] - qc1[1] * qc2[1] - qc1[2] * qc2[2] - qc1[3] * qc2[3] );
    out[1] = ( qc1[0] * qc2[1] + qc1[1] * qc2[0] + qc1[2] * qc2[3] - qc1[3] * qc2[2] );
    out[2] = ( qc1[0] * qc2[2] - qc1[1] * qc2[3] + qc1[2] * qc2[0] + qc1[3] * qc2[1] );
    out[3] = ( qc1[0] * qc2[3] + qc1[1] * qc2[2] - qc1[2] * qc2[1] + qc1[3] * qc2[0] );
    return out;
}
/*! \brief Builds a 3x4 rotation matrix out of a quaternion inside a 4x4 matrix.

    Builds a 3x4 rotation matrix inside a 4x4 matrix, leaving the last row (translation) intact.
    The contents of the 3x4 matrix are overwritten, but the 4th row is untouched,
    so it must be set to something meaningful before use.
    \param q [in] Quaternion.
    \param m [in/out] Matrix.
*/
inline void QuatTo4x4Matrix ( float* q, float* m )
{
    // leaves the translation row intact
    m[ 0] = 1.0f - 2.0f * ( q[2] * q[2] + q[3] * q[3] );
    m[ 1] = 2.0f * ( q[1] * q[2] + q[3] * q[0] );
    m[ 2] = 2.0f * ( q[1] * q[3] - q[2] * q[0] );
    m[ 3] = 0.0f;
    // Second row
    m[ 4] = 2.0f * ( q[1] * q[2] - q[3] * q[0] );
    m[ 5] = 1.0f - 2.0f * ( q[1] * q[1] + q[3] * q[3] );
    m[ 6] = 2.0f * ( q[3] * q[2] + q[1] * q[0] );
    m[ 7] = 0.0f;
    // Third row
    m[ 8] = 2.0f * ( q[1] * q[3] + q[2] * q[0] );
    m[ 9] = 2.0f * ( q[2] * q[3] - q[1] * q[0] );
    m[10] = 1.0f - 2.0f * ( q[1] * q[1] + q[2] * q[2] );
    m[11] = 0.0f;
}
/*! \brief Builds a 3x3 rotation matrix out of a quaternion.
    \param q [in] Quaternion.
    \param m [in/out] Matrix.
*/
inline void QuatTo3x3Matrix ( const float* q, float* m )
{
#if 0
    // First row
    m[ 0] = 1.0f - 2.0f * ( q[2] * q[2] + q[3] * q[3] );
    m[ 1] = 2.0f * ( q[1] * q[2] + q[3] * q[0] );
    m[ 2] = 2.0f * ( q[1] * q[3] - q[2] * q[0] );
    // Second row
    m[ 3] = 2.0f * ( q[1] * q[2] - q[3] * q[0] );
    m[ 4] = 1.0f - 2.0f * ( q[1] * q[1] + q[3] * q[3] );
    m[ 5] = 2.0f * ( q[3] * q[2] + q[1] * q[0] );
    // Third row
    m[ 6] = 2.0f * ( q[1] * q[3] + q[2] * q[0] );
    m[ 7] = 2.0f * ( q[2] * q[3] - q[1] * q[0] );
    m[ 8] = 1.0f - 2.0f * ( q[1] * q[1] + q[2] * q[2] );
#else
    // Products
    float p1 = q[0] * q[1];
    float p2 = q[0] * q[2];
    float p3 = q[0] * q[3];

    float p4 = q[1] * q[1];
    float p5 = q[1] * q[2];
    float p6 = q[1] * q[3];

    float p7 = q[2] * q[2];
    float p8 = q[2] * q[3];

    float p9 = q[3] * q[3];

    // First row
    m[ 0] = 1.0f - 2.0f * ( p7 + p9 );
    m[ 1] = 2.0f * ( p5 + p3 );
    m[ 2] = 2.0f * ( p6 - p2 );
    // Second row
    m[ 3] = 2.0f * ( p5 - p3 );
    m[ 4] = 1.0f - 2.0f * ( p4 + p9 );
    m[ 5] = 2.0f * ( p8 + p1 );
    // Third row
    m[ 6] = 2.0f * ( p6 + p2 );
    m[ 7] = 2.0f * ( p8 - p1 );
    m[ 8] = 1.0f - 2.0f * ( p4 + p7 );
#endif
}
/*! \brief Extracts rotation matrix and converts it into a quaternion.

    Extracts 3x3 rotation matrix and converts it into a 4 element float array representing a quaternion,
    the quaternion is represented as W,X,Y,Z.

    \param matrix [in] The matrix from which to extract the rotation.
    \param q [out] Resulting quaternion.
*/
inline void Matrix4x4ToQuat ( float *matrix, float *q )
{
    /*
    [0][4][ 8][12]
    [1][5][ 9][13]
    [2][6][10][14]
    [3][7][11][15]
    */
    float T = matrix[0] + matrix[5] + matrix[10] + 1;
    float S;
    if ( T > 0.0f )
    {
        S = 0.5f / sqrtf ( T );
        q[0] = 0.25f / S;
        q[1] = ( matrix[6] - matrix[9] ) * S;
        q[2] = ( matrix[8] - matrix[2] ) * S;
        q[3] = ( matrix[1] - matrix[4] ) * S;
        return;
    }
    else
    {
        if ( ( matrix[0] > matrix[5] ) & ( matrix[0] > matrix[10] ) )
        {
            S = sqrtf ( 1.0f + matrix[0] - matrix[5] - matrix[10] ) * 2.0f; // S=4*qx
            q[0] = ( matrix[9] - matrix[6] ) / S;
            q[1] = 0.25f * S;
            q[2] = ( matrix[4] + matrix[1] ) / S;
            q[3] = ( matrix[8] + matrix[2] ) / S;
        }
        else if ( matrix[5] > matrix[10] )
        {
            S = sqrt ( 1.0f + matrix[5] - matrix[0] - matrix[10] ) * 2.0f; // S=4*qy
            q[0] = ( matrix[8] - matrix[2] ) / S;
            q[1] = ( matrix[4] + matrix[1] ) / S;
            q[2] = 0.25f * S;
            q[3] = ( matrix[9] + matrix[6] ) / S;
        }
        else
        {
            S = sqrt ( 1.0f + matrix[10] - matrix[0] - matrix[5] ) * 2.0f; // S=4*qz
            q[0] = ( matrix[4] - matrix[1] ) / S;
            q[1] = ( matrix[8] + matrix[2] ) / S;
            q[2] = ( matrix[9] + matrix[6] ) / S;
            q[3] = 0.25f * S;
        }
    }
}
/*! \brief Multiplies a matrix and a quaternion

    Converts the quaternion to a matrix and multiplies the provided matrix
    with the resulting matrix, this is equivalent of applying the rotation in the
    quaternion to the matrix.
    The dst parameter may be the same as m, in which case m is overwritten.
    \param q [in] Quaternion
    \param m [in] 4x4 matrix.
    \param dst [out] Resulting matrix.
    \sa QuatTo4x4Matrix,MultiplyMatrix
    \note the multiplication is actualy done with the matrix to the left and the quaternion to the right, so the name and input parameters are misleading, should be refactored.
*/
inline void Quat4x4MatrixMult ( float* q, float* m, float* dst )
{
    // \todo Reformat parameter order to match MultiplyMatrix
    float qm[16];
    qm[12] = 0;
    qm[13] = 0;
    qm[14] = 0;
    qm[15] = 1;
    QuatTo4x4Matrix ( q, qm );
    Multiply4x4Matrix ( qm, m, dst );
}
//-------------------------------------------//
/*! \brief Rotates a vector around the origin by a quaternion.

    Applies the rotation stored in a quaternion to a vector, using the origin as pivot point.
    The out parameter may be the same as v or even q in which case the values are overwritten.
    \param q [in] 4 element quaternion [w,x,y,z].
    \param v [in] 3 element vector [x,y,z].
    \param out [out] Rotated 3 element vector.
*/
inline void RotateVectorByQuat ( const float* q, const float* v, float* out )
{
    float localout[3];
#if 1
    float t1 = ( -q[1] * v[0] - q[2] * v[1] - q[3] * v[2] );
    float t2 = (  q[0] * v[0] + q[2] * v[2] - q[3] * v[1] );
    float t3 = (  q[0] * v[1] + q[3] * v[0] - q[1] * v[2] );
    float t4 = (  q[0] * v[2] + q[1] * v[1] - q[2] * v[0] );

    localout[0] = t1 * -q[1] + t2 * q[0] + t3 * -q[3] - t4 * -q[2];
    localout[1] = t1 * -q[2] + t3 * q[0] + t4 * -q[1] - t2 * -q[3];
    localout[2] = t1 * -q[3] + t4 * q[0] + t2 * -q[2] - t3 * -q[1];
#else
    localout[0] =
        ( -q[1] * v[0] - q[2] * v[1] - q[3] * v[2] ) * -q[1] +
        (  q[0] * v[0] + q[2] * v[2] - q[3] * v[1] ) *  q[0] +
        (  q[0] * v[1] + q[3] * v[0] - q[1] * v[2] ) * -q[3] -
        (  q[0] * v[2] + q[1] * v[1] - q[2] * v[0] ) * -q[2] ;
    localout[1] =
        ( -q[1] * v[0] - q[2] * v[1] - q[3] * v[2] ) * -q[2] +
        (  q[0] * v[1] + q[3] * v[0] - q[1] * v[2] ) *  q[0] +
        (  q[0] * v[2] + q[1] * v[1] - q[2] * v[0] ) * -q[1] -
        (  q[0] * v[0] + q[2] * v[2] - q[3] * v[1] ) * -q[3] ;
    localout[2] =
        ( -q[1] * v[0] - q[2] * v[1] - q[3] * v[2] ) * -q[3] +
        (  q[0] * v[2] + q[1] * v[1] - q[2] * v[0] ) *  q[0] +
        (  q[0] * v[0] + q[2] * v[2] - q[3] * v[1] ) * -q[2] -
        (  q[0] * v[1] + q[3] * v[0] - q[1] * v[2] ) * -q[1];
#endif
    out[0] = localout[0];
    out[1] = localout[1];
    out[2] = localout[2];
}
/*! \brief Multiplies two quaternions.

    Multiplies 2 quaternions together effectively combining both rotations.
    The out parameter may be the same as either q1 or q2 in which case the values get overwritten.

    \param q1 [in] Left Quaternion.
    \param q2 [in] Right Quaternion.
    \param out [out] Resulting Quaternion.
*/
inline void MultQuats4 ( float* q1, float* q2, float* out )
{
    float localout[4];
    localout[0] = ( q1[0] * q2[0] ) - ( q1[1] * q2[1] ) - ( q1[2] * q2[2] ) - ( q1[3] * q2[3] );
    localout[1] = ( q1[1] * q2[0] ) + ( q1[0] * q2[1] ) + ( q1[2] * q2[3] ) - ( q1[3] * q2[2] );
    localout[2] = ( q1[2] * q2[0] ) + ( q1[0] * q2[2] ) + ( q1[3] * q2[1] ) - ( q1[1] * q2[3] );
    localout[3] = ( q1[3] * q2[0] ) + ( q1[0] * q2[3] ) + ( q1[1] * q2[2] ) - ( q1[2] * q2[1] );

#if 1
    /* compute magnitude of the quaternion */
    float mag = sqrtf ( ( localout[1] * localout[1] ) + ( localout[2] * localout[2] )
                        + ( localout[3] * localout[3] ) + ( localout[0] * localout[0] ) );

    /* check for bogus length, to protect against divide by zero */
    if ( mag > 0.0f )
    {
        /* normalize it */
        float oneOverMag = 1.0f / mag;
        localout[0] *= oneOverMag;
        localout[1] *= oneOverMag;
        localout[2] *= oneOverMag;
        localout[3] *= oneOverMag;
    }
#endif
    out[0] = localout[0];
    out[1] = localout[1];
    out[2] = localout[2];
    out[3] = localout[3];
}

/*! \brief Linearly interpolate between two quaternions.

    Each element is interpolated as v' = v1+((v2-v1)*interpolation).
    The out parameter may be the same as either q1 or q2 in which case the values are overwritten.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
*/
inline float* LerpQuats ( const float* q1, const float* q2, double interpolation, float* out )
{
#if 0
    float sign = 1.0;
    float dot = Dot4 ( q1, q2 );
#endif
    if ( interpolation <= 0.0f )
    {
        out[1] = q1[1];
        out[2] = q1[2];
        out[3] = q1[3];
        out[0] = q1[0];
        return out;
    }
    else if ( interpolation >= 1.0f )
    {
        out[1] = q2[1];
        out[2] = q2[2];
        out[3] = q2[3];
        out[0] = q2[0];
        return out;
    }
#if 0
    else if ( dot < 0.0f )
    {
        sign = -1.0;
    }
    out[0] = q1[0] + ( ( ( q2[0] * sign ) - q1[0] ) * interpolation );
    out[1] = q1[1] + ( ( ( q2[1] * sign ) - q1[1] ) * interpolation );
    out[2] = q1[2] + ( ( ( q2[2] * sign ) - q1[2] ) * interpolation );
    out[3] = q1[3] + ( ( ( q2[3] * sign ) - q1[3] ) * interpolation );
#else
    out[0] = static_cast<float> ( ( q1[0] * ( 1.0 - interpolation ) ) + ( q2[0] * interpolation ) );
    out[1] = static_cast<float> ( ( q1[1] * ( 1.0 - interpolation ) ) + ( q2[1] * interpolation ) );
    out[2] = static_cast<float> ( ( q1[2] * ( 1.0 - interpolation ) ) + ( q2[2] * interpolation ) );
    out[3] = static_cast<float> ( ( q1[3] * ( 1.0 - interpolation ) ) + ( q2[3] * interpolation ) );
#endif
    return out;
}

/*! \brief Linearly interpolate between two quaternions return the normalized result.

    Each element is interpolated as v' = v1+((v2-v1)*interpolation).
    The out parameter may be the same as either q1 or q2 in which case the values are overwritten.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
*/
inline float* NlerpQuats ( const float* q1, const float* q2, double interp, float* out )
{
    return Normalize4 ( LerpQuats ( q1, q2, interp, out ) );
}

/*! \brief Spherical Linear interpolation between two quaternions.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
*/
inline float* SlerpQuats ( float* q1, float* q2, float interpolation, float* out )
{
    if ( interpolation <= 0.0f )
    {
        out[1] = q1[1];
        out[2] = q1[2];
        out[3] = q1[3];
        out[0] = q1[0];
        return out;
    }
    else if ( interpolation >= 1.0f )
    {
        out[1] = q2[1];
        out[2] = q2[2];
        out[3] = q2[3];
        out[0] = q2[0];
        return out;
    }
    float dot = Dot4 ( q1, q2 );
    float sign = 1.0f;
    if ( fabs ( dot ) > 0.9999f )
    {
        memcpy ( out, q1, sizeof ( float ) * 4 );
        return out;
    }
    else if ( dot < 0.0f )
    {
        dot = -dot;
        sign = -1.0;
    }
    float   theta       = acosf ( dot );
    float   sinT        = 1.0f / sinf ( theta );
    float   newFactor   = sinf ( interpolation * theta ) * sinT;
    float   invFactor   = sinf ( ( 1.0f - interpolation ) * theta ) * sinT;

    out[0] =  invFactor * q1[0] + newFactor * q2[0] * sign;
    out[1] =  invFactor * q1[1] + newFactor * q2[1] * sign;
    out[2] =  invFactor * q1[2] + newFactor * q2[2] * sign;
    out[3] =  invFactor * q1[3] + newFactor * q2[3] * sign;
    return out;
}
#if 0
// Quaternion Logarithm
inline float* QuatLn ( float* q, float* out )
{
    // Clip W
    float w = q[0] > 1.0f ? 1.0f : q[0] < -1.0f ? -1.0f : q[0];
    float a = acosf ( w );
    float s = sinf ( a );

    if (  s == 0.0f )
    {
        memset ( out, 0, sizeof ( float ) * 4 );
    }
    else
    {
        a /= s;
        out[0] = 0;
        out[1] = q[1] * a;
        out[2] = q[2] * a;
        out[3] = q[3] * a;
    }
    return out;
}

// Quaternion Exponent
inline float* QuatExp ( float* q, float* out )
{
    float a = Length ( q + 1 );
    float s = sinf ( a );
    float c = cosf ( a );

    out[0] = c;
    if ( a == 0.0f )
    {
        memset ( out + 1, 0, sizeof ( float ) * 3 );
    }
    else
    {
        s /= a;
        out[1] = q[1] * s;
        out[2] = q[2] * s;
        out[3] = q[3] * s;
    }
    return out;
}
inline float* GetInnerQuat ( float* past, float* current, float* future, float* out )
{
    float q[4] = {current[0], -current[1], -current[2], -current[3]};
    float tmp1[4];
    float tmp2[4];
    float tmp3[4];
    float tmp4[4];
    float tmp5[4];
    float tmp6[4];
    float tmp7[4];
    return Normalize4 ( MultQuats ( current , ScalarMultiply4 ( QuatExp ( Add4 ( QuatLn ( MultQuats ( q , past, tmp1 ), tmp2 ) , QuatLn ( MultQuats ( q , future, tmp3 ), tmp4 ), tmp5 ), tmp6 ) , -0.25f, tmp7 ), out ) );
}

/*! \brief Spherical quadrangle interpolation between two quaternions.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
*/
inline float* SquadQuats ( float* q0, float* q1, float* q2, float* q3, float interp, float* out )
{
    // http://theory.org/software/qfa/writeup/node12.html
    // ken shoemake Quaternion Calculus and Fast Animation
    // http://www.gamedev.net/topic/272295-interpolation-using-squad/
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb281657%28v=vs.85%29.aspx
    // http://www.gamedev.net/topic/421365-spline-based-quaternion-interpolation/
    // http://willperone.net/Code/quaternion.php
    float tmp1[4];
    float tmp2[4];
    float a1[4];
    float a2[4];
    return SlerpQuats ( SlerpQuats ( q1, q2, interp, tmp1 ), SlerpQuats ( GetInnerQuat ( q0, q1, q2, a1 ), GetInnerQuat ( q1, q2, q3, a2 ), interp, tmp2 ), 2 * interp * ( 1 - interp ), out );
    //return SlerpQuatsNoInvert ( SlerpQuatsNoInvert ( q1, q2, interp, tmp1 ), SlerpQuatsNoInvert ( GetInnerQuad ( q0, q1, q2, a1 ), GetInnerQuad ( q1, q2, q3, a2 ), interp, tmp2 ), 2 * interp * ( 1 - interp ), out );
}
#endif

// @}
/*! \name SRT Functions */
// @{
/// This function should be equivalent to multiplying the two matrices generated from the SRTs.
inline float* MultSRTs ( const float* srt1, const float* srt2, float* out )
{
    float localout[10];
    // Scale
    localout[0] = srt1[0] * srt2[0];
    localout[1] = srt1[1] * srt2[1];
    localout[2] = srt1[2] * srt2[2];
    // Rotation
    MultQuats ( srt1 + 3, srt2 + 3, localout + 3 );
    // Translation
    RotateVectorByQuat ( srt1 + 3, srt2 + 7, localout + 7 );
    localout[7] += srt1[7];
    localout[8] += srt1[8];
    localout[9] += srt1[9];
    memcpy ( out, localout, sizeof ( float ) * 10 );
    return out;
}

inline float* InvertSRT ( const float* srt, float* out )
{
    out[0] = 1.0f / srt[0];
    out[1] = 1.0f / srt[1];
    out[2] = 1.0f / srt[2];
    out[3] = srt[3];
    out[4] = -srt[4];
    out[5] = -srt[5];
    out[6] = -srt[6];
    out[7] = -srt[7];
    out[8] = -srt[8];
    out[9] = -srt[9];
    RotateVectorByQuat ( out + 3, out + 7, out + 7 );
    return out;
}
// @}

/*! \name Distance functions */
// @{
inline float PointDistanceToPlane ( float* plane, const float* point, const float* dimensions = nullptr )
{
    return
        plane[0] * point[0] +
        plane[1] * point[1] +
        plane[2] * point[2] -
        plane[3];
}

inline float SphereDistanceToPlane ( float* plane, const float* point, const float* dimensions )
{
    return
        plane[0] * point[0] +
        plane[1] * point[1] +
        plane[2] * point[2] -
        ( plane[3] + dimensions[0] );
}

inline float BoxDistanceToPlane ( float* plane, const float* point, const float* dimensions )
{
    float offsets[3] =
    {
        ( plane[0] < 0 ) ? dimensions[0] : -dimensions[0],
        ( plane[1] < 0 ) ? dimensions[1] : -dimensions[1],
        ( plane[2] < 0 ) ? dimensions[2] : -dimensions[2]
    };

    float dist = plane[3] - Dot ( offsets, plane );

    return
        plane[0] * point[0] +
        plane[1] * point[1] +
        plane[2] * point[2] - dist;
}

inline float CapsuleDistanceToPlane ( float* plane, const float* point, const float* dimensions )
{
    // radius is dimensions[0] and halfheight is dimensions [1]

    float dist = plane[3] + dimensions[0];

    float offset = dimensions[1] - dimensions[0];

    float direction = plane[2] * offset;

    if ( direction > 0 )
    {
        // if the offset (halfheight-radius) is in the same direction as the plane normal subtract it from the point
        return
            plane[0] * point[0] +
            plane[1] * point[1] +
            plane[2] * ( point[2] - offset ) -
            dist;
    }
    // if the offset (halfheight-radius) is in the oposite direction or parallel to the plane normal add it to the point
    return
        plane[0] * point[0] +
        plane[1] * point[1] +
        plane[2] * ( point[2] + offset ) -
        dist;
}
// @}

inline void PrintMatrix ( float* M )
{
    printf (
        "%f %f %f %f\n"
        "%f %f %f %f\n"
        "%f %f %f %f\n"
        "%f %f %f %f\n",
        M[0], M[4], M[ 8], M[12],
        M[1], M[5], M[ 9], M[13],
        M[2], M[6], M[10], M[14],
        M[3], M[7], M[11], M[15]
    );
}
#endif

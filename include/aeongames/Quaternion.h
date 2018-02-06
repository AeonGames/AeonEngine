/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_QUATERNION_H
#define AEONGAMES_QUATERNION_H
/*! \file
    \brief Header for the quaternion class.
    \author Rodrigo Hernandez.
    \copy 2017
*/

#include "aeongames/Platform.h"

namespace AeonGames
{
    class Matrix4x4;
    class Vector3;
    /*! \brief Quaternion class. */
    class Quaternion
    {
    public:
        ///@brief Default constructor.
        DLL Quaternion();
        DLL Quaternion ( float w, float x, float y, float z );
        /// destructor.
        DLL ~Quaternion();
        DLL Matrix4x4 GetMatrix4x4() const;
        /*! \name Operators */
        //@{
        DLL Quaternion& operator*= ( const Quaternion& lhs );
        DLL float operator[] ( const size_t aIndex ) const;
        DLL float& operator [] ( const size_t aIndex );
        //@}
        DLL static const Quaternion GetFromAxisAngle ( float angle, float x, float y, float z );
        DLL Quaternion& Normalize();
    private:
        /// W,X,Y,Z
        float mQuaternion[4] {1.0f, 0.0f, 0.0f, 0.0f};
    };
    DLL const Quaternion operator* ( const Quaternion& lhs, const Quaternion& rhs );
    DLL const Vector3 operator* ( const Quaternion& lhs, const Vector3& rhs );
    DLL bool operator== ( const Quaternion& lhs, const Quaternion& rhs );


    /*! \brief Linearly interpolate between two quaternions.

    Each element is interpolated as v' = v1+((v2-v1)*interpolation).
    The out parameter may be the same as either q1 or q2 in which case the values are overwritten.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
    */
    DLL const Quaternion LerpQuats ( const Quaternion& q1, const Quaternion& q2, double interpolation );

    /*! \brief Linearly interpolate between two quaternions return the normalized result.

    Each element is interpolated as v' = v1+((v2-v1)*interpolation).
    The out parameter may be the same as either q1 or q2 in which case the values are overwritten.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
    */
    DLL const Quaternion NlerpQuats ( const Quaternion& q1, const Quaternion& q2, double interpolation );

    /*! \brief Spherical Linear interpolation between two quaternions.
    \param q1 [in] Origin quaternion.
    \param q2 [in] Destination quaternion.
    \param interp [in] Interpolation factor.
    \param out [out] Resulting quaternion.
    */
    DLL const Quaternion SlerpQuats ( const Quaternion& q1, const Quaternion& q2, float interpolation );
}
#endif

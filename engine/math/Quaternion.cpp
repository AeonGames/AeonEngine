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
#include "aeongames/Quaternion.h"
#include "aeongames/Matrix4x4.h"
#include "3DMath.h"

namespace AeonGames
{
    Quaternion::Quaternion()
    {
    }
    Quaternion::Quaternion ( float w, float x, float y, float z ) :
        mQuaternion{w, x, y, z}
    {
    }
    Quaternion::~Quaternion()
        = default;
    Matrix4x4 Quaternion::GetMatrix4x4() const
    {
        return Matrix4x4
        {
            1.0f - 2.0f * ( mQuaternion[2] * mQuaternion[2] + mQuaternion[3] * mQuaternion[3] ),
            2.0f * ( mQuaternion[1] * mQuaternion[2] + mQuaternion[3] * mQuaternion[0] ),
            2.0f * ( mQuaternion[1] * mQuaternion[3] - mQuaternion[2] * mQuaternion[0] ),
            0.0f,
            // Second row
            2.0f * ( mQuaternion[1] * mQuaternion[2] - mQuaternion[3] * mQuaternion[0] ),
            1.0f - 2.0f * ( mQuaternion[1] * mQuaternion[1] + mQuaternion[3] * mQuaternion[3] ),
            2.0f * ( mQuaternion[3] * mQuaternion[2] + mQuaternion[1] * mQuaternion[0] ),
            0.0f,
            // Third row
            2.0f * ( mQuaternion[1] * mQuaternion[3] + mQuaternion[2] * mQuaternion[0] ),
            2.0f * ( mQuaternion[2] * mQuaternion[3] - mQuaternion[1] * mQuaternion[0] ),
            1.0f - 2.0f * ( mQuaternion[1] * mQuaternion[1] + mQuaternion[2] * mQuaternion[2] ),
            0.0f,
            0, 0, 0, 1};
    }
}

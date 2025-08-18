/*
Copyright (C) 2018,2025 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_VECTOR2_H
#define AEONGAMES_VECTOR2_H

#include "aeongames/Platform.hpp"

namespace AeonGames
{
    /*! \brief 2D vector class. */
    class Vector2
    {
    public:
        ///@brief Default constructor.
        DLL Vector2();
        /*! @brief Constructor
        @param aVector a float pointer or array containing vector data.
        */
        DLL Vector2 ( const float* const aVector );
        DLL Vector2 ( float aX, float aY );
        /// destructor.
        DLL ~Vector2();
        DLL const float* const GetVector() const;
        DLL const float& GetX() const;
        DLL const float& GetY() const;
    protected:
        float mVector[2];
    };
    DLL bool operator== ( const Vector2& aLhs, const Vector2& aRhs );
}
#endif

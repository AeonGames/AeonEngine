/*
Copyright (C) 2018,2025,2026 Rodrigo Jose Hernandez Cordoba

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
        /** @brief Constructor.
            @param aX X component.
            @param aY Y component.
        */
        DLL Vector2 ( float aX, float aY );
        /// destructor.
        DLL ~Vector2();
        /** @brief Get a pointer to the internal vector data.
            @return Pointer to the float array containing X,Y components.
        */
        DLL const float* const GetVector() const;
        /** @brief Get the X component.
            @return Reference to the X component.
        */
        DLL const float& GetX() const;
        /** @brief Get the Y component.
            @return Reference to the Y component.
        */
        DLL const float& GetY() const;
    protected:
        float mVector[2]; ///< X and Y components.
    };
    /** @brief Equality comparison operator.
        @param aLhs Left-hand side vector.
        @param aRhs Right-hand side vector.
        @return true if both vectors are equal.
    */
    DLL bool operator== ( const Vector2& aLhs, const Vector2& aRhs );
}
#endif

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
#ifndef AEONGAMES_CONTAINER_H
#define AEONGAMES_CONTAINER_H

#include <vector>
#include <memory>
#include <algorithm>

namespace AeonGames
{
    /**
     * @brief Owning container that stores uniquely-owned objects of type T.
     * @tparam T The type of objects stored in the container.
     */
    template<class T>
    class Container
    {
    public:
        /**
         * @brief Construct and store a new object in-place.
         * @tparam Args Constructor argument types.
         * @param args Arguments forwarded to the T constructor.
         * @return Raw pointer to the newly stored object.
         */
        template <typename... Args>
        T* Store ( Args... args )
        {
            mStorage.emplace_back ( std::make_unique<T> ( args... ) );
            return mStorage.back().get();
        }
        /**
         * @brief Store an existing uniquely-owned object.
         * @param value Unique pointer to the object to store (moved).
         * @return Raw pointer to the stored object.
         */
        T* Store ( std::unique_ptr<T>&& value )
        {
            mStorage.emplace_back ( std::move ( value ) );
            return mStorage.back().get();
        }
        /**
         * @brief Remove an object from the container and return ownership.
         * @param t Pointer to the object to dispose.
         * @return Unique pointer to the removed object, or nullptr if not found.
         */
        std::unique_ptr<T> Dispose ( const T* t )
        {
            std::unique_ptr<T> result{};
            auto i = std::find_if ( mStorage.begin(), mStorage.end(), [t] ( const std::unique_ptr<T>& ref )
            {
                return t == ref.get();
            } );
            if ( i != mStorage.end() )
            {
                result = std::move ( *i );
                mStorage.erase ( std::remove ( i, mStorage.end(), *i ), mStorage.end() );
            }
            return result;
        }
    private:
        std::vector<std::unique_ptr<T >> mStorage{};
    };
}
#endif

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
#ifndef AEONGAMES_CONTAINER_H
#define AEONGAMES_CONTAINER_H

#include <vector>
#include <memory>
#include <algorithm>

namespace AeonGames
{
    template<class T>
    class Container
    {
    public:
        template <typename... Args>
        T* Store ( Args... args )
        {
            mStorage.emplace_back ( std::make_unique<T> ( args... ) );
            return mStorage.back().get();
        }
        T* Store ( std::unique_ptr<T>&& value )
        {
            mStorage.emplace_back ( std::move ( value ) );
            return mStorage.back().get();
        }
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

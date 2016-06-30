/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_UNIFORM_H
#define AEONGAMES_UNIFORM_H

#include <cstdint>
#include <memory>

namespace AeonGames
{
    class Texture;
    class Uniform
    {
    public:
        Uniform ( float aX );
        Uniform ( float aX, float aY );
        Uniform ( float aX, float aY, float aZ );
        Uniform ( float aX, float aY, float aZ, float aW );
        ~Uniform();
        int32_t Location() const;
        void SetLocation ( int32_t aLocation );
    private:
        int32_t mLocation = -1;
        uint32_t mType = 0;
        uint8_t mData[sizeof ( float ) * 4];
    };
}
#endif
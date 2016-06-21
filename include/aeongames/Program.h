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
#ifndef AEONGAMES_PROGRAM_H
#define AEONGAMES_PROGRAM_H

namespace AeonGames
{
    class Program
    {
    public:
        virtual void SetViewMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetProjectionMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetModelMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetViewProjectionMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetModelViewMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetModelViewProjectionMatrix ( const float aMatrix[16] ) = 0;
        virtual void SetNormalMatrix ( const float aMatrix[9] ) = 0;
        virtual void Use() const = 0;
    protected:
        virtual ~Program() = default;
    };
}
#endif
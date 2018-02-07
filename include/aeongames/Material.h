/*
Copyright (C) 2016,2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MATERIAL_H
#define AEONGAMES_MATERIAL_H
#include <string>
#include <vector>
#include "Uniform.h"

namespace AeonGames
{
class MaterialBuffer;
class Material
{
public:
    Material ( std::string  aFilename );
    Material ( const MaterialBuffer& aMaterialBuffer );
    ~Material();
    DLL const std::vector<Uniform>& GetUniformMetaData() const;
    DLL uint32_t GetUniformBlockSize() const;
private:
    void Initialize ( const MaterialBuffer& aMaterialBuffer );
    void Finalize();
    std::string mFilename;
    std::vector<Uniform> mUniformMetaData;
};
}
#endif

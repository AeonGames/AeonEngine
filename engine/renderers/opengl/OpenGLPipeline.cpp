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
#include "OpenGLPipeline.h"
#include "OpenGLFunctions.h"
namespace AeonGames
{
#include "main.vert.h"
#include "main.frag.h"
}
namespace AeonGames
{
OpenGLPipeline::OpenGLPipeline() try :
        mProgram ( 0 )
    {
        Initialize();
    }
    catch ( ... )
    {
        Finalize();
        throw;
    }

    OpenGLPipeline::~OpenGLPipeline()
    {
    }

    void OpenGLPipeline::Initialize()
    {
#if 0
        // There must be a Current OGL Context before calling glCreateProgram
        mProgram = glCreateProgram();
        OPENGL_CHECK_ERROR ( true );
#endif
    }

    void OpenGLPipeline::Finalize()
    {
        if ( glIsProgram ( mProgram ) )
        {
            glDeleteProgram ( mProgram );
            mProgram = 0;
        }
    }
}
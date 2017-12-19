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
#include "aeongames/Window.h"
#include "aeongames/Platform.h"
namespace AeonGames
{
    DLL Window::~Window() {}
    void Window::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix;
    }
    void Window::SetViewTransform ( const Transform& aTransform )
    {
        mViewTransform = aTransform;
    }
    const Matrix4x4 & Window::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }
    const Transform & Window::GetViewTransform() const
    {
        return mViewTransform;
    }
}

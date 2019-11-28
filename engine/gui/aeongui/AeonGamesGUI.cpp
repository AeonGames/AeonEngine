/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#include "AeonGamesGUI.h"
namespace AeonGames
{
    AeonGamesGUI::AeonGamesGUI()
    {
    }
    AeonGamesGUI::~AeonGamesGUI()
    {
    }
    void AeonGamesGUI::ResizeViewport ( uint32_t aWidth, uint32_t aHeight )
    {
        mAeonGUIWindow.ResizeViewport ( aWidth, aHeight );
    }
    const uint8_t* AeonGamesGUI::GetPixels() const
    {
        return mAeonGUIWindow.GetPixels();
    }
    size_t AeonGamesGUI::GetWidth() const
    {
        return mAeonGUIWindow.GetWidth();
    }
    size_t AeonGamesGUI::GetHeight() const
    {
        return mAeonGUIWindow.GetHeight();
    }
    size_t AeonGamesGUI::GetStride() const
    {
        return mAeonGUIWindow.GetStride();
    }
    void AeonGamesGUI::Draw()
    {
        mAeonGUIWindow.Draw();
    }
}

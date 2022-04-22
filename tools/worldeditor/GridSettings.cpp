/*
Copyright (C) 2015-2018,2022 Rodrigo Jose Hernandez Cordoba

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
#include "GridSettings.h"

namespace AeonGames
{
    GridSettings::GridSettings() :
        mWidth ( 768.0f ), mHeight ( 768.0f ),
        mOddLineColor ( 74, 74, 74 ),
        mEvenLineColor ( 74, 74, 74 ),
        mXLineColor ( 255, 0, 0 ),
        mYLineColor ( 0, 255, 0 ),
        mBorderLineColor ( 74, 74, 74 ),
        mHorizontalSpacing ( 16 ), mVerticalSpacing ( 16 )
    {
    }

    GridSettings::~GridSettings() = default;

    void GridSettings::setWidth ( float aWidth )
    {
        mWidth = aWidth;
    }
    void GridSettings::setHeight ( float aHeight )
    {
        mHeight = aHeight;
    }
    void GridSettings::setOddLineColor ( QColor& aColor )
    {
        mOddLineColor = aColor;
    }
    void GridSettings::setEvenLineColor ( QColor& aColor )
    {
        mEvenLineColor = aColor;
    }
    void GridSettings::setXLineColor ( QColor& aColor )
    {
        mXLineColor = aColor;
    }
    void GridSettings::setYLineColor ( QColor& aColor )
    {
        mYLineColor = aColor;
    }
    void GridSettings::setBorderLineColor ( QColor& aColor )
    {
        mBorderLineColor = aColor;
    }
    void GridSettings::setHorizontalSpacing ( uint32_t aHorizaontalSpacing )
    {
        mHorizontalSpacing = aHorizaontalSpacing;
    }
    void GridSettings::setVerticalSpacing ( uint32_t aVerticalSpacing )
    {
        mVerticalSpacing = aVerticalSpacing;
    }
    float GridSettings::width() const
    {
        return mWidth;
    }
    float GridSettings::height() const
    {
        return mHeight;
    }
    const QColor& GridSettings::oddLineColor() const
    {
        return mOddLineColor;
    }
    const QColor& GridSettings::evenLineColor() const
    {
        return mEvenLineColor;
    }
    const QColor& GridSettings::xLineColor() const
    {
        return mXLineColor;
    }
    const QColor& GridSettings::yLineColor() const
    {
        return mYLineColor;
    }
    const QColor& GridSettings::borderLineColor() const
    {
        return mBorderLineColor;
    }
    uint32_t GridSettings::horizontalSpacing() const
    {
        return mHorizontalSpacing;
    }
    uint32_t GridSettings::verticalSpacing() const
    {
        return mVerticalSpacing;
    }
}

/*
Copyright (C) 2015-2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_GRIDSETTINGS_H
#define AEONGAMES_GRIDSETTINGS_H
#include <QColor>
#include <cstdint>
namespace AeonGames
{
    class GridSettings
    {
    public:
        GridSettings();
        ~GridSettings();
        void setWidth ( float aWidth );
        void setHeight ( float aHeight );
        void setOddLineColor ( QColor& aColor );
        void setEvenLineColor ( QColor& aColor );
        void setXLineColor ( QColor& aColor );
        void setYLineColor ( QColor& aColor );
        void setBorderLineColor ( QColor& aColor );
        void setHorizontalSpacing ( uint32_t aHorizaontalSpacing );
        void setVerticalSpacing ( uint32_t aVerticalSpacing );
        float width() const;
        float height() const;
        const QColor& oddLineColor() const;
        const QColor& evenLineColor() const;
        const QColor& xLineColor() const;
        const QColor& yLineColor() const;
        const QColor& borderLineColor() const;
        uint32_t horizontalSpacing() const;
        uint32_t verticalSpacing() const;
#if 0
        void setDimensions ( float aWidth, float aHeight );
        void setOddLineColor ( float aRed, float aGreen, float aBlue, float aAlpha );
        void setEvenLineColor ( float aRed, float aGreen, float aBlue, float aAlpha );
        void setXLineColor ( float aRed, float aGreen, float aBlue, float aAlpha );
        void setYLineColor ( float aRed, float aGreen, float aBlue, float aAlpha );
        void setBorderLineColor ( float aRed, float aGreen, float aBlue, float aAlpha );
#endif
    private:
        float mWidth;
        float mHeight;
        QColor mOddLineColor;
        QColor mEvenLineColor;
        QColor mXLineColor;
        QColor mYLineColor;
        QColor mBorderLineColor;
        uint32_t mHorizontalSpacing;
        uint32_t mVerticalSpacing;
    };
}
#endif

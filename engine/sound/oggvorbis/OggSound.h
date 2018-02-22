/*
Copyright (C) 2017-2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OGGSOUND_H
#define AEONGAMES_OGGSOUND_H
#include <string>
#include <vector>
#include "aeongames/Sound.h"
#include "vorbis/vorbisfile.h"

namespace AeonGames
{
    class OggSound : public Sound
    {
    public:
        OggSound ( const std::string & aFileName );
        ~OggSound() final;
    private:
        ///@name Callback Functions
        ///@{
        static size_t Read  ( void *ptr, size_t size, size_t nmemb, void *datasource );
        static int Seek  ( void *datasource, ogg_int64_t offset, int whence );
        static int Close ( void *datasource );
        static long Tell  ( void *datasource );
        static const ov_callbacks Callbacks;
        ///@}
        //OggVorbis_File mOggVorbisFile{};
        std::vector<uint8_t> mData;
    };
}
#endif

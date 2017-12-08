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
#include "OggSound.h"
#include "vorbis/codec.h"
#include <fstream>
#include <algorithm>
#include <exception>
#include <stdexcept>

namespace AeonGames
{
    size_t OggSound::Read ( void *ptr, size_t size, size_t nmemb, void *datasource )
    {
        OggSound* ogg_sound = static_cast<OggSound*> ( datasource );
        return 0;
    }
    int OggSound::Seek ( void *datasource, ogg_int64_t offset, int whence )
    {
        OggSound* ogg_sound = static_cast<OggSound*> ( datasource );
        return 0;
    }
    int OggSound::Close ( void *datasource )
    {
        OggSound* ogg_sound = static_cast<OggSound*> ( datasource );
        return 0;
    }
    long OggSound::Tell ( void *datasource )
    {
        OggSound* ogg_sound = static_cast<OggSound*> ( datasource );
        return 0;
    }

    const ov_callbacks OggSound::Callbacks
    {
        Read,
        Seek,
        Close,
        Tell
    };

    OggSound::OggSound ( const std::string & aFileName )
    {
        //--------------------------------------------------------
        // File loading code
        std::ifstream file;
        file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        file.open ( aFileName, std::ios::binary );
        mData = std::vector<uint8_t> ( ( std::istreambuf_iterator<char> ( file ) ),
                                       ( std::istreambuf_iterator<char>() ) );
        file.close();
        //--------------------------------------------------------
    }

    OggSound::~OggSound()
    {
    }
}

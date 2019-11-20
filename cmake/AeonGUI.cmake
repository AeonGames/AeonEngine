# Copyright (C) 2019 AeonGames, Rodrigo Jose Hernandez Cordoba Licensed under
# the terms of the Apache 2.0 License.

include(gitclone)
option(USE_AEONGUI "Use the AeonGUI library for the user interface" OFF)
if(USE_AEONGUI)
    message(STATUS "Using AeonGUI")
    gitclone(https://github.com/AeonGames/AeonGUI.git)
endif()

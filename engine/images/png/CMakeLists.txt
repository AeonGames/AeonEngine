set(PNGIMAGE_HEADERS PngImage.h)
set(PNGIMAGE_SOURCES PngImage.cpp)
add_library(PngImage SHARED ${PNGIMAGE_HEADERS} ${PNGIMAGE_SOURCES})
target_link_libraries(PngImage engine ${PNG_LIBRARIES})

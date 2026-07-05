find_package(PNG)
if(PNG_FOUND)
  add_subdirectory(png)
endif()

# Radiance HDR (.hdr) loader for high-dynamic-range environment maps. The
# decoder is self-contained (no external dependency).
add_subdirectory(hdr)

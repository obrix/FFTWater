# Try to find FFTW3 library and include dir.
# Once done this will define
#
# FFTW3_FOUND
# FFTW3_INCLUDE_DIR
# FFTW3_LIBRARY
#

include(FindPackageHandleStandardArgs)

if (WIN32)
    find_path( FFTW3_INCLUDE_DIR
        NAMES
            fftw3/fftw3.h
        PATHS
            C:/lib
            $ENV{PROGRAMFILES}
            ${FFTW3_LOCATION}
            $ENV{FFTW3_LOCATION}
            DOC "The directory where GLFW/glfw3.h resides" )

    find_library( FFTW3_LIBRARY
        NAMES
            fftw3/libfftw3l-3.lib
        PATHS
            C:/lib
            $ENV{PROGRAMFILES}
            ${FFTW3_LOCATION}
            $ENV{FFTW3_LOCATION}
            DOC "The GLFW library")
else ()

find_path( FFTW3_INCLUDE_DIR
        NAMES
            fftw3.h
        PATHS
            /usr/include/
            $ENV{PROGRAMFILES}
            ${FFTW3_LOCATION}
            $ENV{FFTW3_LOCATION}
            DOC "The directory where GLFW/glfw3.h resides" )

    find_library( FFTW3_LIBRARY
        NAMES
            libfftw3.so
        PATHS
            /usr/lib/x86_64-linux-gnu/
            $ENV{PROGRAMFILES}
            ${FFTW3_LOCATION}
            $ENV{FFTW3_LOCATION}
            DOC "The GLFW library")

endif ()

find_package_handle_standard_args(FFTW3 DEFAULT_MSG
    FFTW3_INCLUDE_DIR
    FFTW3_LIBRARY
)

mark_as_advanced( FFTW3_FOUND )

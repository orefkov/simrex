if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/simrex-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package simrex)

install(
    DIRECTORY
    include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT simrex_Development
)

install(
    TARGETS simrex_simrex
    EXPORT simrexTargets
    RUNTIME #
    COMPONENT simrex_Runtime
    LIBRARY #
    COMPONENT simrex_Runtime
    NAMELINK_COMPONENT simrex_Development
    ARCHIVE #
    COMPONENT simrex_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    simrex_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE simrex_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(simrex_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${simrex_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT simrex_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${simrex_INSTALL_CMAKEDIR}"
    COMPONENT simrex_Development
)

install(
    EXPORT simrexTargets
    NAMESPACE simrex::
    DESTINATION "${simrex_INSTALL_CMAKEDIR}"
    COMPONENT simrex_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()

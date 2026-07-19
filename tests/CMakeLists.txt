# Unit tests (QtTest). Built only when -DWHATLY_TESTS=ON. Kept lightweight: they
# link just the code under test and its direct dependencies, not the whole app
# (no MainWindow, no WebEngine instances), so they build and run fast and
# headless (offscreen).
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Widgets Gui Svg WebEngineCore)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src)

# Optional gcov instrumentation for coverage reports (see tools/coverage.sh).
option(WHATLY_COVERAGE "Instrument the tests for gcov/gcovr coverage" OFF)
if(WHATLY_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

set(S ${CMAKE_SOURCE_DIR}/src)

# Shared build-stamp macros the app sources expect.
set(TEST_DEFS VERSIONSTR="test" GIT_HASH="test" GIT_BRANCH="test"
              BUILD_TIMESTAMP="test")

# ── UI-asset test (widgets: About + RateApp) ────────────────────────────────
add_executable(tst_ui_assets
    tst_ui_assets.cpp
    ${S}/about.cpp ${S}/rateapp.cpp ${S}/utils.cpp ${S}/debuglog.cpp
    ${S}/appprofile.cpp ${S}/icons.qrc
)
target_include_directories(tst_ui_assets PRIVATE ${S} ${CMAKE_BINARY_DIR})
target_compile_definitions(tst_ui_assets PRIVATE ${TEST_DEFS})
target_link_libraries(tst_ui_assets PRIVATE
    Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Svg)
add_test(NAME ui_assets COMMAND tst_ui_assets)
set_tests_properties(ui_assets PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# ── Logic test (headless helpers, script generators, queue, sun, etc.) ───────
add_executable(tst_logic
    tst_logic.cpp
    ${S}/utils.cpp ${S}/debuglog.cpp ${S}/appprofile.cpp ${S}/common.cpp
    ${S}/sunclock.cpp ${S}/identicons.cpp ${S}/theme.cpp ${S}/dictionaries.cpp
    ${S}/scheduledmessages.cpp
    ${S}/webfont.cpp ${S}/chattheme.cpp ${S}/mutedstatus.cpp ${S}/privacyblur.cpp
    ${S}/chatwallpaper.cpp ${S}/customcss.cpp ${S}/webtweaks.cpp
    ${S}/linkeddevicename.cpp
    ${S}/icons.qrc
)
target_include_directories(tst_logic PRIVATE ${S} ${CMAKE_BINARY_DIR})
target_compile_definitions(tst_logic PRIVATE ${TEST_DEFS})
target_link_libraries(tst_logic PRIVATE
    Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::WebEngineCore)
add_test(NAME logic COMMAND tst_logic)
set_tests_properties(logic PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

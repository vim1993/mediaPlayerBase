PROJECT(PLAYER_1_0)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

#####################################
#videoPlayer: compiler video player OBJ
#audioPlayer: compiler audio player OBJ
#Player: compiler player OBJ
#####################################
#SET(OBJ_PRODUCT "audioPlayer")

#set include path
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/inc)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/inc/media)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/inc/thirdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/inc/Utils)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/inc/surface)

#set src path
AUX_SOURCE_DIRECTORY(. MAIN_SRC)
#IF(OBJ_PRODUCT STREQUAL "videoPlayer")
#AUX_SOURCE_DIRECTORY("./src/videoPlay/" PLAYER_SRC)
#ELSE(OBJ_PRODUCT STREQUAL "audioPlayer")
AUX_SOURCE_DIRECTORY(./src/platform PLATFORM_PLAYER_SRC)
AUX_SOURCE_DIRECTORY(./src/utils PLAYER_UTILS)
#ENDIF()

#set lib path
LINK_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/lib)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/lib)
#executable
ADD_EXECUTABLE(${PROJECT_NAME} ${PLATFORM_PLAYER_SRC} ${MAIN_SRC} ${PLAYER_UTILS})

#link lib
TARGET_LINK_LIBRARIES(${PROJECT_NAME} WHEEL_1_0 SDL2 avcodec avdevice avfilter avformat avutil swresample swscale pthread)
cmake_minimum_required(VERSION 3.22)

message(STATUS "Getting version information...")

################################
# How a version string is built:
# if branch == "main" || branch == "hotfix":
#   version := <MAJOR>.<MINOR>.<PATCH>.0
# if branch == "release":
#   version := <MAJOR>.<MINOR>.<PATCH>.<#Commits since diverted from develope branch>
# elif ENV(CI_PIPELINE_IID) != "":
#   version := <MAJOR>.<MINOR>.<PATCH>.ENV(CI_PIPELINE_IID)_<BRANCH>
# else:
#   version := <MAJOR>.<MINOR>.<PATCH>.-1_<BRANCH>
################################

# Ensure git is available
find_program(GIT_PATH NAMES git)
if(GIT_PATH STREQUAL "GIT_PATH-NOTFOUND")
    message(FATAL_ERROR "git not found but it is requred for generating version information.")
endif()

# Get the current branch
if(DEFINED ENV{CI_COMMIT_BRANCH} AND NOT $ENV{CI_COMMIT_BRANCH} STREQUAL "")
    set(BRANCH_NAME "$ENV{CI_COMMIT_BRANCH}")
else()
    # Execute: git rev-parse --abbrev-ref HEAD
    set(GIT_REV_PARSE_COMMAND "rev-parse" "--abbrev-ref" "HEAD")
    list(JOIN GIT_REV_PARSE_COMMAND " " GIT_REV_PARSE_COMMAND_STR)
    execute_process(COMMAND "${GIT_PATH}" ${GIT_REV_PARSE_COMMAND}
                    OUTPUT_VARIABLE GIT_REV_PARSE_OUTPUT
                    ERROR_VARIABLE GIT_REV_PARSE_ERROR
                    RESULT_VARIABLE GIT_REV_PARSE_RETURN
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT GIT_REV_PARSE_RETURN EQUAL 0)
        message(FATAL_ERROR "Running '${GIT_PATH} ${GIT_REV_PARSE_COMMAND_STR}' failed with return code ${GIT_REV_PARSE_RETURN}. Output:\n${GIT_REV_PARSE_OUTPUT}\n${GIT_REV_PARSE_ERROR}")
    endif()

    set(BRANCH_NAME "${GIT_REV_PARSE_OUTPUT}")
endif()

message(STATUS "Current branch: ${BRANCH_NAME}")

# CI_COMMIT_BRANCH will be set by the CI
if(BRANCH_NAME STREQUAL "main")
    message(STATUS "main-branch compilation detected.")
    
    set("${PROJECT_NAME}_VERSION_MORE" "0")
    set("${PROJECT_NAME}_VERSION_END" ${${PROJECT_NAME}_VERSION_MORE})

elseif(BRANCH_NAME MATCHES "^hotfix\/.*")
    message(STATUS "hotfix-branch compilation detected.")

    set("${PROJECT_NAME}_VERSION_MORE" "0")
    set("${PROJECT_NAME}_VERSION_END" ${${PROJECT_NAME}_VERSION_MORE})

elseif(BRANCH_NAME MATCHES "^release\/.*")
    message(STATUS "release-branch compilation detected.")

    # Execute: git fetch origin develop:develop
    set(GIT_FETCH_COMMAND "fetch" "origin" "develop:develop")
    list(JOIN GIT_FETCH_COMMAND " " GIT_FETCH_COMMAND_STR)
    execute_process(COMMAND "${GIT_PATH}" ${GIT_FETCH_COMMAND}
                    OUTPUT_VARIABLE GIT_FETCH_OUTPUT
                    ERROR_VARIABLE GIT_FETCH_ERROR
                    RESULT_VARIABLE GIT_FETCH_RETURN
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT GIT_FETCH_RETURN EQUAL 0)
        message(FATAL_ERROR "Running '${GIT_PATH} ${GIT_FETCH_COMMAND_STR}' failed with return code ${GIT_FETCH_RETURN}. Output:\n${GIT_FETCH_OUTPUT}\n${GIT_FETCH_ERROR}")
    endif()

    # Get the current release candidate number (git rev-list --count HEAD ^develop)
    set(GIT_REV_LIST_COMMAND "rev-list" "--count" "HEAD" "^develop")
    list(JOIN GIT_REV_LIST_COMMAND " " GIT_REV_LIST_COMMAND_STR)
    execute_process(COMMAND "${GIT_PATH}" ${GIT_REV_LIST_COMMAND}
                    OUTPUT_VARIABLE GIT_REV_LIST_OUTPUT
                    ERROR_VARIABLE GIT_REV_LIST_ERROR
                    RESULT_VARIABLE GIT_REV_LIST_RETURN
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT GIT_REV_LIST_RETURN EQUAL 0)
        message(FATAL_ERROR "Running '${GIT_PATH} ${GIT_REV_LIST_COMMAND_STR}' failed with return code ${GIT_REV_LIST_RETURN}. Output:\n${GIT_REV_LIST_OUTPUT}\n{GIT_REV_LIST_ERROR}")
    endif()

    set("${PROJECT_NAME}_VERSION_MORE" "${GIT_REV_LIST_OUTPUT}")
    set("${PROJECT_NAME}_VERSION_END" ${${PROJECT_NAME}_VERSION_MORE})

else()
    message(STATUS "No special branch detected. Using standard versioning.")

    # Check if the VERSION_MORE part got overriden by the CI variable
    if(DEFINED ENV{VERSION_MORE})
        set("${PROJECT_NAME}_VERSION_MORE" "$ENV{VERSION_MORE}")
    else()
        if(DEFINED ENV{CI_PIPELINE_IID} AND NOT $ENV{CI_PIPELINE_IID} STREQUAL "")
            set("${PROJECT_NAME}_VERSION_MORE" $ENV{CI_PIPELINE_IID})
        else()
            set("${PROJECT_NAME}_VERSION_MORE" -1)
        endif()
    endif()
    set("${PROJECT_NAME}_VERSION_END" "${${PROJECT_NAME}_VERSION_MORE}_${BRANCH_NAME}")
endif()

set("${PROJECT_NAME}_VERSION_MAJOR" ${PROJECT_INTERNAL_VERSION_MAJOR})
set("${PROJECT_NAME}_VERSION_MINOR" ${PROJECT_INTERNAL_VERSION_MINOR})
set("${PROJECT_NAME}_VERSION_PATCH" ${PROJECT_INTERNAL_VERSION_PATCH})
set("${PROJECT_NAME}_VERSION_MORE" ${${PROJECT_NAME}_VERSION_MORE})
set("${PROJECT_NAME}_VERSION_BRANCH" ${BRANCH_NAME})
set("${PROJECT_NAME}_VERSION_FULL_STRING" "${PROJECT_INTERNAL_VERSION_MAJOR}.${PROJECT_INTERNAL_VERSION_MINOR}.${PROJECT_INTERNAL_VERSION_PATCH}.${${PROJECT_NAME}_VERSION_END}")

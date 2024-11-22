#pragma once
#include <filesystem>
#include <iostream>
#include <string>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

static std::string getLogDirectory() {
    const std::string APP_NAME = "WSCpp";
    std::filesystem::path logDir;

#if defined(_WIN32) || defined(_WIN64)
    const char* appData = std::getenv("APPDATA");
    if (appData) {
        logDir = std::filesystem::path(appData) / APP_NAME / "Logs";
    } else {
        logDir = std::filesystem::path("C:\\") / APP_NAME / "Logs";
    }
#elif defined(__APPLE__) && defined(__MACH__)
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        logDir = std::filesystem::path(homeDir) / "Library/Logs" / APP_NAME;
    } else {
        logDir = "/tmp/" + APP_NAME + "/Logs";
    }
#elif defined(__linux__)
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        logDir = std::filesystem::path(homeDir) / ".local/share" / APP_NAME / "Logs";
    } else {
        logDir = "/tmp/" + APP_NAME + "/Logs";
    }
#else
    logDir = "/tmp/" + APP_NAME + "/Logs";
#endif
    try {
        std::filesystem::create_directories(logDir);
    } catch (const std::filesystem::filesystem_error& e) {
        return "";
    }
    return logDir.string();
}

static std::string getBasePath() {
    std::string basePath = ".";

#ifdef __APPLE__
    // macOS: Get app bundle resources directory
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle) {
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if (resourcesURL) {
            char path[PATH_MAX];
            if (CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8*)path, PATH_MAX)) {
                basePath = path;
            }
            CFRelease(resourcesURL);
        }
    }
#endif
    return basePath;
}

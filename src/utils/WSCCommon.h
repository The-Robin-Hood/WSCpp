#pragma once
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <filesystem>
#include <iostream>
#include <string>

namespace WSCUtils {
    inline std::string getLogDirectory() {
        const std::string APP_NAME = "WSCpp";
        std::filesystem::path logDir;

#if defined(_WIN32) || defined(_WIN64)
        char* appData = nullptr;
        size_t len = 0;
        if (_dupenv_s(&appData, &len, "APPDATA") == 0 && appData) {
            logDir = std::filesystem::path(appData) / APP_NAME / "Logs";
            free(appData);
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
            std::cerr << "Failed to create log directory: " << e.what() << std::endl;
            return "";
        }
        return logDir.string();
    }

    inline std::string getBasePath() {
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

#ifdef _WIN32
    inline std::vector<char> loadResource(int resourceId, const std::string& resourceType) {
        std::cout << "Loading resource: " << resourceId << " of type: " << resourceType
                  << std::endl;
        HMODULE hModule = GetModuleHandle(NULL);
        std::wstring resourceTypeW(resourceType.begin(), resourceType.end());
        HRSRC hResource =
            FindResource(hModule, MAKEINTRESOURCE(resourceId), resourceTypeW.c_str());
        if (!hResource) {
            std::cerr << "Error: Resource not found. " << GetLastError() << std::endl;
            return {};
        }

        HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
        if (!hLoadedResource) {
            std::cerr << "Error: Could not load resource.\n";
            return {};
        }

        DWORD resourceSize = SizeofResource(hModule, hResource);
        if (resourceSize == 0) {
            std::cerr << "Error: Resource size is zero.\n";
            return {};
        }

        void* pResourceData = LockResource(hLoadedResource);
        return std::vector<char>((char*)pResourceData, (char*)pResourceData + resourceSize);
    }
#endif
}  // namespace WSCUtils
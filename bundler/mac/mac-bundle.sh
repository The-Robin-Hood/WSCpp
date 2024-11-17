#!/bin/bash

# Application settings
readonly APP_NAME="WSCpp"
readonly RELEASE_DIR="../../bin/Release"
readonly BUNDLE_NAME="${APP_NAME}.app"

# Bundle structure paths
readonly BUNDLE_PATH="${RELEASE_DIR}/${BUNDLE_NAME}"
readonly BUNDLE_CONTENTS="${BUNDLE_PATH}/Contents"
readonly BUNDLE_MACOS="${BUNDLE_CONTENTS}/MacOS"
readonly BUNDLE_RESOURCES="${BUNDLE_CONTENTS}/Resources"
readonly EXECUTABLE="${RELEASE_DIR}/${APP_NAME}"

# Required files
readonly INFO_PLIST="Info.plist"
readonly ICON_FILE="${APP_NAME}.icns"

log() {
    echo "[${APP_NAME}] :: $1"
}

check_prerequisites() {
    if [ ! -f "${EXECUTABLE}" ]; then
        log "Error: Executable not found! Please compile your application first."
        exit 1
    fi

    if [ ! -f "${INFO_PLIST}" ]; then
        log "Error: ${INFO_PLIST} not found. Aborting."
        exit 1
    fi
}

create_bundle_structure() {
    if [ -d "${BUNDLE_PATH}" ]; then
        log "Removing existing app bundle..."
        rm -rf "${BUNDLE_PATH}"
    fi

    log "Creating app bundle structure..."
    mkdir -p "${BUNDLE_MACOS}" "${BUNDLE_RESOURCES}"
}

copy_files() {
    log "Copying executable..."
    cp "${EXECUTABLE}" "${BUNDLE_MACOS}/"
    chmod +x "${BUNDLE_MACOS}/${APP_NAME}"

    log "Copying Info.plist..."
    cp "${INFO_PLIST}" "${BUNDLE_CONTENTS}/"

    if [ -f "${ICON_FILE}" ]; then
        log "Copying app icon..."
        cp "${ICON_FILE}" "${BUNDLE_RESOURCES}/icon.icns"
    else
        log "Warning: ${ICON_FILE} not found. App will use default icon."
    fi
}

main() {
    check_prerequisites
    create_bundle_structure
    copy_files

    log "Setting permissions..."
    chmod -R 755 "${BUNDLE_PATH}"
    log "Done! App bundle created at ${BUNDLE_NAME}"
}

main
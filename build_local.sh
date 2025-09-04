#!/bin/bash

# Pi Stream v2 Local Build Script
# This script attempts to build the firmware locally

set -e

echo "🔧 Pi Stream v2 Local Build Script"
echo "=================================="

# Check prerequisites
echo "📋 Checking prerequisites..."

if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "❌ ARM toolchain not found. Please install arm-none-eabi-gcc"
    echo "   On macOS: brew install arm-none-eabi-gcc"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 not found. Please install Python 3"
    exit 1
fi

echo "✅ Prerequisites check passed"

# Check if we're in the right directory
if [ ! -f "Makefile" ]; then
    echo "❌ Makefile not found. Please run this script from the project root"
    exit 1
fi

echo "📁 Project directory: $(pwd)"

# Clean previous build
echo "🧹 Cleaning previous build..."
make clean || {
    echo "⚠️  Clean failed, but continuing..."
}

# Build the firmware
echo "🔨 Building Pi Stream firmware..."
echo "   Target: n0110 (NumWorks calculator)"
echo "   Platform: device"
echo "   Username: PiStream"

make OMEGA_USERNAME="PiStream" PLATFORM=device TARGET=n0110 binpack -j1

# Check build results
if [ -d "output/release/device/bootloader" ]; then
    echo "✅ Build successful!"
    echo "📂 Build artifacts:"
    ls -la output/release/device/bootloader/

    echo ""
    echo "🎉 Firmware compilation completed successfully!"
    echo "   You can now flash the .dfu files to your NumWorks calculator"
else
    echo "❌ Build failed - no bootloader directory found"
    echo "📂 Checking build output directory:"
    ls -la output/release/device/ 2>/dev/null || echo "   No device directory found"
    exit 1
fi

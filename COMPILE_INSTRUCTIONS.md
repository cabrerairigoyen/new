# Pi Stream v2 Firmware Compilation Guide

## 🚀 Quick Compilation with GitHub Actions

The easiest way to compile the fixed Pi Stream firmware is using GitHub Actions:

### Step 1: Push Your Changes
```bash
git add .
git commit -m "Fix Pi Stream app crashes with safe UART handling"
git push origin main
```

### Step 2: Trigger GitHub Actions Build
1. Go to your GitHub repository
2. Click on "Actions" tab
3. Select "Build Pi Stream v2 Upsilon" workflow
4. Click "Run workflow" button
5. Choose the branch (main) and click "Run workflow"

### Step 3: Download Compiled Firmware
Once the build completes:
1. Go to the Actions tab
2. Click on the completed workflow run
3. Download the `pistream-v2-upsilon` artifact
4. Extract the `.dfu` files for flashing

## 🔧 Local Compilation (Advanced Users)

If you prefer local compilation, follow these steps:

### Prerequisites
```bash
# Install ARM toolchain
brew install arm-none-eabi-gcc

# Install dependencies
brew install python3 imagemagick libpng freetype jpeg
```

### Build Commands
```bash
# Clean previous build
make clean

# Build for n0110 (Pi Stream target)
make OMEGA_USERNAME="PiStream" PLATFORM=device TARGET=n0110 binpack -j1

# Check build output
ls -la output/release/device/bootloader/
```

## 📋 What Was Fixed

The following critical issues were resolved in the Pi Stream app:

### ✅ Fixed Issues
1. **Blocking UART Read** - Replaced blocking `Ion::Console::readChar()` with non-blocking implementation
2. **Buffer Overflow Protection** - Added bounds checking and safe memory operations
3. **Watchdog Mechanism** - Added timeout protection and processing limits
4. **Error Recovery** - Implemented emergency reset functionality
5. **Safe LaTeX Processing** - Added try-catch blocks for math expression parsing

### 🔧 Technical Changes
- Added timeout protection for UART operations (1000ms)
- Implemented processing counter limits (100 operations max)
- Added safe buffer shifting with bounds checking
- Enhanced error handling with graceful degradation
- Added emergency reset functionality for critical errors

## 🎯 Testing the Fix

After flashing the new firmware:

1. **Launch the Pi Stream app** from the menu
2. **Connect Raspberry Pi** via UART cables (RX/TX GPIO pins)
3. **Verify stable operation** - app should not crash or freeze
4. **Test error recovery** - disconnect/reconnect should work smoothly

## 📞 Troubleshooting

If compilation fails:
1. Check that ARM toolchain is properly installed
2. Verify Python 3 and required libraries are available
3. Ensure all submodules are initialized (`git submodule update --init --recursive`)
4. Try using GitHub Actions instead of local compilation

## 🔗 Related Files
- `apps/pistream/pi_stream_controller.cpp` - Main controller with fixes
- `apps/pistream/pi_stream_controller.h` - Updated header
- `.github/workflows/build-pistream-v2.yml` - GitHub Actions workflow

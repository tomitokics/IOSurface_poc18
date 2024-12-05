PROJECT_FILE         := $(shell find . -name "*.xcodeproj" | head -n 1)
ARCHIVE_PATH         := build/explt.xcarchive
DERIVED_DATA_PATH    := build/DerivedData
IPA_PATH             := build/explt.ipa
EXPORT_OPTIONS_PLIST := $(shell pwd)/entitlements.plist

# Define the target for the full build process
all: build_ipa

# Step 1: List available Xcode versions and switch to Xcode 16.1
setup_xcode:
	@echo "Available Xcode versions:"
	@ls /Applications | grep Xcode
	@echo "Targeting Xcode 16.1..."
	@if [ ! -d "/Applications/Xcode_16.1.app" ]; then \
		echo "Error: Xcode 16.1 is not installed on the runner."; \
		exit 1; \
	fi
	@echo "Switching to Xcode 16.1..."
	@sudo xcode-select --switch /Applications/Xcode_16.1.app/Contents/Developer
	@xcodebuild -version

# Step 2: Clean the project
clean:
	@if [ -z "$(PROJECT_FILE)" ]; then \
		echo "Error: No .xcodeproj file found."; \
		exit 1; \
	fi
	@echo "Cleaning project..."
	@xcodebuild -project "$(PROJECT_FILE)" \
				-scheme explt \
				-configuration Release \
				-sdk iphoneos \
				-derivedDataPath "$(DERIVED_DATA_PATH)" \
				clean

# Step 3: Build the app (unsigned) and archive
archive:
	@if [ -z "$(PROJECT_FILE)" ]; then \
		echo "Error: No .xcodeproj file found."; \
		exit 1; \
	fi
	@echo "Building and archiving the app..."
	@xcodebuild -project "$(PROJECT_FILE)" \
				-scheme explt \
				-configuration Release \
				-sdk iphoneos \
				-derivedDataPath "$(DERIVED_DATA_PATH)" \
				-archivePath "$(ARCHIVE_PATH)" \
				CODE_SIGN_IDENTITY="" \
				CODE_SIGNING_REQUIRED=NO \
				CODE_SIGN_ENTITLEMENTS="" \
				CODE_SIGNING_ALLOWED=NO \
				archive

# Step 4: Manually create the IPA file
export_ipa:
	@echo "Creating the IPA file manually..."
	@mkdir -p build/temp
	@cp -R "$(ARCHIVE_PATH)/Products/Applications/explt.app" build/temp/
	@cd build/temp && zip -r ../explt.ipa explt.app
	@rm -rf build/temp

# Step 5: Complete build process (combine all steps)
build_ipa: setup_xcode clean archive export_ipa
	@echo "IPA build complete and available at $(IPA_PATH)"

# Target to clean the build directory
clean_all:
	@echo "Cleaning all build files..."
	@rm -rf build

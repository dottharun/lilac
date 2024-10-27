#!/bin/bash

# Set the desired LLVM version
LLVM_VERSION="18.1.8"

# Set the download URL for the pre-built LLVM binaries
LLVM_DOWNLOAD_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04.tar.xz"

# Set the installation directory
INSTALL_DIR="/usr/local/llvm-${LLVM_VERSION}"

# Create a temporary directory for downloading and extracting LLVM
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# Download the LLVM binaries
echo "Downloading LLVM ${LLVM_VERSION}..."
curl -L -o "clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04.tar.xz" "${LLVM_DOWNLOAD_URL}"

# Extract the LLVM binaries
echo "Extracting LLVM ${LLVM_VERSION}..."
tar xf "clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04.tar.xz"

# Move the extracted LLVM directory to the installation directory
rm -rf $INSTALL_DIR
echo "Installing LLVM ${LLVM_VERSION} to ${INSTALL_DIR}..."
mv "clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04" "${INSTALL_DIR}"

# Clean up the temporary directory
rm -rf "$TEMP_DIR"

echo "LLVM ${LLVM_VERSION} has been successfully installed to ${INSTALL_DIR}."

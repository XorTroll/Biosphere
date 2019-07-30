#!/bin/bash

# Remove if present
rm -f /etc/profile.d/biosphere-vars.sh

# Script's directory
CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "Setting root variable 'BIOSPHERE_ROOT' in '/etc/profile.d/biosphere-vars.sh' to \"${CUR_DIR}\"..."

# Set file with root path
echo "# Biosphere root directory (BIOSPHERE_ROOT)

export BIOSPHERE_ROOT=$CUR_DIR" >> /etc/profile.d/biosphere-vars.sh
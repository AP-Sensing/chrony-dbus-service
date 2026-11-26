#!/bin/bash

printf "🗑️  Clearing your build directory...\n"
rm -rf build/*

GREEN='\033[0;32m'
NC='\033[0m'

printf "✅ ${GREEN}Done. Build directory deleted.${NC}\n"

#!/bin/sh

set -e

TRUNK_BUILD_FILEHASH=false TRUNK_BUILD_NO_SRI=true trunk build --release

cd dist/

find -name "*.js" -exec terser {} --compress -o {} \;
gzip -r --best .

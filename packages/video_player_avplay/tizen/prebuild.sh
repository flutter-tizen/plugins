BUILD_ARCH=${1}
LIB_API_VERSION=""

if [ "$API_VERSION" = "6.0" ]; then
  LIB_API_VERSION="6.0"
elif [ "$API_VERSION" = "6.5" ] || [ "$API_VERSION" = "7.0" ] || [ "$API_VERSION" = "8.0" ] || [ "$API_VERSION" = "9.0" ]; then
  LIB_API_VERSION="6.5"
else
  LIB_API_VERSION="10.0"
fi

SRC_DIR="lib/${BUILD_ARCH}/${LIB_API_VERSION}"
DST_DIR="lib/${BUILD_ARCH}/target_plusplayer_libs"

if [ ! -d "$DST_DIR" ]; then
  mkdir -p "$DST_DIR"
fi
cp -f "$SRC_DIR"/*.so "$DST_DIR"/

echo "USER_LIB_PATH=${SRC_DIR}" > custom_def.prop

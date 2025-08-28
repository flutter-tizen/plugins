BUILD_ARCH=${1}

SRC_DIR=""
if [ "$API_VERSION" = "6.5" ] || [ "$API_VERSION" = "7.0" ] || [ "$API_VERSION" = "8.0" ] || [ "$API_VERSION" = "9.0" ]; then
  SRC_DIR="lib/${BUILD_ARCH}/6.5"
else
  SRC_DIR="lib/${BUILD_ARCH}/${API_VERSION}"
fi
DST_DIR="lib/${BUILD_ARCH}/candidate"

mkdir -p "$DST_DIR"
cp -f "$SRC_DIR"/*.so "$DST_DIR"/

echo "Use ${BUILD_ARCH}/$API_VERSION libs."

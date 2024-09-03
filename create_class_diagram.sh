# Expects the BOOST_ROOT environment variable to be set.

# Get and build clang-uml: https://github.com/bkryza/clang-uml
# Expects the CLANG_UML environment variable to be set, containing the path to the clang-uml executable

# Example:
# BOOST_ROOT=~/projects/boost CLANG_UML=~/projects/clang-uml/build/clang-uml ./create_class_diagram.sh

mkdir -p example/cpp11/http/server/build
pushd example/cpp11/http/server/build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make -j4
popd

${CLANG_UML} -c .clang-uml -d example/cpp11/http/server/build/compile_commands.json -o ./doc

# Install plantuml & use it to convert to image format
# sudo apt install plantuml
plantuml ./doc/boost_asio.puml -tsvg

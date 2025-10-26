find -type f -name "*.cpp" -exec sed -i '0r LICENSE.cpp' {} \;
find -type f -name "*.h" -exec sed -i '0r LICENSE.cpp' {} \;

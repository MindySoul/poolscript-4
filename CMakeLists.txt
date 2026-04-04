cmake_minimum_required(VERSION 3.18)
project(PscompilerPoolscript VERSION 4.3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Otimização específica para Intel i5 4ª Geração (Haswell)
# AVX2 e FMA são suportados pelo Haswell — não use AVX512 (não tem no i5 4xxx)
if(NOT MSVC)
    add_compile_options(-O3 -mavx2 -mfma -march=haswell)
else()
    add_compile_options(/O2 /arch:AVX2)
endif()

# Inclui a pasta da biblioteca llama.cpp (deve ser clonada do GitHub)
add_subdirectory(llama.cpp)

# Cria o executável a partir do PScom.cpp
add_executable(PscompilerPoolscript PScom.cpp)

# Une o código com a llama.cpp
target_link_libraries(PscompilerPoolscript PRIVATE llama ${CMAKE_THREAD_LIBS_INIT})

# Inclui os headers da llama.cpp
target_include_directories(PscompilerPoolscript PRIVATE llama.cpp)

# Para GPU NVIDIA (opcional — descomente se tiver GPU NVIDIA):
# cmake .. -G "MinGW Makefiles" -DLLAMA_CUDA=ON -DCMAKE_BUILD_TYPE=Release

SRC_DIR := D:/QtProjects/Mp3tagQt
BUILD_DIR := D:/QtProjects/Mp3tagQt/build/Desktop_Qt_6_10_1_llvm_mingw_64_bit-Debug
QT_DIR := D:/Programms/Qt/6.10.1/llvm-mingw_64
CMAKE := D:/Programms/Qt/Tools/CMake_64/bin/cmake.exe
CXX_COMPILER := D:/Programms/Qt/Tools/llvm-mingw1706_64/bin/clang++.exe
C_COMPILER := D:/Programms/Qt/Tools/llvm-mingw1706_64/bin/clang.exe
QMAKE := D:/Programms/Qt/6.10.1/llvm-mingw_64/bin/qmake.exe
MAINTENANCE_TOOL := D:/Programms/Qt/MaintenanceTool.exe

generate:
	@echo "generate cmake file ..."
	$(CMAKE) -S $(SRC_DIR) -B $(BUILD_DIR) \
		"-DCMAKE_PREFIX_PATH:PATH=$(QT_DIR)" \
		"-DCMAKE_CXX_COMPILER:FILEPATH=$(CXX_COMPILER)" \
		"-DCMAKE_BUILD_TYPE:STRING=Debug" \
		"-DCMAKE_COLOR_DIAGNOSTICS:BOOL=ON" \
		"-DCMAKE_CXX_FLAGS_INIT:STRING=-DQT_QML_DEBUG" \
		"-DCMAKE_GENERATOR:STRING=Ninja" \
		"-DQT_QMAKE_EXECUTABLE:FILEPATH=$(QMAKE)" \
		"-DCMAKE_C_COMPILER:FILEPATH=$(C_COMPILER)" \
		"-DQT_MAINTENANCE_TOOL:FILEPATH=$(MAINTENANCE_TOOL)"
	@echo "generate end"

build: generate
	@echo "compile ..."
	$(CMAKE) --build $(BUILD_DIR) --target all
	@echo "compile end"

run: build
	@echo "start ..."
	$(BUILD_DIR)/mp3tag.exe

clean:
	@echo "clean build-directory..."
	-rm -rf $(BUILD_DIR)

all: generate build run

.PHONY: generate build run clean all

# Nombre del ejecutable
TARGET = holy-c.exe

# Compilador
CXX = cl

# Flags
CXXFLAGS = /std:c++latest /EHsc /O2 /W4 /DDEBUG_MODE /I "." /I "C:\Users\Teologado-6\Documents\Fr. Emanuel\dev\cpp\fmt\include"

# Sources
SRCS = main.cpp ast.cpp parser.cpp lexer.cpp util.cpp

# Objetos
OBJS = $(SRCS:.cpp=.obj)

# Link
$(TARGET): $(OBJS)
	link /OUT:$(TARGET) $(OBJS)

# Compile
%.obj: %.cpp
	$(CXX) $(CXXFLAGS) /c $< /Fo$@

# Clean
clean:
	del /Q *.obj *.exe

.PHONY: clean
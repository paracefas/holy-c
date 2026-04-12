# Nombre del ejecutable
TARGET = holy-c.exe

# Compilador y flags
CXX = g++
# -I. le dice que busque cabeceras en el directorio actual
CXXFLAGS = -DDEBUG_MODE -Wall -Wextra -I. -O3 -std=c++26 -I "C:\Users\Teologado-6\Documents\Fr. Emanuel\dev\cpp\fmt\include" -lstdc++exp -fno-diagnostics-show-option -fmax-errors=1

# Lista de archivos fuente (.cpp)
# Añadimos todos los que has creado
SRCS = main.cpp ast.cpp parser.cpp lexer.cpp util.cpp

# Lista de objetos (.o)
# Esto transforma la lista de .cpp en .o (main.o, ast.o, parser.o)
OBJS = $(SRCS:.cpp=.o)

# Regla principal: Enlazar los objetos para crear el ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regla para compilar cada .cpp a un .o
# El flag -c indica que solo compile, no enlace
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Evitar conflictos con archivos que se llamen clean
.PHONY: clean
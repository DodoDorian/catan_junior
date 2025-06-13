# Compiler und Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -I include
DEBUGFLAGS = -g
TARGET = OCEngine

# Verzeichnisse
SRCDIR = src
OBJDIR = obj

# Quellcodedateien
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Objektdateien mit eigenem Objektpfad
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Standardziel
all: $(TARGET)

release: $(TARGET)

# Debug-Ziel
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

# Regel zum Erstellen des Ziels
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

# Regel zum Erstellen der Objektdateien
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)  # Erstellen Sie den Objektpfad, falls er nicht existiert
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regel zum Bereinigen
clean:
	rm -f $(OBJECTS) $(TARGET)

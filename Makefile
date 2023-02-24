CC        := g++
LD        := g++
CC_FLAGS := -std=c++11 -O3 -g -Wno-gnu-array-member-paren-init

MODULES   := exec host nvm_chip nvm_chip/flash_memory sim ssd utils integration
SRC_DIR   := $(addprefix src/,$(MODULES)) src
BUILD_DIR := $(addprefix build/,$(MODULES)) build

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
#SRC       := src/main.cpp $(SRC)
OBJ       := $(patsubst src/%.cpp,build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR))

vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
	$(CC) $(CC_FLAGS) $(INCLUDES) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs libMQSim.so MQSim

libMQSim.so: $(OBJ)
	$(LD) $(CC_FLAGS) -shared $^ -o $@ -lz


MQSim: libMQSim.so
	$(CC) $(CC_FLAGS) src/main.cpp libMQSim.so -o MQSim

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f MQSim

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

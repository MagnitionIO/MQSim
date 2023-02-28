CC        := g++
LD        := g++
CC_FLAGS := -std=c++17 -fPIC -g -Wno-gnu-array-member-paren-init -O0 -g3 -ggdb3 -DSINGLETON -O0

MODULES   := exec host nvm_chip nvm_chip/flash_memory sim ssd utils lib
SRC_DIR   := $(addprefix src/,$(MODULES)) src
BUILD_DIR := $(addprefix build/,$(MODULES)) build

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
OBJ       := $(patsubst src/%.cpp,build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR))

LIB_PATH = /usr/local/lib
INC_PATH = /usr/local/include
CFG_PATH = /usr/local/share

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
	$(CC) $(CC_FLAGS) $(INCLUDES) src/main.cpp libMQSim.so -o MQSim

install: all
	@mkdir -p $(INC_PATH)/libMQSim
	@cp -v src/lib/libmqsim.h $(INC_PATH)/libMQSim
	@cp -v src/lib/base.h $(INC_PATH)/libMQSim
	@cp -v src/lib/logger.h $(INC_PATH)/libMQSim
	@cp -v src/lib/data_store.h $(INC_PATH)/libMQSim
	@cp -v libMQSim.so $(LIB_PATH)
	@mkdir -p $(CFG_PATH)/libMQSim
	@cp -v ssdconfig.xml $(CFG_PATH)/libMQSim
	@cp -r traces $(CFG_PATH)/libMQSim
	@chmod 777 $(CFG_PATH)/libMQSim

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f MQSim libMQSim.so

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

LIBS_ = sfml-graphics sfml-window sfml-system
LIBS = $(patsubst %,-l%,$(LIBS_))

SRC=main.cpp guitarNeck.cpp
DEPS=guitarNeck.hpp xShape.hpp $(SRC)

main: $(DEPS)
	g++ -g -o $@ $(SRC) $(LIBS)

.PHONY: clean
clean:
	rm -f main
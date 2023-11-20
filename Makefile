LIBS_ = sfml-graphics sfml-window sfml-system sfml-audio
LIBS = $(patsubst %,-l%,$(LIBS_))

SRC=main.cpp guitarNeck.cpp button.cpp
DEPS=guitarNeck.hpp xShape.hpp button.hpp resources.hpp $(SRC)

main: $(DEPS)
	g++ -g -o $@ $(SRC) $(LIBS)

.PHONY: clean
clean:
	rm -f main
LIBS_ = sfml-graphics sfml-window sfml-system
LIBS = $(patsubst %,-l%,$(LIBS_))

main: main.cpp guitarNeck.cpp
	g++ -g -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f main